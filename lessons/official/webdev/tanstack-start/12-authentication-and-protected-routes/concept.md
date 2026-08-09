# TanStack Start authentication and protected routes - concepts

Authentication answers:

```text
"Who is making this request?"
```

Authorization answers:

```text
"What is that user allowed to do?"
```

They are related, but they are not the same operation.

A typical authenticated application has this flow:

```text
request
   |
   v
session / credentials
   |
   v
identify user
   |
   v
authorization
   |
   v
route or server operation
```

TanStack Start provides the server-side and routing primitives around which
this system can be built, but it does not magically make an application
authenticated. The actual authentication mechanism can come from an
authentication library or from an application-specific implementation.

## Sessions

A common web authentication model uses a session.

Conceptually:

```text
browser
   |
   | session cookie
   v
server
   |
   v
session lookup
   |
   v
current user
```

The browser does not need to send the user's complete identity and
permissions with every request.

Instead, it can send a session identifier, commonly through a secure cookie,
and the server resolves that session to the authenticated user.

The important boundary is:

```text
browser -> session credential
server  -> identity + authorization
```

The server remains responsible for deciding what the session means.

## Authentication is a server concern

A component can display:

```text
Welcome, Ada
```

but that does not prove that the request is authenticated.

Likewise, this:

```tsx
if (!user) {
    return null
}
```

is only a UI decision.

It does not prevent someone from directly calling a protected server
operation.

A protected operation needs a server-side check:

```text
request
   |
   v
authenticate
   |
   +-- no user -> reject
   |
   v
authorized operation
```

## Protected routes

A protected route is a route whose access depends on authentication.

A common architecture is to establish the current user high in the route
tree and have protected descendants verify that user.

```text
root
 |
 +-- session
 |
 +-- dashboard
 |     |
 |     +-- settings
 |     +-- profile
 |
 +-- public
```

The dashboard and its descendants can require authentication while public
routes remain accessible without it.

## Redirecting unauthenticated users

A protected route commonly performs:

```text
request /dashboard
       |
       v
is user authenticated?
       |
   +---+---+
   |       |
  yes      no
   |       |
   v       v
render   redirect
```

The redirect is a routing behavior.

The authentication decision itself must come from trusted server-side
information.

## Route guards versus server authorization

These should not be confused.

A route guard can prevent an unauthenticated user from entering a UI route.

A server authorization check protects the actual operation.

For example:

```text
/dashboard
    |
    v
route guard
    |
    v
dashboard UI
    |
    v
update profile
    |
    v
server authorization
    |
    v
database
```

The route guard improves application behavior.

The server authorization check provides the actual security boundary.

Both can be useful.

## Authentication data in route context

Once the server has determined the current user, route context can provide
that information to the route hierarchy.

Conceptually:

```text
request
   |
   v
session lookup
   |
   v
current user
   |
   v
router context
   |
   +-- dashboard
   +-- settings
   +-- profile
```

Descendant routes can then use the current user when making routing or data
decisions.

The context itself is not the session.

It is a convenient way of making the already-established information
available to the route tree.

## Authorization

Authentication tells us:

```text
user = 42
```

Authorization asks:

```text
can user 42 modify record 91?
```

That decision may depend on:

```text
user identity
resource ownership
roles
permissions
organization membership
resource state
```

For example:

```text
user
 |
 +-- identity
 |
 +-- permissions
 |
 v
authorization
 |
 v
database operation
```

Authorization should happen before the protected resource is changed or
returned.

## Avoid client-controlled authorization

This is not authorization:

```tsx
if (user.role === "admin") {
    showDeleteButton()
}
```

It is merely conditional rendering.

A malicious client can still attempt the underlying operation.

The actual server operation must enforce:

```text
is this user allowed to delete this resource?
```

The UI check is useful because it produces a better interface, but it is not
the security mechanism.

## Authentication in loaders

A loader can use authentication information when determining which data a
route should load.

For example:

```text
authenticated user
       |
       v
loader
       |
       v
user-specific database query
```

This makes it possible to avoid loading data that the current user should not
see.

However, the underlying server operation should still enforce authorization
when it is independently reachable.

## Authentication in mutations

Mutations are particularly important because they change persistent state.

A secure mutation looks conceptually like:

```text
request
   |
   v
authenticate
   |
   v
validate input
   |
   v
authorize
   |
   v
database mutation
```

The order is important conceptually even when the exact implementation
differs.

The server should establish who is making the request and whether they may
perform the operation before changing protected data.

## Authentication failures

An unauthenticated request and an unauthorized request are different.

Unauthenticated means:

```text
"we do not have a valid authenticated identity"
```

Unauthorized means:

```text
"we know who you are, but you are not allowed to perform this operation"
```

These cases can lead to different application behavior.

For example:

```text
not authenticated
    -> redirect to login

authenticated but forbidden
    -> show forbidden response
```

The exact HTTP behavior depends on the application architecture.

## The complete model

Authentication should be thought of as a chain:

```text
browser
   |
   v
credential / session
   |
   v
server
   |
   v
authentication
   |
   v
current user
   |
   v
authorization
   |
   +--------+
   |        |
   v        v
loader   mutation
   |        |
   +----+---+
        |
        v
     database
```

The router coordinates navigation and route loading.

The server establishes identity and enforces access.

The database stores persistent state.

Keeping those responsibilities separate prevents authentication logic from
becoming merely a collection of client-side checks.
