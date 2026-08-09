# Supabase overview - typing

This lesson types a minimal TanStack Start + Supabase application:
install the client libraries, configure environment variables, create browser
and server clients, query PostgreSQL through Supabase, and understand where an
Edge Function fits.

## Install the Supabase libraries

The TanStack Start integration uses `supabase-js` for the Supabase client and
`@supabase/ssr` for browser/server session-aware clients.

```text
# install the Supabase JavaScript client
bun add @supabase/supabase-js

# install the SSR helpers used by TanStack Start
bun add @supabase/ssr
```

The two packages have different responsibilities.

```text
@supabase/supabase-js
    -> Supabase client API

@supabase/ssr
    -> browser/server session integration
```

## Configure the project

The application needs the Supabase project URL and publishable key.

```env
# identify the Supabase project
VITE_SUPABASE_URL=https://your-project.supabase.co

# identify the client as an application using the publishable key
VITE_SUPABASE_PUBLISHABLE_KEY=your-publishable-key
```

The current TanStack Start quickstart uses these variables for its browser
and server clients.

Do not put privileged server credentials into `VITE_` variables.

Vite exposes `VITE_` variables to browser-side application code.

## Create the browser client

The browser needs a client that can communicate with Supabase from browser
code.

```tsx
// provide Vite's import-meta environment type information
/// <reference types="vite/types/importMeta.d.ts" />

// import the helper for browser Supabase clients
import { createBrowserClient } from "@supabase/ssr"

// create a browser-side Supabase client
export function createClient() {
    // configure the client with the public project information
    return createBrowserClient(
        // identify the Supabase project
        import.meta.env.VITE_SUPABASE_URL,

        // authenticate the application with the publishable key
        import.meta.env.VITE_SUPABASE_PUBLISHABLE_KEY,
    )
}
```

This client belongs in something such as:

```text
src/lib/supabase/client.ts
```

It is suitable for code that actually executes in the browser.

## Create the server client

TanStack Start also needs a server-side client so loaders and server
operations can work with Supabase while preserving the request's cookies.

```tsx
// import Supabase's server-side client helper
import { createServerClient } from "@supabase/ssr"

// import TanStack Start's server cookie helpers
import {
    getCookies,
    setCookie,
    setResponseHeader,
} from "@tanstack/react-start/server"

// create a server-side Supabase client
export function createClient() {
    // configure the client for the current server request
    return createServerClient(
        // identify the Supabase project
        process.env.VITE_SUPABASE_URL!,

        // use the publishable key for the application client
        process.env.VITE_SUPABASE_PUBLISHABLE_KEY!,

        // connect Supabase session handling to TanStack Start
        {
            // provide the request and response cookie operations
            cookies: {
                // read cookies from the current request
                getAll() {
                    // convert TanStack's cookie map into Supabase's format
                    return Object.entries(getCookies()).map(
                        ([name, value]) => ({ name, value }),
                    )
                },

                // write cookies and response headers produced by Supabase
                setAll(cookies, headers) {
                    // apply each cookie to the response
                    cookies.forEach(({ name, value, options }) => {
                        // write the Supabase session cookie
                        setCookie(name, value, options)
                    })

                    // apply response headers returned by Supabase
                    Object.entries(headers).forEach(([name, value]) => {
                        // attach the header to the TanStack response
                        setResponseHeader(name, value)
                    })
                },
            },
        },
    )
}
```

This belongs in something such as:

```text
src/lib/supabase/server.ts
```

The important distinction is:

```text
client.ts
    -> browser

server.ts
    -> server
```

## Query PostgreSQL from a route

Create a small table in the Supabase SQL Editor.

```sql
-- create a table representing instruments
create table instruments (
    -- create an automatically generated numeric primary key
    id bigint primary key generated always as identity,

    -- require every instrument to have a name
    name text not null
);

-- add an example instrument
insert into instruments (name)
values ('Piano');

-- add another example instrument
insert into instruments (name)
values ('Violin');
```

Now a TanStack Start route can query the table through the server client.

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

        // query the instruments table through Supabase
        const { data: instruments, error } = await supabase
            .from("instruments")
            .select("id, name")

        // stop the route if the database request failed
        if (error) {
            throw error
        }

        // expose the query result as route data
        return { instruments }
    },

    // render the page after the loader completes
    component: Home,
})

// define the component for the index route
function Home() {
    // retrieve the data returned by the route loader
    const { instruments } = Route.useLoaderData()

    // render every instrument returned by PostgreSQL
    return (
        <ul>
            {instruments.map((instrument) => (
                // render one list item for each database row
                <li key={instrument.id}>
                    {instrument.name}
                </li>
            ))}
        </ul>
    )
}
```

The complete request path is now:

```text
browser
   |
   v
TanStack Start route
   |
   v
server loader
   |
   v
Supabase server client
   |
   v
Supabase Data API
   |
   v
PostgreSQL
   |
   v
route data
   |
   v
HTML / browser
```

## Create an Edge Function

Edge Functions are separate server-side TypeScript programs running in
Supabase's Deno-compatible Edge Runtime. They are created and deployed with
the Supabase CLI.

Initialize the Supabase portion of the project.

```text
# initialize Supabase configuration in the project
supabase init
```

Create a function.

```text
# create a new TypeScript Edge Function
supabase functions new hello
```

The generated function can contain a minimal HTTP handler.

```ts
// import the HTTP request and response types from the Deno runtime
Deno.serve(async (request) => {
    // create the response returned by the function
    return new Response("Hello from Supabase Edge Functions", {
        // identify the response as plain text
        headers: {
            "Content-Type": "text/plain",
        },
    })
})
```

The architecture is now:

```text
TanStack Start
    |
    +-- route loader
    |      |
    |      v
    |   Supabase
    |      |
    |      v
    |   PostgreSQL
    |
    +-- browser
    |
    +-- optional Edge Function
           |
           +-- external APIs
           +-- Auth
           +-- Storage
           +-- database
```

## Now type it again

Start with the core server-side query:

```tsx
// import the server-side Supabase client
import { createClient } from "@/lib/supabase/server"

// load database data
const supabase = createClient()

// query PostgreSQL through Supabase
const { data, error } = await supabase
    .from("instruments")
    .select("id, name")
```

Then connect that operation to a route:

```tsx
// create the index route
export const Route = createFileRoute("/")({
    // load the database data for the route
    loader: async () => {
        // create the server client
        const supabase = createClient()

        // execute the database query
        const { data, error } = await supabase
            .from("instruments")
            .select("id, name")

        // reject failed database requests
        if (error) {
            throw error
        }

        // return the database result
        return { instruments: data }
    },
})
```

The central pattern to remember is:

```text
TanStack route
    -> Supabase client
    -> Supabase API
    -> PostgreSQL
    -> route data
```

The rest of the track expands each major Supabase service without changing
this fundamental architecture.

