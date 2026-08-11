# Supabase development workflow - concepts

Throughout this track, the database schema has been the foundation of the
application.

The database lesson created tables by running SQL in the dashboard.

That works for a first step, but a real project needs the schema to evolve
reproducibly.

```text
developer A  ->  schema change
developer B  ->  schema change
CI           ->  schema change
staging      ->  schema change
production   ->  schema change
```

The tool that makes this manageable is the Supabase CLI.

## The Supabase CLI

The CLI is the command-line interface to a Supabase project.

It handles local development, migrations, linking, and deployment.

```text
supabase init     ->  set up configuration
supabase start    ->  run a local stack
supabase migration new  ->  create a migration
supabase db push  ->  apply migrations
supabase link     ->  connect to a hosted project
supabase gen types ->  generate TypeScript types
```

The CLI turns manual database work into files and commands that a team can
share.

## Local development

`supabase start` runs a full Supabase stack on the developer's machine.

```text
local stack
    |
    +-- PostgreSQL
    +-- Auth
    +-- Storage
    +-- Realtime
    +-- Edge Functions
```

The local stack mirrors the hosted project, so features behave the same way
locally as they do remotely.

Development happens against this local environment:

```text
code
  -> local Supabase
  -> test
  -> push changes
```

An application's environment variables point at the local project during
development and at the hosted project when deployed.

## Migrations are schema as code

A migration is a versioned file of SQL changes.

```text
migration 1  ->  create artists table
migration 2  ->  add albums table
migration 3  ->  add RLS policies
```

The schema is no longer a set of manual clicks.

It is a sequence of reproducible steps.

```text
supabase/migrations/
    |
    +-- 20260101000000_create_artists.sql
    +-- 20260102000000_add_albums.sql
    +-- ...
```

Migrations make the database state of any environment reproducible from
scratch.

## Applying migrations

Migrations are applied in order.

```text
migration 1  ->  applied
migration 2  ->  applied
migration 3  ->  applied
```

Applying the same migration files to a fresh database produces the same
schema.

This is how a developer, staging, and production environment stay in sync:
they all apply the same ordered list of changes.

## Creating a migration

A migration starts as a new file.

The CLI creates the file with a timestamp-based name.

```text
supabase migration new add_albums
```

The developer writes the SQL change into the file, just like the SQL typed in
the dashboard earlier in this track.

```sql
create table albums (
    ...
);
```

The difference is that the change is now recorded and shareable.

## Linking to a hosted project

Local work connects to a hosted project through `supabase link`.

```text
supabase link --project-ref <ref>
```

Linking lets local commands operate against the remote project, for example
pushing migrations or pulling configuration.

This creates the bridge between local development and deployment.

## Environment variables

Different environments need different configuration.

```text
local       ->  local Supabase URL and keys
production  ->  hosted Supabase URL and keys
```

The application reads these from its environment, not from hard-coded
values.

Secrets such as the service role key belong in the environment too, and only
in server-side contexts.

The overview lesson's `VITE_` variables cover the publishable configuration;
privileged secrets use a different mechanism.

## Generated types

Supabase can generate TypeScript types from the database schema.

```text
supabase gen types typescript
    |
    v
Database type definitions
```

These types describe the tables, columns, and relationships the client can
query.

Passing the generated `Database` type to the client makes queries
type-checked.

```text
createClient(url, key, { db: { schema } })
```

A change in the schema is then visible to the type checker, not just at
runtime.

## Dashboard versus migrations

The dashboard is convenient for inspection and quick experiments.

Migrations are the source of truth for how an environment gets built.

```text
dashboard     ->  inspect, experiment, learn
migrations    ->  version, share, deploy
```

Making schema changes only in the dashboard creates environments that cannot
be reproduced.

The workflow that scales is:

```text
write migration
apply locally
verify
apply to shared environments
```

## CI and deployment

Because migrations are files, they can participate in the normal pipeline.

```text
push code
    |
    v
CI runs checks
    |
    v
apply migrations
    |
    v
deploy application
```

The same workflow that builds and tests application code can also apply
database changes.

This is what makes the whole architecture from this track deployable.

## Branching and teams

Migrations are versioned files, so teams can work on them like code.

Each branch can carry its own migration files.

```text
main
  +-- 20260101000000_create_artists.sql
  +-- 20260102000000_add_albums.sql

feature/shared-documents
  +-- 20260103000000_add_documents.sql
```

A migration is applied once, in order.

Because the files are timestamped and sequential, two developers adding
migrations at the same time produce files that merge without overwriting
each other.

Reviewing a schema change becomes reviewing a SQL file, just like reviewing
application code.

## Secrets management

The repository should not contain secrets.

```text
committed            ->  no secrets
environment          ->  secrets
server-side only     ->  privileged secrets
```

Configuration that is safe to share lives in the repository or in
publishable variables.

Credentials belong in the environment of the environment that needs them.

A local developer has local values, staging has staging values, and
production has production values.

The application reads them the same way; only the values differ.

## Common workflow errors

The CLI workflow has a few recurring mistakes:

```text
editing the database without writing a migration
applying changes only to one environment
committing secrets into the repository
forgetting to regenerate types after a schema change
```

Each one breaks the reproducibility that the workflow is designed to
provide.

The rule to remember is: if it is a schema change, it belongs in a
migration file, and if it is a secret, it belongs in an environment.

## The complete workflow

```text
supabase init
    |
    v
supabase start (local stack)
    |
    v
supabase migration new + write SQL
    |
    v
apply + verify locally
    |
    v
supabase gen types
    |
    v
supabase link + push
    |
    v
CI applies migrations and deploys
```

Every piece of this track now has a reproducible home:

```text
database   ->  migrations
auth       ->  configuration + migrations
storage    ->  buckets and policies in migrations
realtime   ->  channels in application code
edge functions -> supabase/functions
RLS        ->  policies in migrations
```

The whole Supabase side of the application is described by files.

## End of track

This lesson closes the Supabase track.

The architecture from the overview has been filled in service by service:

```text
TanStack Start
    |
    v
Supabase
    |
    +-- PostgreSQL (migrations)
    +-- Auth
    +-- Storage
    +-- Realtime
    +-- Edge Functions
    |
    v
RLS everywhere
```

Learning a platform is mostly learning where each piece fits and how the
pieces agree.

In Supabase, everything agrees on PostgreSQL and the authenticated identity,
and the development workflow makes that agreement reproducible.
