# Supabase storage - typing

This lesson types a storage bucket for album covers, a policy for uploading
into it, a browser upload, and the download path through a public URL.

## Create a bucket

Buckets are rows in `storage.buckets`.

```sql
-- create a bucket for album cover images
insert into storage.buckets (id, name, public)
values ('covers', 'covers', false);
```

The bucket is private, so objects inside it are not openly accessible yet.

## Allow authenticated uploads

Storage access is controlled by policies.

```sql
-- allow signed-in users to upload covers
create policy "users upload covers"
on storage.objects
for insert
to authenticated
with check (bucket_id = 'covers');
```

```sql
-- allow signed-in users to read covers
create policy "users read covers"
on storage.objects
for select
to authenticated
using (bucket_id = 'covers');
```

The policies restrict access to authenticated requests.

## Upload a cover from the browser

The browser client uploads the file to a path.

```tsx
// import the browser-side Supabase client
import { createClient } from "@/lib/supabase/client"

// create a client for browser-side storage access
const supabase = createClient()

// upload a cover image for an album
const { data: upload, error } = await supabase.storage
    .from("covers")
    .upload(`album-${albumId}.png`, file, {
        // replace an existing object at the same path
        upsert: true,

        // tell Storage what kind of file was uploaded
        contentType: "image/png",
    })
```

The path ties the object to the album.

## Save the path in the database

The album row stores a reference to the uploaded object.

```tsx
// import the helper for creating a server-side function
import { createServerFn } from "@tanstack/react-start"

// import the server-side Supabase client
import { createClient } from "@/lib/supabase/server"

// create a server function for recording an album cover
export const setAlbumCover = createServerFn({
    // declare that this operation changes server-side state
    method: "POST",
})
    // validate the expected cover input
    .inputValidator((data: { albumId: string; coverPath: string }) => data)
    // record the cover path on the server
    .handler(async ({ data }) => {
        // create a Supabase client for the current server request
        const supabase = createClient()

        // save the object path on the album record
        const { error } = await supabase
            .from("albums")
            .update({ cover_path: data.coverPath })
            .eq("id", data.albumId)

        // surface database failures to the client
        if (error) {
            throw error
        }
    })
```

The database now points at the file.

## Read the path and render the cover

A route loader returns the stored path, and the component builds the URL.

```tsx
// import the helper for creating a file-based TanStack route
import { createFileRoute } from "@tanstack/react-router"

// import the server-side Supabase client
import { createClient } from "@/lib/supabase/server"

// define a route for one album
export const Route = createFileRoute("/albums/$albumId")({
    // load the album and its cover before rendering
    loader: async ({ params }) => {
        // create a Supabase client for the current server request
        const supabase = createClient()

        // load the album record
        const { data: album, error } = await supabase
            .from("albums")
            .select("id, title, cover_path")
            .eq("id", params.albumId)
            .single()

        // stop the route if the database request failed
        if (error) {
            throw error
        }

        // expose the album with its stored cover path
        return { album }
    },

    // render the page after the loader completes
    component: Album,
})

// define the album component
function Album() {
    // retrieve the album returned by the route loader
    const { album } = Route.useLoaderData()

    // render the album with its cover image
    return (
        <div>
            <img src={album.cover_path} alt={album.title} />
            <h1>{album.title}</h1>
        </div>
    )
}
```

The component renders the stored path directly.

## Serve the object publicly

Because this bucket is private, serving the cover needs the public URL or a
signed URL. For a public bucket, the client can build the URL directly.

```tsx
// create a public URL for an object
const { data } = supabase.storage
    .from("covers")
    .getPublicUrl("album-abc.png")

// the URL that can serve the object
const coverUrl = data.publicUrl
```

For a private object, the server creates a temporary signed URL instead.

```tsx
// create a signed URL for a private object
const { data, error } = await supabase.storage
    .from("covers")
    .createSignedUrl("album-abc.png", 60)

// the temporary URL that can serve the object
const coverUrl = data.signedUrl
```

The signed URL stops working after the time limit.

## Practice

Type the bucket policy shape:

```sql
-- allow signed-in users to upload covers
create policy "users upload covers"
on storage.objects
for insert
to authenticated
with check (bucket_id = 'covers');
```

Then type the browser upload:

```tsx
const { data, error } = await supabase.storage
    .from("covers")
    .upload(`album-${albumId}.png`, file)
```

And the server-side signed URL:

```tsx
const { data, error } = await supabase.storage
    .from("covers")
    .createSignedUrl("album-abc.png", 60)
```

The central pattern is:

```text
bucket
  -> policy (auth.uid())
  -> browser upload at a path
  -> database path column
  -> public or signed URL
```

Storage holds the bytes; the database holds the reference; policies hold the
gate.
