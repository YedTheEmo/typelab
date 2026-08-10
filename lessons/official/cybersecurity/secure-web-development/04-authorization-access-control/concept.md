# Authorization and access control - concepts

Authentication establishes identity. Authorization determines what an
authenticated identity is allowed to access or change.

This distinction is one of the most important security concepts in web
development. A correctly authenticated user can still be an attacker against
another user's data if authorization is implemented incorrectly.

For this course, assume a TanStack Start application deployed on Vercel with
Supabase Auth, PostgreSQL, Storage, Edge Functions, and Realtime.

## Authentication is not authorization

Authentication answers:

```text
Who are you?
```

Authorization answers:

```text
What are you allowed to do?
```

For example, Supabase Auth may establish:

```text
user.id = 123
```

That does not mean the user may access every record associated with user
123, every organization, every file, or every administrative operation.

The request must pass both boundaries:

```text
request
  |
  v
authenticated identity
  |
  v
authorization decision
  |
  v
resource
```

An application that verifies identity but skips authorization is still
insecure.

## Deny by default

Authorization should begin with no access and explicitly grant required
capabilities.

Conceptually:

```text
unknown request
    |
    v
deny
    |
    +--> explicit permission
             |
             v
            allow
```

Avoid architectures where every authenticated user receives access and
individual routes attempt to remember which users should be blocked.

A deny-by-default model makes missing rules fail toward rejection rather than
exposure.

## Establish the resource boundary

Authorization is always about a resource or operation.

Examples include:

```text
profile
organization
client
order
background-check report
document
storage object
billing record
administrative action
```

A useful authorization question is:

```text
Who may perform which operation on which resource under which conditions?
```

This is more precise than asking whether a user is "allowed."

## Object ownership

A common authorization model is ownership.

For example:

```text
user A
  |
  +--> profile A
  +--> document A
  +--> order A

user B
  |
  +--> profile B
  +--> document B
  +--> order B
```

User A should not be able to replace an identifier in a request and retrieve
user B's resource.

The server or database must establish the ownership relationship.

This is the core defense against insecure direct object references, commonly
called IDOR.

## IDOR

An IDOR vulnerability occurs when an application accepts a resource
identifier from a client but fails to verify that the authenticated identity
is authorized to access that resource.

An unsafe request might look like:

```text
GET /orders/1001
```

The application authenticates the user and then retrieves order 1001 without
checking ownership.

Changing the identifier to:

```text
GET /orders/1002
```

may expose another user's order.

The identifier itself is not the problem. The missing authorization check is.

The secure relationship is:

```text
authenticated identity
        +
requested resource
        |
        v
authorization check
        |
        v
resource access
```

## Never authorize from the frontend

A frontend can hide a button:

```text
if (user.isAdmin) {
    showDeleteButton();
}
```

This improves the interface. It does not provide security.

An attacker can call the underlying endpoint directly.

Frontend state is untrusted input:

```text
browser
   |
   +-- hidden button
   +-- modified JavaScript
   +-- forged request
   +-- altered user role
   |
   v
server
```

The server or underlying data service must enforce the actual permission.

## Role-based access control

Role-based access control, or RBAC, assigns permissions through roles.

A simplified model is:

```text
user
  |
  v
role
  |
  v
permissions
```

For example:

```text
member
    read own resources

manager
    read organization resources
    manage organization resources

admin
    manage administrative resources
```

Roles are useful when permissions naturally group into stable sets.

The application should not assume that a role name alone is enough. The
resource scope still matters.

A manager of Organization A is not automatically a manager of Organization B.

## Resource-scoped authorization

Real applications frequently require permissions scoped to an organization,
tenant, project, or other resource boundary.

For example:

```text
user
  |
  +--> organization A --> manager
  |
  +--> organization B --> member
```

The same user therefore has different capabilities depending on the
resource being accessed.

A global `isAdmin` flag cannot represent every authorization model.

Authorization often needs both:

```text
identity
    +
resource relationship
    +
operation
```

## Multi-tenant authorization

A white-label or SaaS application usually has a tenant boundary.

Conceptually:

```text
Tenant A
  +-- users
  +-- clients
  +-- orders
  +-- documents

Tenant B
  +-- users
  +-- clients
  +-- orders
  +-- documents
```

The most important rule is that a request operating inside Tenant A must not
accidentally read or modify Tenant B data.

Tenant isolation should be enforced at a trusted server or data boundary.

Do not depend solely on the frontend selecting the correct tenant.

## Row Level Security

Supabase PostgreSQL provides Row Level Security, commonly abbreviated RLS.

RLS can enforce authorization at the database boundary.

A conceptual policy might say:

```text
authenticated user
    |
    v
row.owner_id = auth.uid()
    |
    v
allow
```

This is valuable because application bugs elsewhere cannot simply bypass the
database policy when the same database access path remains subject to RLS.

RLS should be treated as an authorization layer, not merely a database
feature.

## Use policies that match business rules

A policy should express the actual access requirement.

For ownership:

```sql
using (auth.uid() = owner_id)
```

For organization membership:

```text
authenticated user
    |
    v
membership
    |
    v
organization_id
    |
    v
resource.organization_id
```

The important question is whether the policy correctly represents the
business boundary.

A syntactically valid policy can still be logically incorrect.

## Separate read and write authorization

Reading and modifying a resource are not necessarily equivalent.

A user might be allowed to:

```text
read document
```

but not:

```text
delete document
```

Likewise, a user may be allowed to create records but not modify records
created by another user.

Authorization should therefore consider the operation:

```text
select
insert
update
delete
```

and any higher-level application actions.

## Understand insert authorization

For a new record, the application must establish who owns it.

Unsafe:

```text
client submits owner_id
        |
        v
database accepts owner_id
```

The client could submit another user's identifier.

Safer:

```text
authenticated identity
        |
        v
server/database derives owner
        |
        v
new record
```

When the client does provide an ownership field, the server must validate it
against the authenticated identity and permitted context.

## Understand update authorization

Updates have two questions:

```text
Who may update this existing row?
```

and:

```text
What values may they change?
```

An authorization policy that allows a user to update their own profile does
not necessarily mean they should be allowed to change fields such as:

```text
role
organization_id
account_status
billing_status
```

Authorization and field-level integrity can therefore overlap.

Sensitive fields should be controlled explicitly.

## Understand delete authorization

Deletion is usually an explicit privileged operation.

A user may have permission to update a resource without permission to delete
it.

Deletion may also require additional business rules:

```text
resource owner
+
resource state
+
role
```

For high-impact resources, consider whether permanent deletion should be
available at all or whether the application should use a controlled state
transition.

## Prevent privilege escalation

Privilege escalation occurs when an identity gains capabilities beyond those
intended for it.

A common example is allowing a normal user to submit:

```json
{
  "role": "admin"
}
```

and persisting that value.

The server must treat authorization-related fields as privileged.

```text
client-controlled fields
        !=
security-sensitive fields
```

Roles, permissions, tenant membership, account status, and similar values
should only be changed through authorized operations.

## Do not trust JWT claims blindly

Tokens can contain claims such as:

```text
sub
role
aud
exp
```

Claims can be useful for authorization decisions, but the application must
understand their source, lifetime, validation, and intended meaning.

Do not allow the client to manufacture a role claim.

Do not treat a stale role claim as permanently authoritative if the
application requires immediate revocation.

When authorization depends on current database state, the architecture should
use an appropriate trusted lookup or database policy.

## Authorization through Supabase Storage

Storage objects require authorization too.

A private object should not become public merely because a client knows its
object path.

The conceptual boundary is:

```text
authenticated user
        |
        v
storage policy
        |
        v
authorized object
```

Storage paths should also avoid becoming accidental authorization mechanisms.

Knowing:

```text
tenant-a/private/report.pdf
```

should not itself grant access.

## Authorization through Realtime

Realtime connections also cross a security boundary.

A client subscribing to a channel or receiving database changes should only
receive data it is authorized to observe.

The application must consider:

```text
who can subscribe?
what channel are they joining?
what records can be delivered?
what happens when membership changes?
```

Realtime is not a reason to bypass database authorization.

## Authorization in Edge Functions

Supabase Edge Functions often perform operations that require server-side
credentials or privileged workflows.

That makes authorization inside the function critical.

A function should not assume:

```text
request reached the function
        =
request is authorized
```

The function should establish identity and verify the operation's permission
before performing privileged work.

This is especially important when a function uses a service-role credential,
because that credential can bypass ordinary RLS protections.

## Service-role credentials bypass normal client restrictions

A service-role Supabase credential is highly privileged.

If an Edge Function uses it, the function becomes a security boundary.

The flow becomes:

```text
browser
    |
    v
Edge Function
    |
    | authorization required here
    v
service-role Supabase client
    |
    v
database
```

Never allow an untrusted request to directly control a privileged operation
just because the operation is implemented inside a server function.

## Prevent confused-deputy behavior

A privileged server component can become a confused deputy when it performs
an operation using its own authority without verifying that the requesting
identity is allowed to ask for that operation.

For example:

```text
user
  |
  v
Edge Function
  |
  | service role
  v
another user's private record
```

The function may have permission while the user does not.

The function must explicitly bridge those two identities:

```text
requesting identity
        +
requested resource
        +
requested operation
        |
        v
authorization decision
        |
        v
privileged action
```

## Avoid relying on obscurity

Random-looking IDs can make guessing harder.

They are not authorization.

For example:

```text
/order/550e8400-e29b-41d4-a716-446655440000
```

is preferable to exposing sequential identifiers in some designs, but the
identifier still needs authorization.

The security property must be:

```text
unknown identifier
    +
known identifier
```

both remain inaccessible without permission.

## Fail closed

Authorization failures should result in rejection.

Examples:

```text
unknown user -> deny
unknown role -> deny
unknown tenant membership -> deny
unknown resource ownership -> deny
missing policy context -> deny
authorization service failure -> fail safely
```

Do not convert an authorization error into an unrestricted fallback.

A system should not say:

```text
could not determine permission
        |
        v
allow
```

## Minimize authorization complexity

Complex authorization logic becomes difficult to review.

Prefer clear boundaries:

```text
identity
  +
tenant membership
  +
role
  +
resource ownership
  +
operation
```

rather than scattered checks across unrelated frontend and backend files.

Where possible, centralize policy definitions and enforce critical
boundaries at the database or server layer.

## Test authorization as an attacker

Security tests should deliberately cross boundaries.

Examples:

```text
User A requests User B's record
User A updates User B's record
Member attempts manager operation
Tenant A requests Tenant B resource
User changes their own role to admin
Unauthenticated request calls protected endpoint
Expired membership accesses resource
Deleted membership accesses resource
```

A useful test matrix is:

```text
identity | resource | operation | expected result
```

Do not test only that authorized users can perform actions. Test that
unauthorized users cannot.

## Authorization review

For every protected operation, ask:

```text
who is the authenticated identity?
what resource is being accessed?
what operation is being attempted?
what tenant or organization owns it?
what role or relationship does the identity have?
where is the decision enforced?
what happens if the decision cannot be established?
```

Then inspect every path:

```text
TanStack Start
Supabase Edge
Postgres
Storage
Realtime
```

The goal is a consistent authorization boundary rather than isolated checks.

## Security architecture

The intended flow is:

```text
browser
    |
    v
TanStack Start
    |
    +--> Supabase Auth -> identity
    |
    +--> authorization context
    |
    +--> Supabase Postgres -> RLS
    |
    +--> Supabase Storage -> policies
    |
    +--> Supabase Edge -> explicit authorization
    |
    +--> Supabase Realtime -> authorized subscriptions
```

Authentication identifies the caller. Authorization controls the caller's
relationship with each resource.

The two mechanisms should remain conceptually separate.

## Next step

Now type the code version of this lesson.
