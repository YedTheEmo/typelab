# TanStack Start server functions - concepts

A full-stack application eventually needs browser code to request work from
the server.

Examples include:

```text
create a database record
read private data
send an email
access a filesystem
use a secret API key
perform an authenticated mutation
```

The browser cannot safely perform these operations directly when they depend
on resources that must remain private.

TanStack Start provides server functions for establishing this boundary.

The fundamental model is:

```text
browser
   |
   | invoke server function
   v
server
   |
   | perform server-only work
   v
result
   |
   v
browser
```

The important part is that the function's implementation belongs to the
server even though browser code can invoke it.

## Why a server boundary is necessary

Consider a database connection.

A database credential must not be shipped to the browser:

```text
browser
    X
    |
database credential
```

If the credential becomes part of browser JavaScript, the user can inspect
it.

Instead, the browser should request the operation:

```text
browser
    |
    | "give me these records"
    v
server
    |
    | database query
    v
database
```

The server returns the appropriate result.

The credential and database implementation remain server-side.

## Creating a server function

TanStack Start uses `createServerFn` to define server functions.

A simplified example is:

```tsx
import { createServerFn } from "@tanstack/react-start"

export const getUsers = createServerFn({
    method: "GET",
}).handler(() => {
    return ["Ada", "Grace"]
})
```

The function contains server-side work.

Client code can invoke the exported function without containing the server
implementation itself.

This is the essential abstraction:

```text
createServerFn
      |
      v
server-side handler
      ^
      |
client invocation
```

## GET and POST

Server functions can specify an HTTP method.

For example:

```tsx
createServerFn({
    method: "GET",
})
```

is appropriate for retrieving information.

A mutation can use:

```tsx
createServerFn({
    method: "POST",
})
```

The distinction communicates the intended operation.

Conceptually:

```text
GET  -> retrieve
POST -> perform a mutation
```

The HTTP method is part of the server function's contract.

## Calling a server function

A server function can be called from application code.

Conceptually:

```tsx
const users = await getUsers()
```

The caller does not directly execute the server-side implementation in the
browser.

The framework handles the boundary between the two environments.

This is different from importing an ordinary function:

```tsx
import { getUsers } from "./database"
```

An ordinary function import does not inherently create a server boundary.

`createServerFn` does.

## Input

Real server functions usually need input.

For example:

```text
getUser(42)
createUser(...)
deleteUser(...)
```

Server functions can define input validation so data crossing the boundary
has a known shape.

The conceptual flow is:

```text
browser input
     |
     v
validation
     |
     v
server handler
     |
     v
result
```

Validation is particularly important because data coming from the browser
must be treated as untrusted input.

The server cannot assume that a client invocation was produced by the
application's own UI.

## Server functions and security

A server function is a server boundary, not an authorization policy.

This distinction is extremely important.

Defining:

```tsx
const getUser = createServerFn(...)
```

does not automatically mean every caller is authorized to retrieve every
user.

The server function must still enforce the application's rules.

For example:

```text
request
   |
   v
authenticate
   |
   v
authorize
   |
   v
database operation
```

Authentication determines who is making the request.

Authorization determines whether that user is allowed to perform the
operation.

A server function gives the operation a server-side execution boundary; it
does not magically establish either of those policies.

## Server functions versus loaders

Loaders and server functions solve related but different problems.

A loader belongs to a route:

```text
route
  |
  +-- loader
        |
        v
      data
```

A server function represents a server-side operation:

```text
component / loader / action
        |
        v
 server function
        |
        v
 server operation
```

A loader answers:

```text
"What data does this route need?"
```

A server function answers:

```text
"What server-side operation should be performed?"
```

A loader may itself call a server function when the route needs data that
must be obtained through a server-only operation.

## Server functions versus API endpoints

A traditional application might expose an endpoint such as:

```text
POST /api/users
```

and manually write client code that performs:

```tsx
fetch("/api/users", {
    method: "POST",
})
```

Server functions provide a higher-level abstraction around this kind of
browser-server interaction.

Instead of manually maintaining a URL, request format, and client-side
wrapper, application code can work with the server function abstraction.

The framework handles the transport mechanism.

This reduces the amount of plumbing required for internal application
operations.

## The server function handler

The handler contains the actual server-side operation.

For example:

```tsx
export const createUser = createServerFn({
    method: "POST",
}).handler(async ({ data }) => {
    return saveUser(data)
})
```

The handler receives the input and performs the operation.

The handler can therefore be the place where server-only resources are
accessed:

```text
handler
   |
   +-- database
   +-- filesystem
   +-- private service
   +-- secret credentials
```

Those resources should not be exposed to browser code.

## Returning data

A server function can return serializable data to its caller.

For example:

```tsx
return {
    id: user.id,
    name: user.name,
}
```

The browser receives the result.

The server should therefore return only the information the client actually
needs.

Returning an entire internal database object may accidentally expose fields
that should remain private.

A server boundary is also a data boundary.

## Server functions can be reused

Because a server function represents an operation rather than a specific
component, multiple parts of an application can potentially use it.

For example:

```text
dashboard
    |
    +-- getCurrentUser()

profile
    |
    +-- getCurrentUser()

settings
    |
    +-- getCurrentUser()
```

The server operation does not need to be duplicated in every component.

The route loader, event handler, or other appropriate application code can
invoke the same server-side operation.

## The complete model

The server-function architecture can be represented as:

```text
browser
   |
   | input
   v
server function boundary
   |
   v
validation
   |
   v
authentication / authorization
   |
   v
server-only operation
   |
   +-- database
   +-- private API
   +-- filesystem
   |
   v
sanitized result
   |
   v
browser
```

The most important concept is the boundary itself.

A server function is an explicit declaration that a particular operation
belongs on the server.

It allows browser code to request that operation without exposing the
implementation or its private dependencies to the browser.

## Next step

Now type the code version of this lesson.
