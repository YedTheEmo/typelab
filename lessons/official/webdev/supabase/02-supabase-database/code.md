# Supabase database - typing

This lesson types a relational schema for a music store in the Supabase SQL
Editor, seeds it with data, and then queries the related tables through the
Supabase client from a TanStack Start route.

## Create the artists table

Start with the table that owns the top level of the model.

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

The primary key guarantees that every row can be identified.

## Create the albums table

An album belongs to exactly one artist.

```sql
-- create a table representing albums
create table albums (
    -- generate a unique identifier for every album
    id uuid primary key default gen_random_uuid(),

    -- link the album to its owning artist
    artist_id uuid not null references artists(id),

    -- require every album to have a title
    title text not null,

    -- record when the album was released
    released_at timestamptz not null default now()
);
```

The foreign key makes PostgreSQL enforce that every album references a real
artist.

## Create the tracks table

A track belongs to exactly one album.

```sql
-- create a table representing tracks
create table tracks (
    -- generate a unique identifier for every track
    id uuid primary key default gen_random_uuid(),

    -- link the track to its owning album
    album_id uuid not null references albums(id),

    -- require every track to have a title
    title text not null,

    -- store the duration as an exact decimal
    duration numeric not null check (duration > 0),

    -- record the order of the track on the album
    position integer not null
);
```

The check constraint prevents an impossible duration.

## Add an index on the foreign key

Queries commonly look up albums by their artist.

```sql
-- speed up lookups by artist
create index on albums (artist_id);
```

The index helps the join between artists and albums.

## Seed the data

Insert one artist, one album, and two tracks.

```sql
-- insert an artist and return its identifier
insert into artists (name)
values ('The Starlings')
returning id;
```

```sql
-- insert an album for the artist
insert into albums (artist_id, title)
values ('00000000-0000-0000-0000-000000000000', 'Night Train')
returning id;
```

```sql
-- insert the first track on the album
insert into tracks (album_id, title, duration, position)
values ('00000000-0000-0000-0000-000000000000', 'Departure', 214, 1);

-- insert the second track on the album
insert into tracks (album_id, title, duration, position)
values ('00000000-0000-0000-0000-000000000000', 'Arrival', 187, 2);
```

In practice you replace the placeholder identifiers with the values returned
by the previous statements.

## Query the relationship with SQL

A join lets one statement read related data.

```sql
-- read every track with its album and artist
select
    artists.name as artist,
    albums.title as album,
    tracks.title as track
from tracks
join albums on albums.id = tracks.album_id
join artists on artists.id = albums.artist_id
order by albums.released_at, tracks.position;
```

## Query the relationship through Supabase

The same relationship is available through the client using nested selection.

```tsx
// import the helper for creating a file-based TanStack route
import { createFileRoute } from "@tanstack/react-router"

// import the server-side Supabase client
import { createClient } from "@/lib/supabase/server"

// define the application's index route
export const Route = createFileRoute("/")({
    // load database data before rendering the route
    loader: async () => {
        // create a Supabase client for the current server request
        const supabase = createClient()

        // read artists together with their albums and tracks
        const { data: artists, error } = await supabase
            .from("artists")
            .select("id, name, albums(id, title, tracks(id, title, duration))")

        // stop the route if the database request failed
        if (error) {
            throw error
        }

        // expose the nested query result as route data
        return { artists }
    },

    // render the page after the loader completes
    component: Home,
})

// define the component for the index route
function Home() {
    // retrieve the data returned by the route loader
    const { artists } = Route.useLoaderData()

    // render every artist with their albums
    return (
        <ul>
            {artists.map((artist) => (
                <li key={artist.id}>
                    {artist.name}
                    <ul>
                        {artist.albums.map((album) => (
                            <li key={album.id}>{album.title}</li>
                        ))}
                    </ul>
                </li>
            ))}
        </ul>
    )
}
```

The nested `select` follows the foreign keys you defined in SQL.

## Practice

Type the schema for a simpler pair of related tables.

```sql
-- create a table representing genres
create table genres (
    -- generate a unique identifier for every genre
    id uuid primary key default gen_random_uuid(),

    -- require every genre to have a distinct name
    name text not null unique
);
```

```sql
-- create a table representing artists that belong to a genre
create table artists (
    -- generate a unique identifier for every artist
    id uuid primary key default gen_random_uuid(),

    -- link the artist to its genre
    genre_id uuid not null references genres(id),

    -- require every artist to have a name
    name text not null
);
```

Then type the client query for the relationship.

```tsx
// read artists together with their genre
const { data: artists, error } = await supabase
    .from("artists")
    .select("id, name, genres(name)")
```

The central pattern is:

```text
SQL schema
    -> tables and foreign keys
    -> Supabase Data API
    -> nested select
    -> related data in the browser
```

The database defines the relationships; the client reads them.
