# Supabase development workflow - typing

This lesson types the Supabase CLI workflow: initialize the project, start
the local stack, create and apply a migration, generate TypeScript types, and
wire the typed client into the application.

## Initialize the project

Set up Supabase configuration in the repository.

```text
# initialize Supabase configuration in the project
supabase init
```

This creates the `supabase/` directory and a configuration file.

## Start the local stack

Run the full local Supabase environment.

```text
# start the local Supabase services
supabase start
```

The local stack includes the database, Auth, Storage, Realtime, and Edge
Functions.

## Create a migration

A migration captures a schema change as a versioned file.

```text
# create a new migration for the artists table
supabase migration new create_artists
```

The command generates an empty timestamped SQL file.

```text
supabase/migrations/
    |
    +-- 20260811000000_create_artists.sql
```

## Write the migration SQL

The migration file contains the same SQL typed in the database lesson.

```sql
-- create a table representing artists
create table artists (
    -- generate a unique identifier for every artist
    id uuid primary key default gen_random_uuid(),

    -- require every artist to have a distinct name
    name text not null unique,

    -- record when the artist was added
    created_at timestamptz not null default now()
);
```

The schema change is now versioned and shareable.

## Apply the migration

Push the migration to the database.

```text
# apply the migration to the current database
supabase db push
```

The local database now contains the artists table.

## Generate TypeScript types

Ask the CLI to describe the schema as TypeScript types.

```text
# generate types from the database schema
supabase gen types typescript --local
    > src/lib/supabase/database.types.ts
```

The command writes a `Database` type describing the tables and columns.

## Use the typed client

Pass the generated type to the Supabase client.

```tsx
// import the generated database schema types
import { Database } from "@/lib/supabase/database.types"

// import the Supabase client constructor
import { createClient } from "@supabase/supabase-js"

// create a client typed against the database schema
const supabase = createClient<Database>(
    // identify the Supabase project
    import.meta.env.VITE_SUPABASE_URL,

    // authenticate the application with the publishable key
    import.meta.env.VITE_SUPABASE_PUBLISHABLE_KEY,
)
```

Queries against the client are now checked against the schema.

```tsx
// read artists with full type checking
const { data, error } = await supabase
    .from("artists")
    .select("id, name")
```

A schema change appears in the types the next time they are generated.

## Link to a hosted project

Connect local tooling to a hosted project.

```text
# link to the hosted project by reference
supabase link --project-ref your-project-ref
```

Linked commands can operate against the remote project.

## Stop the local stack

The local environment can be shut down when development is finished.

```text
# stop the local Supabase services
supabase stop
```

Starting and stopping the stack keeps local resources in check.

## Configure environment variables

The application reads its configuration from the environment.

```env
# identify the hosted Supabase project
VITE_SUPABASE_URL=https://your-project.supabase.co

# identify the client with the publishable key
VITE_SUPABASE_PUBLISHABLE_KEY=your-publishable-key
```

Privileged server secrets use a separate, server-only mechanism.

## Practice

Type the core workflow commands:

```text
supabase init
```

```text
supabase start
```

```text
supabase migration new create_artists
```

```text
supabase db push
```

Then type the typed client:

```tsx
const supabase = createClient<Database>(
    import.meta.env.VITE_SUPABASE_URL,
    import.meta.env.VITE_SUPABASE_PUBLISHABLE_KEY,
)
```

The central pattern is:

```text
init
  -> start (local)
  -> migration new + write SQL
  -> db push (apply)
  -> gen types
  -> link (remote)
```

The schema, policies, and functions from this track all live in the
`supabase/` directory as files that any environment can apply.
