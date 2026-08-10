# Web security foundations - concepts

Web security is the practice of keeping application data, actions, and
infrastructure inside their intended trust boundaries. A secure web
application does not depend on one feature or library. It combines controls
at the browser, application, identity, database, storage, network, and
deployment layers.

For this course, assume a TanStack Start application deployed on Vercel.
Supabase provides authentication, PostgreSQL, Storage, Edge Functions, and
Realtime. The browser is untrusted, the application server is a controlled
boundary, and Supabase is an external service boundary.

## Start with assets

An asset is something the application needs to protect or keep under control.
User records, organization data, access tokens, uploaded documents, database
records, and administrative actions are examples. Availability can also be
an asset when an attacker can prevent legitimate users from using the
application.

The first question is not "what attack can happen?" It is "what must remain
protected?" If a user owns one organization's records, those records are an
asset whose access must be restricted to the appropriate organization.

A useful first model is:

```text
user identity -> organization membership -> application data
````

The relationship between these values matters more than any individual
screen. A page can hide another organization's records while the server still
returns them to a modified request. Security therefore starts with protected
assets and their allowed relationships.

## Identify trust boundaries

A trust boundary is a point where data, authority, or assumptions cross from
one security context into another. Web applications contain several of them.

A browser is controlled by the user. It can send arbitrary requests, modify
JavaScript-visible state, replay requests, and ignore interface restrictions.
The browser must therefore never be the final authority for permissions.

The TanStack Start server runs outside the user's direct control. It can
inspect authenticated identity, perform server-side authorization, and call
trusted services. It is a security boundary, but code running there must
still validate data received from the browser.

Supabase introduces additional boundaries. Auth establishes identity.
PostgreSQL stores application data. Row Level Security can enforce database
access. Storage controls object access. Edge Functions provide server-side
execution. Realtime can expose database changes to connected clients.

A simplified boundary looks like this:

```text
browser
    |
    v
TanStack Start
    |
    +------> Supabase Auth
    |
    +------> Supabase Postgres
    |
    +------> Supabase Storage
    |
    +------> Supabase Edge Functions
    |
    +------> Supabase Realtime
```

Every arrow is a potential trust transition. Data crossing that transition
must be treated according to the authority available at the destination.

## Treat the browser as untrusted

The browser is useful for presentation and interaction, but it is not an
authority boundary. A user can alter form values, call an endpoint directly,
change request parameters, disable JavaScript, or reproduce a request with a
different client.

Suppose an interface sends this value:

```text
organization_id=customer-a
```

The server cannot assume that the value represents the organization belonging
to the authenticated user. The client supplied it.

The server must establish the relationship independently:

```text
authenticated user
        |
        v
organization membership
        |
        v
requested resource
```

Client-side checks still have value. They improve usability and prevent
obvious mistakes. They do not replace server-side security controls.

The same principle applies to hidden fields, disabled buttons, route guards,
frontend role checks, and values stored in browser storage. They can shape the
interface, but they cannot establish authority by themselves.

## Map the attack surface

The attack surface is the collection of places where an attacker can
interact with the application or influence data.

For a TanStack Start application, this can include pages, server routes,
forms, query parameters, request bodies, cookies, file uploads, API
endpoints, webhooks, authentication flows, realtime channels, storage
objects, and server-side integrations.

Supabase adds additional surfaces. Database functions, Storage policies,
Realtime subscriptions, Edge Functions, Auth configuration, and database
extensions can all affect the security boundary.

A simple inventory might look like this:

```text
browser routes
server routes
authentication
database queries
storage objects
realtime subscriptions
edge functions
webhooks
third-party services
deployment configuration
```

An endpoint that appears harmless can still expose an asset if it accepts an
attacker-controlled identifier. A file upload can become dangerous when its
contents are later interpreted by another component. A realtime subscription
can become a data disclosure if authorization is not applied to the records
being delivered.

Security review therefore starts by finding interaction points, not by
searching for a particular vulnerability name.

## Separate identity from authority

Authentication answers one question:

```text
Who is this user?
```

Authorization answers another:

```text
What is this user allowed to do?
```

Supabase Auth can establish an authenticated identity. It does not by itself
define every business permission in the application.

Consider a user who is authenticated as:

```text
user_id = 42
```

That identity does not automatically imply:

```text
can_delete_invoice = true
```

The application still needs an authorization model. It may consider
organization membership, role, resource ownership, workflow state, or another
business rule.

A useful security flow is:

```text
authenticate
    ->
identify
    ->
authorize
    ->
perform action
```

Skipping authorization because authentication succeeded is a common source of
access-control failures.

## Use defense in depth

Defense in depth means that important security properties should not depend
on a single control when additional independent controls are practical.

Suppose an application protects organization data. A reasonable design can
enforce the boundary in several places:

```text
TanStack Start authorization
        +
Supabase database authorization
        +
appropriate data model
```

The controls should have distinct responsibilities. Application code can
reject an invalid operation early. PostgreSQL Row Level Security can provide
a database-level boundary. The data model can make invalid relationships
harder to express.

Defense in depth does not mean copying the same check everywhere without
reason. Each layer should protect a meaningful boundary.

## Apply least privilege

Least privilege means giving a component only the authority required for its
job.

A browser should not receive a Supabase service role key. A user should not
receive unrestricted database credentials. A server function should not have
access to unrelated secrets merely because the deployment environment makes
them available.

A useful distinction is:

```text
public client
authenticated user
server application
privileged server operation
```

Each context should receive only the credentials and permissions it needs.

This becomes especially important with Supabase. A privileged server-side
credential can bypass protections that are appropriate for ordinary clients.
Such credentials therefore belong only in controlled server environments and
must never be exposed to browser code.

## Prefer secure defaults

A secure default is a behavior that protects the application when a developer
does nothing special.

Examples include denying access until permission is established, requiring
authentication for protected operations, keeping secrets outside client
bundles, and rejecting malformed input.

The opposite pattern is dangerous:

```text
allow first
then add exceptions for known bad cases
```

A stronger design is:

```text
deny first
then explicitly allow valid cases
```

This principle applies to routes, database policies, storage objects,
organization membership, administrative operations, and external
integrations.

## Understand data flow

Security controls must follow data as it moves through the application.

Consider a profile update:

```text
browser
    |
    v
TanStack Start
    |
    v
validation
    |
    v
authentication
    |
    v
authorization
    |
    v
Supabase Postgres
    |
    v
response
```

The browser supplies the data. The server determines whether the request is
allowed. The database provides another boundary around stored data.

If validation happens only in the browser, the attacker can bypass it. If
authorization happens only in a UI component, the attacker can bypass it. If
the database is directly exposed without an appropriate policy, an otherwise
secure-looking application layer may not protect the underlying records.

Security is therefore a property of the entire data flow.

## Model failure explicitly

Security failures are not limited to successful attacks. Systems also fail
when they behave unsafely during errors, timeouts, missing credentials, or
partial operations.

A protected operation should not become publicly accessible because an
authorization lookup failed. A missing security configuration should not
silently enable a permissive mode in production.

Prefer failure behavior such as:

```text
unknown identity -> reject
unknown permission -> reject
invalid input -> reject
missing secret -> fail deployment or operation
unexpected state -> reject or stop safely
```

The exact response shown to the user can remain generic while detailed
diagnostics stay on the server.

## Think in threats, not checklists

A vulnerability checklist is useful, but it is not a security model.

Threat modeling asks what an attacker can control, what they want to affect,
which boundaries they can reach, and which assumptions protect the asset.

For example:

```text
asset:
    organization records

attacker control:
    request parameters

boundary:
    server authorization

security property:
    user can access only permitted records
```

This immediately suggests what must be tested. Changing a resource identifier
should not allow the attacker to cross an organization boundary.

The same reasoning works for files, invoices, user accounts, administrative
actions, realtime events, and external API operations.

## Build security into architecture

Security should not be added only after the application is complete. The
architecture determines which controls are possible and where they can be
enforced.

A useful baseline for this course is:

```text
browser
    |
    v
TanStack Start
    |
    +--> Supabase Auth
    |
    +--> Postgres + Row Level Security
    |
    +--> Storage policies
    |
    +--> Edge Functions
    |
    +--> Realtime authorization
    |
    v
Vercel deployment
```

The application layer coordinates requests. Supabase provides several
security enforcement points. Vercel provides the deployment boundary. The
developer remains responsible for deciding which component is trusted, which
data crosses each boundary, and which authority is granted at each point.

## A practical security sequence

For a new feature, work through the following reasoning sequence.

```text
identify the asset
    ->
identify the actor
    ->
identify attacker-controlled input
    ->
map the trust boundaries
    ->
authenticate the actor
    ->
authorize the requested action
    ->
validate and constrain data
    ->
perform the operation
    ->
enforce storage-level boundaries
    ->
return the minimum necessary data
    ->
log important security events
```

This sequence is not a replacement for detailed security testing. It is a
development habit that makes security properties explicit before code grows
around unsafe assumptions.

## Next step

Now type the code version of this lesson.

```
```

