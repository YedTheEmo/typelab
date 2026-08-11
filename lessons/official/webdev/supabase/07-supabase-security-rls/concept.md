# Supabase security - concepts

The overview lesson introduced the central security question of a Supabase
application:

```text
browser
    |
    +-- project URL
    +-- publishable key
    |
    v
Supabase Data API
    |
    v
PostgreSQL
```

Browser code holds the publishable key and can issue database operations.

If nothing constrained those operations, any visitor could read or write any
row.

Row Level Security, commonly called RLS, is the mechanism that constrains
them.

## The threat model

A Supabase application trusts browser clients in a controlled way.

```text
attacker
    |
    v
browser request (valid-looking key)
    |
    v
Supabase Data API
    |
    v
database
```

An attacker can craft any request the client library can express.

They do not need to respect the application's UI or its rules.

Security must therefore be enforced where the data lives, not only where the
UI lives.

## Authorization at the database level

RLS is PostgreSQL Row Level Security.

When RLS is enabled on a table, PostgreSQL evaluates a policy for each row
being accessed.

```text
database request
        |
        v
row
    |
    v
policy
    |
    +-- allow
    +-- reject
```

The decision happens in the database, using the identity attached to the
request.

This means the rules hold no matter which client issued the request.

## Default deny

RLS follows a default-deny model.

When RLS is enabled on a table and no policy allows an operation, the
operation is rejected.

```text
RLS enabled + no policy  ->  denied
```

That is the safe starting point.

Policies then explicitly grant the operations that should be allowed.

## Enabling RLS

RLS is not on automatically for new tables.

It must be enabled explicitly.

```sql
alter table documents enable row level security;
```

Until RLS is enabled, the table relies on whatever other access controls
exist, which is normally nothing useful for untrusted clients.

The first step of securing a table is therefore enabling RLS.

## Policies

A policy grants one kind of operation on a table.

Policies exist for each data operation:

```text
select  ->  read rows
insert  ->  add rows
update  ->  change rows
delete  ->  remove rows
```

Each policy can be scoped to a role.

```text
to anon
to authenticated
```

And each policy carries conditions that decide whether an operation is
allowed.

## Policies compose

Multiple policies for the same operation are combined with OR.

```text
policy A OR policy B  ->  allowed
```

For example, a read policy could allow both owners and shared collaborators:

```sql
create policy "read own"
on documents
for select
using (auth.uid() = owner_id);

create policy "read shared"
on documents
for select
using (auth.uid() = any(shared_with));
```

A row passes if either policy lets it through.

Composition is useful, but it also means a broad extra policy can widen
access in unexpected ways.

## Sharing resources

Not every resource is owned by one user.

A common extension is shared access.

```text
document
    |
    +-- owner
    +-- collaborators
```

The owner policy and the collaborator policy can both allow reads.

```sql
create policy "read own"
on documents
for select
using (auth.uid() = owner_id);

create policy "read shared"
on documents
for select
using (auth.uid() = any(shared_with));
```

Updates can be split the same way:

```text
owner         ->  may edit
collaborator  ->  may edit, may not delete
```

RLS handles each operation separately, so an application can express fine
distinctions such as "can edit but cannot delete."

The policy language is just SQL, and any expression that the database can
evaluate can appear in a condition.

## Grants versus RLS

Two different mechanisms control access to a table.

A grant decides whether a role may run an operation on the table at all.

```text
grant  ->  may the role touch this table?
```

RLS decides which rows are visible within that operation.

```text
policy ->  which rows may the role touch?
```

In Supabase, the client roles typically receive the underlying grants so the
Data API can work.

RLS then does the row-level filtering.

Disabling RLS leaves the grants unrestricted, which is why enabling RLS is
the critical step.

## using and with check

A policy has two kinds of conditions.

`using` decides which existing rows an operation may touch.

```sql
using (auth.uid() = owner_id)
```

`with check` decides whether a new or changed row is acceptable.

```sql
with check (auth.uid() = owner_id)
```

The distinction matters:

```text
select  ->  using
delete  ->  using
update  ->  using (existing rows) + with check (resulting row)
insert  ->  with check (the new row)
```

A row that is already there and a row that would be created are different
questions.

## The identity in policies

Policies use the authenticated identity from the auth lesson.

```text
auth.uid()  ->  current user id
```

An anonymous request has no uid.

An authenticated request does.

A typical ownership policy:

```sql
using (auth.uid() = owner_id)
```

This grants access to rows whose `owner_id` matches the current user.

The user id comes from the request's session, not from a value the client
can claim.

## Roles and the service role

RLS decisions depend on the role of the request.

```text
anon            ->  unauthenticated browser requests
authenticated   ->  signed-in browser requests
service_role    ->  privileged server access
```

The `service_role` bypasses RLS entirely.

That makes it powerful and dangerous.

```text
service_role
    |
    v
bypasses RLS
    |
    v
full access
```

It belongs exclusively in trusted server code, such as an Edge Function, and
must never be exposed to browser clients.

## Testing policies

Policies should be tested from the perspective of the client roles.

A policy that looks correct in SQL can still surprise at runtime.

A useful test is to ask, for each operation:

```text
as anon:          what should be visible?
as user A:        what should be visible?
as user B:        what should be visible?
```

Running the same query under different identities reveals whether the
policy conditions are actually doing the filtering.

The Supabase dashboard provides a way to run SQL as these roles, which makes
this kind of verification straightforward.

## RLS and application code

RLS does not replace application checks.

It is the database floor beneath them.

```text
application checks
    |
    v
RLS policies
    |
    v
data
```

Defense in depth means both layers matter.

A TanStack server function can validate input and apply application rules,
while RLS guarantees that even a directly crafted database request is
constrained.

## RLS and the other services

RLS is not limited to application tables.

The same ideas protect Storage objects and gate Realtime events.

```text
tables       ->  RLS policies
Storage      ->  storage policies
Realtime     ->  policies on delivered changes
```

A client should only receive events and files it would be allowed to read.

This is why the auth lesson's identity is so central: one identity, enforced
across every Supabase service.

## Common mistakes

RLS errors usually come from a small set of patterns:

```text
forgetting to enable RLS on a table
writing policies that ignore the current user
granting a broad policy to anon
relying only on UI-level checks
exposing the service role to the browser
```

Each mistake makes the database weaker than the application appears.

## The complete security model

```text
browser request
    |
    v
publishable key + session
    |
    v
Supabase Data API
    |
    v
PostgreSQL
    |
    v
RLS policy
    |
    +-- anon -> default deny
    +-- authenticated -> auth.uid() checks
    |
    v
allowed rows
```

The application UI is a front end to a database that enforces its own rules.

RLS is what makes direct browser access to PostgreSQL safe enough to build
on.

## Next step

The next lesson closes the track by covering the development workflow:
the Supabase CLI, migrations, and how to keep this architecture reproducible
across environments.
