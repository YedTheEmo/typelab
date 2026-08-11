# Supabase security - typing

This lesson types Row Level Security for an owned resource: enable RLS,
write a policy for each operation, and verify that browser clients are
constrained by the database.

## Create an owned resource

Start with a table whose rows belong to users.

```sql
-- create a resource owned by one authenticated user
create table documents (
    -- generate a unique identifier for every document
    id uuid primary key default gen_random_uuid(),

    -- link the document to its authenticated owner
    owner_id uuid not null references auth.users(id),

    -- require every document to have a title
    title text not null,

    -- record when the document was created
    created_at timestamptz not null default now()
);
```

The `owner_id` column ties every row to the user from the auth lesson.

## Enable row level security

RLS is off by default, so it must be turned on explicitly.

```sql
-- enable row level security on the resource
alter table documents enable row level security;
```

With RLS on and no policies, every operation is denied.

## Allow owners to read

Grant select access to rows the current user owns.

```sql
-- allow users to read only their own documents
create policy "users read own documents"
on documents
for select
to authenticated
using (auth.uid() = owner_id);
```

The policy is scoped to signed-in users, and the condition uses the session
identity rather than a client-supplied value.

## Allow owners to create

An insert must produce a row the user is allowed to own.

```sql
-- allow users to create documents only for themselves
create policy "users create own documents"
on documents
for insert
to authenticated
with check (auth.uid() = owner_id);
```

The `with check` condition rejects an insert that assigns a different owner.

## Allow owners to update

An update affects existing rows and produces a new row.

```sql
-- allow users to update only documents they own
create policy "users update own documents"
on documents
for update
to authenticated
using (auth.uid() = owner_id)
with check (auth.uid() = owner_id);
```

`using` constrains which existing rows may change.

`with check` constrains the row that results.

## Allow owners to delete

Deletion requires its own policy.

```sql
-- allow users to delete only documents they own
create policy "users delete own documents"
on documents
for delete
to authenticated
using (auth.uid() = owner_id);
```

Reading a row does not automatically grant the right to delete it.

## Allow sharing

Ownership is not the only access model.

A row can be shared with specific users.

```sql
-- add a column listing users who may read the document
alter table documents
add column shared_with uuid[] not null default '{}';
```

```sql
-- allow access when the user is listed as a collaborator
create policy "users read shared documents"
on documents
for select
to authenticated
using (auth.uid() = any(shared_with));
```

The two read policies combine:

```text
auth.uid() = owner_id
    OR
auth.uid() = any(shared_with)
```

Either condition grants read access.

## Verify through the client

The browser client is now limited to the caller's own rows.

```tsx
// import the browser-side Supabase client
import { createClient } from "@/lib/supabase/client"

// create a client for browser-side access
const supabase = createClient()

// read documents visible to the current user
const { data: documents, error } = await supabase
    .from("documents")
    .select("id, title")
```

An anonymous visitor sees no rows because no policy grants anon access.

```tsx
// attempt an insert as a different owner
const { error } = await supabase
    .from("documents")
    .insert({
        // try to claim ownership of another user's document
        owner_id: "someone-elses-id",

        // provide a title
        title: "Attempted insert",
    })
```

The `with check` policy rejects the row, even though the client tried to
supply the id.

## The service role bypass

A privileged server client can bypass RLS deliberately.

```ts
// import the Supabase client constructor
import { createClient } from "@supabase/supabase-js"

// create a service role client for server code
const supabase = createClient(
    // provide the project URL
    Deno.env.get("SUPABASE_URL")!,

    // provide the privileged service role key
    Deno.env.get("SUPABASE_SERVICE_ROLE_KEY")!,
)
```

The service role bypasses policies, so this client must stay out of browser
code.

## Practice

Type the core enable + select policy:

```sql
alter table documents enable row level security;
```

```sql
create policy "users read own documents"
on documents
for select
to authenticated
using (auth.uid() = owner_id);
```

Then type the update policy with both conditions:

```sql
create policy "users update own documents"
on documents
for update
to authenticated
using (auth.uid() = owner_id)
with check (auth.uid() = owner_id);
```

The central pattern is:

```text
enable RLS
  -> per-operation policies
  -> scoped to authenticated
  -> conditions based on auth.uid()
  -> browser clients constrained by the database
```

The database enforces the rules, so no client can bypass them.
