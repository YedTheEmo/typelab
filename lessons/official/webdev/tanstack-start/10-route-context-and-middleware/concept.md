# TanStack Start route context and middleware - concepts

A route does not exist in isolation.

Routes form a hierarchy:

```text
root
 |
 +-- dashboard
      |
      +-- settings
      |
      +-- profile
```

Sometimes every route underneath a particular point in this hierarchy needs
access to the same application-level information.

Examples include:

```text
authenticated user
request information
logging utilities
database access
application configuration
```

TanStack Router provides route context for passing such values through the
route hierarchy.

Middleware addresses a related but different problem: executing logic around
a request or server operation.

The distinction is important:

```text
context   -> provides values
middleware -> intercepts / wraps execution
```

## Route context

Context is data made available to routes in the route hierarchy.

Conceptually:

```text
root route
    |
    | context
    v
child route
    |
    | context
    v
grandchild route
```

A descendant route can use values established by an ancestor.

This is useful when the value is application infrastructure rather than URL
state.

For example, an authenticated user is not naturally represented as:

```text
/dashboard?user=42
```

Instead, authentication state can be established at the application
boundary and made available through context.

## Context is not global state

Route context can look similar to a global store because many descendants
can access it, but the concepts are different.

A global store represents application state that may be read or changed from
many unrelated parts of the application.

Route context is associated with the router's matched route hierarchy.

That distinction makes context particularly useful for values needed by route
loading and routing logic.

Think of it as:

```text
global store
    -> application-wide state

route context
    -> values associated with the route tree
```

## Root context

The root route is a natural place to establish values needed by many
descendant routes.

For example:

```text
root
 |
 +-- auth
 |
 +-- services
 |
 +-- configuration
 |
 +-- child routes
```

A child route can then depend on those values without recreating them.

This is especially useful for authentication-related routing because many
routes may need to know whether a user exists.

## Context and loaders

Context becomes particularly useful when a loader needs infrastructure
provided higher in the route tree.

Conceptually:

```text
root context
     |
     v
child loader
     |
     v
server operation
```

A loader can therefore use route context when deciding what data to load.

This is different from `loaderDeps`.

`loaderDeps` describes values whose changes affect a loader's result.

Context provides the values the loader can use while executing.

## Context and authentication

Authentication provides a useful example.

Suppose the root route establishes information about the current session.

A protected route can then inspect that information:

```text
request
   |
   v
root
   |
   +-- session
   |
   v
dashboard
   |
   +-- protected loader
```

The protected route can decide whether the current request is allowed to
continue.

However, context itself is not the authentication system.

The actual authentication mechanism still has to establish the user's
identity.

Similarly, checking context in a UI component is not sufficient security
for a sensitive server operation.

The server must enforce authorization at the server boundary.

## Middleware

Middleware is about execution rather than simply providing values.

A middleware function can conceptually surround another operation:

```text
incoming operation
       |
       v
middleware
       |
       v
handler
       |
       v
middleware
       |
       v
result
```

This makes middleware useful for concerns that should happen consistently
around many operations.

Examples include:

```text
logging
authentication
timing
request metadata
authorization checks
```

Instead of duplicating the same logic in every handler, middleware can
establish a common execution pipeline.

## Middleware is not a component

Middleware does not exist primarily to render UI.

A component describes what should be rendered.

Middleware describes what should happen around an operation.

For example:

```text
component
    -> render UI

middleware
    -> inspect / modify / guard execution
```

Keeping these concepts separate prevents server-side request concerns from
leaking into presentation code.

## Middleware ordering

When multiple middleware functions are applied, their ordering matters.

Conceptually:

```text
request
  |
  v
middleware A
  |
  v
middleware B
  |
  v
handler
```

and then the result travels back through the middleware chain.

This resembles nested function calls:

```text
A(
    B(
        handler()
    )
)
```

An authentication middleware placed before an authorization check can
establish the identity that the authorization check needs.

Therefore middleware is not merely a list of unrelated callbacks; it forms
an execution pipeline.

## Middleware and server functions

Server functions are explicit server-side operations.

Middleware can surround those operations or participate in request handling,
depending on the application's architecture.

The distinction remains:

```text
server function
    -> defines the operation

middleware
    -> defines behavior around operations
```

For example:

```text
request
   |
   v
authentication middleware
   |
   v
server function
   |
   v
database operation
```

The middleware establishes or verifies request-level conditions, while the
server function performs the actual operation.

## Context versus middleware

These are easiest to understand by asking different questions.

Context asks:

```text
"What values should this route hierarchy have access to?"
```

Middleware asks:

```text
"What should happen before, after, or around this operation?"
```

For example:

```text
context:
    current session

middleware:
    reject unauthenticated requests
```

The session value and the authentication policy are related, but they are
not the same abstraction.

## The complete model

The route tree can therefore be understood as an execution structure:

```text
root
 |
 +-- context
 |
 +-- middleware
 |
 +-- child route
       |
       +-- context
       |
       +-- middleware
       |
       +-- loader
       |
       +-- component
```

This gives the application a structured way to organize infrastructure and
cross-cutting behavior instead of placing everything inside individual
components.

## The important boundary

Route context is convenient for routing logic, but security decisions must
ultimately be enforced where the protected resource is accessed.

For example:

```text
UI check
    X
    |
server resource

server authorization
    |
    v
server resource
```

A user can manipulate browser code, URLs, and requests.

They cannot be trusted simply because the application's UI would normally
prevent an operation.

Therefore authentication and authorization remain server concerns whenever
the operation protects server resources.

## Summary

Route context provides shared values through the route hierarchy.

Middleware provides reusable execution behavior around requests or server
operations.

Together they allow a Start application to structure cross-cutting concerns
without turning every component or loader into a mixture of routing,
authentication, logging, and infrastructure code.

The next lesson moves from routing infrastructure into database access,
where these server-side boundaries become practically important.
