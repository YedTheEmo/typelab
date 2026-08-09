# TanStack Start database access - concepts

A full-stack application eventually needs persistent data.

For example:

```text
users
posts
orders
products
sessions
```

A database is a server-side resource. The browser should not connect directly
to it when doing so would expose database credentials or bypass the
application's authorization rules.

The basic architecture is:

```text
browser
   |
   v
route / server function
   |
   v
server-side application code
   |
   v
database
```

TanStack Start does not require one particular database or ORM. You can use
the database tooling appropriate for your application.

The important Start concept is the execution boundary around that tooling.

## Database code belongs on the server

A database connection normally requires credentials.

For example:

```text
DATABASE_URL
```

That value belongs in the server environment.

It must not become browser-accessible application data.

The safe architecture is:

```text
server
  |
  +-- DATABASE_URL
  |
  +-- database client
  |
  +-- queries
  |
  v
database
```

The browser receives only the data the application chooses to return.

## Load database data through a route loader

A route loader is a natural place to obtain data required by a route.

Conceptually:

```text
request
   |
   v
route
   |
   v
loader
   |
   v
database query
   |
   v
route data
   |
   v
component
```

For example, a users route might load its users from a database rather than
from an in-memory array.

The component should not need to know how the database query works.

It consumes the route data:

```tsx
const users = Route.useLoaderData()
```

This preserves the separation:

```text
loader -> obtain data
component -> display data
```

## Server functions for database mutations

Reading route data and changing persistent state are different operations.

A mutation can use a server function:

```text
form
  |
  v
server function
  |
  v
database INSERT
```

The database operation stays inside the server boundary.

The browser sends the intended data, not a database query.

## Database client lifetime

Database clients often have connection-management concerns.

The application should avoid creating a completely new database connection
for every small operation without understanding the database driver's
connection model.

The appropriate client setup depends on the database library, deployment
environment, and whether the application runs as a long-lived server or in
an environment where instances are frequently created and destroyed.

This is an infrastructure concern rather than a special TanStack Router
feature.

The important Start principle is that the database client belongs to server
code.

## Queries and loaders

A database-backed loader might conceptually look like:

```tsx
loader: async () => {
    return db.user.findMany()
}
```

The loader is responsible for obtaining the route's data.

The database library is responsible for communicating with the database.

The router is responsible for coordinating the loader with route loading.

These are separate responsibilities:

```text
router
   |
   v
loader
   |
   v
ORM / query builder
   |
   v
database
```

## Query only what the route needs

A server does not need to send every database column to the browser.

Suppose a database record contains:

```text
id
name
email
passwordHash
internalNotes
```

A public user list may need only:

```text
id
name
```

The server should select and return the appropriate representation.

This is both a performance concern and a security concern.

A database object and an API response are not necessarily the same thing.

## Authentication and database access

Database access frequently depends on the current user.

For example:

```text
current user
    |
    v
authorization
    |
    v
database query
```

The query should be constrained by the authorization rules.

For example, a user requesting their own profile should not be able to
simply provide another user's ID and retrieve that record.

The server must establish:

```text
who is the user?
        |
        v
what may they access?
        |
        v
what query is allowed?
```

Authorization should influence the database operation itself.

## Never trust IDs from the browser

Suppose the browser submits:

```text
userId = 42
```

The server should not interpret that as:

```text
the caller is allowed to access user 42
```

It only means:

```text
the caller requested user 42
```

The server must separately determine whether that operation is permitted.

This distinction becomes critical for every database-backed mutation.

## Database errors

Database operations can fail.

Examples include:

```text
connection failure
constraint violation
timeout
duplicate value
transaction failure
```

The server should handle these failures at an appropriate boundary.

Internal database details should generally not be returned directly to users.

Instead, the application can translate expected failures into meaningful
application errors while logging detailed diagnostics on the server.

## Transactions

Some mutations involve multiple database operations that must succeed or
fail together.

For example:

```text
create order
    |
    +-- create order record
    |
    +-- decrement inventory
    |
    +-- create payment record
```

If only some operations succeed, the database can become inconsistent.

A transaction can provide atomicity:

```text
BEGIN
   |
   +-- operation A
   +-- operation B
   +-- operation C
   |
COMMIT
```

If the operation fails:

```text
ROLLBACK
```

Transactions are provided by the database or database library, not by
TanStack Start itself.

Start provides the server execution boundary in which the transaction can
run.

## Database access and SSR

Server rendering makes database-backed loaders particularly useful.

An initial request can follow:

```text
HTTP request
    |
    v
route matching
    |
    v
loader
    |
    v
database
    |
    v
route data
    |
    v
HTML
```

The browser receives the rendered result without ever receiving the
database credentials or direct database connection.

## Database access and client navigation

After hydration, navigation can cause a route loader to run again.

The architecture remains:

```text
client navigation
    |
    v
route loader
    |
    v
server-side data access
    |
    v
fresh route data
```

The exact transport and execution behavior depends on the Start application,
but the architectural boundary remains the same: database access stays on
the server.

## Database access and server functions

There are two common conceptual shapes:

```text
route loader
    |
    v
database
```

and:

```text
route loader
    |
    v
server function
    |
    v
database
```

The second is useful when the database operation is a reusable server-side
operation that is also needed elsewhere.

The first can be appropriate when the loader itself is the natural owner of
the read operation.

The choice is architectural rather than a requirement that every query pass
through another abstraction.

## The complete model

A database-backed Start application can be viewed as:

```text
browser
   |
   +----------------------+
   |                      |
   v                      v
route navigation       form mutation
   |                      |
   v                      v
loader               server function
   |                      |
   +----------+-----------+
              |
              v
       server-side logic
              |
              v
          database
              |
              v
       selected result
              |
              v
           browser
```

The key rule is simple:

```text
browser -> application server -> database
```

not:

```text
browser -> database
```

TanStack Start's server-side primitives provide the boundary; the database
library handles persistence.

The next lesson builds on this by introducing authentication and protected
routes.
