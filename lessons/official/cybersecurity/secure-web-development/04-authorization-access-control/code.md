# Authorization and access control - typing

This lesson types authorization with Supabase PostgreSQL RLS and
server-side checks in a TanStack Start application.

## Create an owned resource

Start with a resource that belongs to one authenticated user.

```sql
-- create a resource owned by one authenticated user
create table documents (
    id uuid primary key default gen_random_uuid(),
    owner_id uuid not null references auth.users(id),
    title text not null,
    created_at timestamptz not null default now()
);
```

The `owner_id` field establishes the database relationship between the
resource and its authenticated owner.

## Enable row level security

A table should not rely on application code alone when RLS is part of the
authorization architecture.

```sql
-- enable row level security on the resource
alter table documents enable row level security;
```

## Authorize reads

A user should only be able to read documents they own.

```sql
-- allow users to read only their own documents
create policy "users read own documents"
on documents
for select
using (auth.uid() = owner_id);
```

The database derives the caller identity from the authenticated Supabase
context rather than from a request parameter.

## Authorize inserts

New records must also establish ownership safely.

```sql
-- allow users to create documents only for themselves
create policy "users create own documents"
on documents
for insert
with check (auth.uid() = owner_id);
```

The policy rejects an insert where the client attempts to assign another
user's identity as the owner.

## Authorize updates

Users can update resources they own.

```sql
-- allow users to update only documents they own
create policy "users update own documents"
on documents
for update
using (auth.uid() = owner_id)
with check (auth.uid() = owner_id);
```

The `using` condition controls which existing rows can be modified.

The `with check` condition controls the resulting row.

Both matter.

## Authorize deletes

Deletion requires its own policy.

```sql
-- allow users to delete only documents they own
create policy "users delete own documents"
on documents
for delete
using (auth.uid() = owner_id);
```

A user who can read a document is not automatically granted delete access.

## Test ownership through the application

The client can request a document by identifier.

```typescript
// describe the identifier supplied by the browser
type DocumentRequest = {
    documentId: string;
};
```

The server can query the resource normally when the database policy enforces
ownership.

```typescript
// retrieve one document through the authenticated Supabase context
async function getDocument(input: DocumentRequest) {
    // query the requested document
    const result = await supabase
        .from("documents")
        .select("id, title, created_at")
        .eq("id", input.documentId)
        .maybeSingle();

    // reject database failures
    if (result.error) {
        throw new Error("document lookup failed");
    }

    // return only the authorized record
    return result.data;
}
```

The important property is that changing `documentId` does not bypass the RLS
policy.

## Add organization membership

Multi-tenant systems require a second relationship.

```sql
-- represent membership between an identity and an organization
create table organization_members (
    organization_id uuid not null,
    user_id uuid not null references auth.users(id),
    role text not null,
    primary key (organization_id, user_id)
);
```

A user can now have different roles in different organizations.

```text
user
  |
  +--> organization A -> manager
  |
  +--> organization B -> member
```

## Associate resources with organizations

The resource needs to belong to an organization.

```sql
-- add the organization boundary to documents
alter table documents
add column organization_id uuid not null;
```

The authorization decision can now consider both user membership and resource
ownership.

## Define an organization policy

A simplified membership policy can allow organization members to read
organization documents.

```sql
-- allow members to read documents in their organizations
create policy "members read organization documents"
on documents
for select
using (
    exists (
        select 1
        from organization_members membership
        where membership.organization_id = documents.organization_id
          and membership.user_id = auth.uid()
    )
);
```

The database now checks the relationship between the authenticated user and
the resource's organization.

## Protect role changes

A normal profile update should not allow a client to change its own role.

```typescript
// describe mutable profile data without security-sensitive fields
type ProfileUpdate = {
    displayName: string;
};
```

Do not accept:

```typescript
// do not make security-sensitive authorization data client-controlled
type UnsafeProfileUpdate = {
    displayName: string;
    role: string;
};
```

Role changes should be separate privileged operations.

## Protect an administrative operation

A server-side operation can establish the authenticated identity before
performing an administrative action.

```typescript
// require an authenticated identity before an administrative operation
async function requireAuthenticatedUser() {
    // retrieve the current authenticated identity
    const result = await supabase.auth.getUser();

    // reject requests without valid authentication
    if (result.error || !result.data.user) {
        throw new Error("authentication required");
    }

    // return the trusted identity
    return result.data.user;
}
```

The application can then perform an authorization lookup.

```typescript
// require membership in a specific organization
async function requireOrganizationRole(
    organizationId: string,
    requiredRole: string,
) {
    // establish the authenticated identity
    const user = await requireAuthenticatedUser();

    // retrieve the user's membership in the requested organization
    const result = await supabase
        .from("organization_members")
        .select("role")
        .eq("organization_id", organizationId)
        .eq("user_id", user.id)
        .maybeSingle();

    // reject failed authorization lookup
    if (result.error) {
        throw new Error("authorization lookup failed");
    }

    // reject missing membership
    if (!result.data) {
        throw new Error("forbidden");
    }

    // reject insufficient privileges
    if (result.data.role !== requiredRole) {
        throw new Error("forbidden");
    }

    // return the authenticated identity
    return user;
}
```

The server establishes both identity and resource-scoped authorization.

## Protect privileged Edge Functions

An Edge Function using a service-role credential must not equate possession
of that credential with permission of the caller.

```typescript
// retrieve the authenticated caller before privileged work
const result = await supabase.auth.getUser();

// reject unauthenticated requests
if (result.error || !result.data.user) {
    throw new Error("authentication required");
}
```

Only after authorization should a privileged client be used.

```typescript
// read the privileged credential from the server environment
const serviceRoleKey = Deno.env.get("SUPABASE_SERVICE_ROLE_KEY")!;

// create the privileged server-side client
const adminClient = createClient(
    supabaseUrl,
    serviceRoleKey,
);
```

The privileged client is a tool used after the authorization decision, not
the authorization decision itself.

## Test cross-user access

A useful authorization test attempts to cross an ownership boundary.

```typescript
// request a document using an identifier belonging to another user
const result = await supabase
    .from("documents")
    .select("id, title")
    .eq("id", otherUsersDocumentId)
    .maybeSingle();
```

The expected result under the ownership policy is that the unauthorized
resource is not returned.

The exact response behavior should match the application's API contract.

## Test privilege escalation

A security test should also attempt to modify authorization data through an
ordinary update path.

```typescript
// attempt to change a role through an ordinary profile operation
const result = await supabase
    .from("profiles")
    .update({
        display_name: "Updated",
        role: "admin",
    })
    .eq("id", currentUserId);
```

The application should not expose a general profile update operation that
permits this security-sensitive field.

## Test tenant isolation

Test that an identity from one organization cannot access another
organization's resources.

```typescript
// request a resource belonging to another organization
const result = await supabase
    .from("documents")
    .select("id, organization_id, title")
    .eq("organization_id", anotherOrganizationId);
```

The authorization policy must prevent unauthorized rows from being exposed.

## Now type it again

Reconstruct the core ownership policy.

```sql
-- enable row level security
alter table documents enable row level security;

-- authorize reads by authenticated ownership
create policy "users read own documents"
on documents
for select
using (auth.uid() = owner_id);

-- authorize writes by authenticated ownership
create policy "users update own documents"
on documents
for update
using (auth.uid() = owner_id)
with check (auth.uid() = owner_id);
```

Then reconstruct the server identity boundary.

```typescript
// retrieve the current authenticated identity
const result = await supabase.auth.getUser();

// reject requests without valid authentication
if (result.error || !result.data.user) {
    throw new Error("authentication required");
}

// use the authenticated identity for authorization
const user = result.data.user;
```

Finally, remember the security rule:

```text
authentication
    !=
authorization

authenticated identity
    +
resource relationship
    +
operation
    =
authorization decision
```

## Wrap up

The flow: identity -> resource relationship -> operation -> policy -> access.
