# Supabase overview - concepts

Supabase is a backend platform built around PostgreSQL.

The most important thing to understand is that Supabase is not simply a
database-as-a-service with several unrelated features attached to it.

A Supabase project gives you a PostgreSQL database and a collection of
services that integrate with that database and with each other:

```text
                         Supabase project
                               |
              +----------------+----------------+
              |                |                |
           PostgreSQL         Auth            Storage
              |                |                |
              +----------------+----------------+
                               |
              +----------------+----------------+
              |                |                |
           Realtime       Data APIs       Edge Functions
```

Every Supabase project includes a full PostgreSQL database. The database is
the foundation around which the other services are built. Supabase exposes
database functionality through its APIs, connects authentication identities
to PostgreSQL, provides Storage for files, offers Realtime communication,
and provides Edge Functions for server-side TypeScript code.

## Supabase is still PostgreSQL

The first mistake to avoid is thinking of Supabase as its own database
technology.

The database underneath Supabase is PostgreSQL.

That means the concepts you learn from PostgreSQL still matter:

```text
tables
rows
columns
primary keys
foreign keys
indexes
constraints
transactions
SQL
schemas
roles
functions
triggers
```

Supabase adds managed infrastructure and application-facing services around
that database.

This matters when deciding whether something belongs in Supabase or in your
application code.

For example, a relational constraint is naturally a database concern:

```text
users
  |
  +-- id
       |
       v
posts.user_id
```

PostgreSQL can enforce that relationship.

Your TypeScript application can then query the resulting data through
Supabase.

## The Data API

A Supabase database is not normally accessed by exposing PostgreSQL directly
to browser JavaScript.

Supabase provides generated APIs around the database, and its client
libraries provide a convenient interface for using them.

A simplified architecture looks like this:

```text
TanStack Start
      |
      v
Supabase client
      |
      v
Supabase API layer
      |
      v
PostgreSQL
```

For JavaScript applications, the main package is:

```text
@supabase/supabase-js
```

The client lets application code express operations such as:

```text
select rows
insert rows
update rows
delete rows
call database functions
authenticate
upload files
subscribe to realtime events
invoke Edge Functions
```

The same client library therefore becomes an entry point into several
different Supabase services.

## Supabase and TanStack Start

TanStack Start gives the application its full-stack web framework.

Supabase provides backend services.

These are complementary rather than competing abstractions.

A useful division is:

```text
TanStack Start
    |
    +-- routing
    +-- SSR
    +-- loaders
    +-- server functions
    +-- application UI
    |
    v
Supabase
    |
    +-- PostgreSQL
    +-- Auth
    +-- Storage
    +-- Realtime
    +-- Edge Functions
```

A route loader might retrieve database data from Supabase.

A server function might perform a mutation.

Authentication can establish the current user.

Storage can hold uploaded images.

Realtime can notify connected clients about changes.

An Edge Function can provide a globally distributed server-side endpoint.

The application can therefore use both systems without forcing one to replace
the other.

## Browser clients and server clients

A TanStack Start application runs in more than one environment.

Some code executes in the browser.

Other code executes on the server.

Supabase therefore needs to be integrated with both environments appropriately.

The current Supabase TanStack Start integration uses `@supabase/ssr` to create
a browser client and a server client, with the server client handling the
cookie-based session integration needed for server-side rendering.

Conceptually:

```text
browser
   |
   v
browser Supabase client
   |
   v
Supabase

server
   |
   v
server Supabase client
   |
   v
Supabase
```

They are clients for the same project, but they operate in different
execution environments.

The distinction becomes particularly important once authentication is
introduced.

## Publishable keys and server secrets

A Supabase application needs configuration containing its project URL and a
key.

The publishable key is intended for application use and is not equivalent to
a privileged secret.

The important security model is:

```text
browser
   |
   +-- project URL
   +-- publishable key
   |
   v
Supabase
   |
   v
RLS policies
```

The browser can safely interact with Supabase only because the database and
its access policies are configured to restrict what that client is allowed
to do.

A privileged server credential is different.

It belongs exclusively on trusted server infrastructure and must never be
embedded into browser code.

This distinction becomes much more important in the database and security
lessons.

## Row Level Security

Supabase's database model is designed to support direct client access while
still enforcing database-level authorization.

The central mechanism is PostgreSQL Row Level Security, commonly called RLS.

Without RLS, exposing a database operation to an untrusted client can create
a serious security problem.

With RLS, PostgreSQL can evaluate whether a particular operation is allowed
for the current request.

Conceptually:

```text
browser request
      |
      v
Supabase API
      |
      v
PostgreSQL
      |
      v
RLS policy
      |
   +--+--+
   |     |
 allow  reject
```

This is one of the defining differences between simply hosting a PostgreSQL
database and using Supabase as an application backend.

Supabase's database documentation specifically recommends understanding RLS
before exposing tables to application clients.

RLS will therefore receive its own lesson later in this track.

## Authentication

Supabase Auth handles identity.

It provides authentication APIs for things such as password-based login,
passwordless authentication, OAuth, and other identity-provider workflows.

Its architecture is connected to the same PostgreSQL database used by the
rest of the Supabase project.

The important conceptual chain is:

```text
user
 |
 v
Supabase Auth
 |
 v
authenticated identity
 |
 v
database policies
 |
 v
application data
```

Authentication answers:

```text
Who is this user?
```

RLS and authorization answer:

```text
What may this user access?
```

These are separate concerns.

## Storage

Supabase Storage is for files rather than relational records.

A typical application might store:

```text
PostgreSQL
    users
    posts
    products

Storage
    avatars
    product-images
    attachments
```

The database can store metadata describing a file while Storage holds the
actual object.

For example:

```text
products
    |
    +-- id
    +-- name
    +-- image_path
              |
              v
         Storage object
```

Storage is integrated with PostgreSQL and its access policies, allowing file
access to participate in the application's authorization model.

## Realtime

Traditional database access follows this model:

```text
client
   |
   v
request
   |
   v
database
   |
   v
response
```

Realtime adds another direction:

```text
database / event
       |
       v
Supabase Realtime
       |
       v
connected clients
```

Supabase Realtime supports several mechanisms, including Broadcast, Presence,
and Postgres Changes. Broadcast is useful for application events such as
messaging and collaborative interactions, Presence tracks client state, and
Postgres Changes can expose database changes to subscribed clients.

Realtime is therefore not simply "database polling."

It gives clients a mechanism for receiving events as they happen.

## Edge Functions

Supabase Edge Functions are server-side TypeScript functions running on
Supabase's Deno-compatible Edge Runtime.

They are useful when an application needs a server-side HTTP endpoint without
turning that operation into browser code.

For example:

```text
browser
   |
   v
Edge Function
   |
   +-- external API
   +-- Supabase Auth
   +-- PostgreSQL
   +-- Storage
```

Common use cases include webhooks, third-party integrations, transactional
email, lightweight AI operations, and other short-lived server-side work.
Supabase distributes these functions globally and provides local development
through the Supabase CLI.

This is particularly useful when the operation needs a secret that should
never reach the browser.

## Where Edge Functions fit with TanStack Start

TanStack Start already has server-side execution.

That means an Edge Function is not automatically the answer whenever you need
server-side code.

A useful distinction is:

```text
TanStack Start server code
    |
    +-- application-specific server behavior
    +-- route loaders
    +-- server functions
    |
    v
application server

Supabase Edge Function
    |
    +-- globally distributed endpoint
    +-- webhook receiver
    +-- Supabase-centric backend operation
    +-- external integration
    |
    v
Supabase Edge Runtime
```

If an operation is tightly coupled to a TanStack Start route, a Start server
function may be the natural choice.

If it needs to exist as an independently callable HTTP endpoint, receive a
webhook, or run within Supabase's globally distributed function environment,
an Edge Function can be more appropriate.

## The complete Supabase model

The most useful mental model is therefore not:

```text
Supabase = database
```

but:

```text
                         PostgreSQL
                              |
        +---------------------+---------------------+
        |                     |                     |
       Auth                Storage               Realtime
        |                     |                     |
        +---------------------+---------------------+
                              |
                         Data APIs
                              |
                       Edge Functions
                              |
                         Your application
```

PostgreSQL is the foundation.

Auth establishes identity.

RLS controls database access.

Storage handles files.

Realtime handles live communication.

The Data API exposes database functionality to applications.

Edge Functions provide server-side TypeScript execution.

TanStack Start then sits on top of these services as the application's web
framework:

```text
browser
   |
   v
TanStack Start
   |
   +-- routes
   +-- SSR
   +-- loaders
   +-- server functions
   |
   v
Supabase
   |
   +-- PostgreSQL
   +-- Auth
   +-- Storage
   +-- Realtime
   +-- Edge Functions
```

Once this architecture is understood, the rest of the Supabase track becomes
a matter of learning each major service and how it participates in this
system.

## Next step

The next lesson goes underneath the platform and focuses on the PostgreSQL
database that forms the foundation of Supabase.

