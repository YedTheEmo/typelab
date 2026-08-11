# Supabase database - concepts

Every Supabase project sits on top of a real PostgreSQL database.

This is not an approximation or a compatibility layer. It is the same
open-source PostgreSQL that you could run yourself.

The most important consequence is that the database is a relational
database. Its design revolves around tables, rows, columns, relationships,
and constraints.

```text
PostgreSQL
    |
    +-- tables
    +-- rows
    +-- columns
    +-- relationships
    +-- constraints
    +-- indexes
    +-- schemas
    +-- SQL
```

Everything else in Supabase builds on this foundation.

## Tables, rows, and columns

Data in a relational database is organized into tables.

A table describes one kind of thing in your application.

```text
instruments
    |
    +-- id     (one instrument)
    +-- name   (its name)
    +-- price  (its price)
```

Each table has columns, and each column stores one attribute.

Each row is one instance.

```text
instruments
    id | name   | price
   ----+--------+-------
     1 | Piano  |  499
     2 | Violin |  129
```

A table with no rows is still a valid table. The schema describes the shape;
the rows are the data.

## Choosing data types

Every column has a type.

PostgreSQL provides many types, and choosing the right one matters:

```text
bigint        -> large whole numbers
numeric       -> exact decimal values
text          -> strings of any length
uuid          -> globally unique identifiers
boolean       -> true or false
timestamptz   -> a moment in time with time zone
jsonb         -> structured JSON documents
```

A price is naturally an exact decimal:

```text
price numeric
```

A name is text:

```text
name text
```

A timestamp such as "when was this created" is a timestamptz:

```text
created_at timestamptz
```

The type tells PostgreSQL how to store the value and what operations are
valid for it.

## Primary keys

Each row needs a reliable way to be identified.

A primary key is the column that uniquely identifies a row.

```text
instruments.id  ->  one specific instrument
```

PostgreSQL enforces that a primary key is unique and never empty.

Supabase projects commonly use one of two styles:

```text
id bigint primary key generated always as identity
id uuid primary key default gen_random_uuid()
```

The first asks PostgreSQL to generate an increasing number.

The second asks PostgreSQL to generate a universally unique identifier.

Both are valid. The choice is a data-modeling decision.

## Relationships and foreign keys

Tables rarely exist in isolation.

A relationship connects a row in one table to a row in another table.

```text
artists
    +-- id
         |
         v
albums.artist_id
```

The column that holds the reference is a foreign key.

```text
artist_id uuid references artists(id)
```

PostgreSQL can enforce that every value in `albums.artist_id` actually
exists in `artists.id`.

This is a one-to-many relationship:

```text
one artist  ->  many albums
one album   ->  belongs to one artist
```

Relational modeling is about deciding where these relationships live.

## Constraints

Constraints are rules that PostgreSQL enforces for you.

Common examples:

```text
not null  ->  the value must be provided
unique    ->  the value must not repeat
check     ->  the value must satisfy an expression
default   ->  use this value when none is provided
```

A name that must exist:

```text
name text not null
```

A name that must be different for every artist:

```text
name text not null unique
```

A duration that must be positive:

```text
duration numeric check (duration > 0)
```

Constraints protect the integrity of the data no matter which client writes
to it.

## Indexes

Queries often look up rows by a column that is not the primary key.

```text
select * from albums where artist_id = ...
```

Without an index, PostgreSQL must scan every row.

An index lets PostgreSQL find the matching rows quickly.

```text
create index on albums (artist_id);
```

Indexes speed up reads but add write overhead. They are a performance
decision, not a correctness decision.

## Schemas

A database can be organized into schemas.

A schema is a namespace for tables.

```text
public   ->  application tables
auth     ->  Supabase Auth's tables
storage  ->  Supabase Storage's tables
```

A Supabase project's application tables normally live in the `public`
schema.

Supabase itself uses the other schemas.

For example, `auth.users` is the table that represents registered users.

Application code can reference it:

```text
auth.users(id)
```

## Roles

PostgreSQL controls access through roles.

A Supabase project exposes several roles that application clients use:

```text
anon            ->  unauthenticated browser requests
authenticated   ->  browser requests with a signed-in user
service_role    ->  privileged server access
```

The `anon` and `authenticated` roles are the ones browser code interacts
with.

The `service_role` is a privileged credential that belongs only on trusted
server infrastructure.

The relationship between roles and Row Level Security is central to how
Supabase authorizes requests. RLS receives its own lesson later in this
track.

## Functions and triggers

PostgreSQL can run logic inside the database.

A database function is a named, reusable SQL or PL/pgSQL routine.

A trigger runs a function automatically when an event occurs:

```text
insert
   |
   v
trigger
   |
   v
function
```

For example, a trigger could update an aggregate column whenever a row is
inserted.

Database-side logic is powerful, but it should be used deliberately. Most
application behavior still belongs in application code.

## Transactions

Some operations must succeed or fail together.

A transaction groups statements so that the database applies them as one
unit.

```text
begin
   |
   +-- insert album
   +-- insert track
   +-- insert track
   |
commit
```

If any statement fails, the database rolls the whole group back.

```text
begin
   |
   +-- insert album     (ok)
   +-- insert track     (fails)
   |
rollback
   |
   +-- album is not saved either
```

Transactions are what make multi-statement changes atomic.

## SQL as the interface

The way to describe and change a schema is SQL.

Supabase provides a SQL editor in the dashboard for running statements
directly against the database.

```text
create table ...
alter table ...
insert into ...
select ...
create index ...
```

Later in this track you will see migrations, which store these statements in
files so the schema can be version-controlled and reproduced.

## The Data API

A browser cannot connect to PostgreSQL directly.

Supabase generates a Data API around the database, and the Supabase client
turns operations into requests against that API.

```text
Supabase client
    |
    v
Supabase Data API
    |
    v
PostgreSQL
```

A table becomes something the client can query:

```text
supabase
    .from("instruments")
    .select("id, name")
```

Inserting, updating, and deleting follow the same pattern:

```text
insert rows
update rows
delete rows
```

Filters narrow the result:

```text
.eq("id", id)
```

Relationships can be fetched in the same request through nested selection.

This means the schema you design in PostgreSQL is the schema the Data API
exposes.

## Data operations through the client

Reading is not the only operation the client can express.

Each data operation maps to a familiar database concept:

```text
select  ->  read rows
insert  ->  add rows
update  ->  change rows
delete  ->  remove rows
rpc     ->  call a database function
```

Filters and modifiers shape a read operation:

```text
.eq("status", "published")   ->  where status = 'published'
.order("created_at")         ->  order by created_at
.limit(10)                   ->  limit 10
```

A mutation supplies the values to write:

```text
.insert({ name: "Piano" })
.update({ price: 549 })
.delete()
```

The schema still governs these operations. A check constraint rejects an
insert that would violate it, and a foreign key rejects a reference to a row
that does not exist.

This means the rules you define in PostgreSQL remain the rules the client
must respect.

## The schema is the contract

A useful way to think about Supabase is that the schema is the contract.

The Data API does not invent its own model. It exposes the tables, columns,
relationships, and constraints you defined in PostgreSQL.

```text
PostgreSQL schema
    |
    v
generated Data API
    |
    v
client queries
```

When the schema changes, the shape of the data the client can request changes
with it.

That makes schema design the most important early decision in a Supabase
project.

## Client and server access

The Supabase client exists in two execution environments.

The browser uses a browser client configured with the publishable key.

The server uses a server client that can carry the request's cookies.

Both clients operate against the same project, the same tables, and the same
Data API.

```text
browser client
    -> Supabase Data API
    -> PostgreSQL

server client
    -> Supabase Data API
    -> PostgreSQL
```

TanStack Start route loaders and server functions are the natural places for
server-side database access.

## The complete model

The database lesson can be summarized as:

```text
schema (tables, columns, types)
    |
    +-- primary keys
    +-- foreign keys
    +-- constraints
    +-- indexes
    |
    v
PostgreSQL
    |
    v
Supabase Data API
    |
    v
Supabase client
    |
    +-- browser client
    +-- server client
    |
    v
TanStack Start application
```

Design the schema first, then let the Data API expose it, then let the
application query it.

## Next step

The next lesson builds on this foundation by introducing Supabase Auth, which
connects real user identities to the database through `auth.users`.
