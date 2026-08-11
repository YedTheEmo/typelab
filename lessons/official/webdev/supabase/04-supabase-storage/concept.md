# Supabase storage - concepts

The database stores structured data.

Storage stores files.

```text
PostgreSQL
    users
    albums
    tracks

Storage
    avatars
    album-covers
    attachments
```

A relational row is not a good place for a large binary file.

A file is not a good place for structured, queryable data.

Supabase Storage exists for the second kind of thing.

## Why files are not database rows

PostgreSQL can store binary data, but that is rarely the right choice for
application files.

```text
database row
    |
    +-- text fields   (great)
    +-- large binary  (usually avoid)
```

Storing large files in the database makes the database bigger, slower to
back up, and slower to serve.

Files also have different access needs than rows:

```text
database  ->  query, join, filter
file      ->  stream, cache, serve directly
```

Storage is built for serving files. The database is built for data.

## Buckets and objects

Storage organizes files into buckets.

A bucket is a top-level container for objects.

```text
storage.buckets
    |
    +-- avatars
    +-- covers
    +-- attachments
```

An object is a file inside a bucket.

```text
avatars
    |
    +-- user-42.png
    +-- user-7.jpg
```

The bucket is the unit of configuration. Access rules can differ from bucket
to bucket.

## Objects have paths

Each object has a path inside its bucket.

```text
avatars/user-42.png
```

The path looks like a file system path, and Storage treats the slashes as
organization rather than as real nested files.

```text
covers/2024/night-train.png
```

Applications commonly derive the path from the related record, such as the
user id or the record id:

```text
avatars/{user.id}.png
albums/{album.id}.png
```

A well-chosen path makes it easy to find the file for a given record.

Paths also have a security dimension. If paths are predictable and policies
are missing, a client could request an object it should not reach.

```text
covers/user-42.png     (owned by user 42)
covers/user-7.png      (not yours)
```

Including the authenticated user's id in the path, and writing policies that
check it, prevents cross-user access. This mirrors the ownership pattern used
for database rows.

## File sizes and limits

Files vary in size, and upload strategy depends on size.

Small files can be uploaded in a single request.

```text
upload(path, file)  ->  done
```

Large files need a different approach, often uploading in parts or keeping
them out of the browser flow entirely.

```text
large file
    |
    +-- upload to the server first
    +-- then move into Storage
    +-- or stream server-side
```

The right choice depends on the application's file sizes and where the bytes
should come from.

## Direct upload from the browser

Storage supports uploads that happen directly from browser code.

```text
browser
    |
    | file bytes + path
    v
Supabase Storage
```

The advantage is that the application server does not have to relay the file
bytes.

The requirement is that Storage policies must correctly restrict what the
browser is allowed to upload.

## Server-mediated upload

Some applications prefer to receive the file on the server first.

```text
browser
    |
    v
application server
    |
    v
Storage
```

The server can validate the file, check permissions in application code, and
then upload it with the server client.

This adds a layer of application control at the cost of moving the bytes
through the server.

Both patterns are valid. The trade-off is control versus directness.

## Public and private buckets

A bucket can be public or private.

A public bucket serves objects at a public URL without any extra
authorization.

```text
browser -> public URL -> object
```

A private bucket requires authorization before an object can be accessed.

```text
browser -> Supabase Storage -> policy -> object
```

The decision is a security decision. Anything that should be restricted must
not live in a public bucket.

## Access control

Storage is connected to the same PostgreSQL database, and its access rules
are database policies.

```text
request
    |
    v
storage policy
    |
    +-- allow
    +-- deny
```

Policies can restrict who may read, upload, and update objects in a bucket.

Because Storage uses the same identity model as the rest of Supabase, the
same `auth.uid()` from the authentication lesson works here:

```text
signed-in user
    |
    v
auth.uid()
    |
    v
storage policy
    |
    v
allowed object
```

Storage and Row Level Security therefore share the same mental model. The
security lesson later in this track applies the same ideas to tables.

## Uploading a file

A browser can upload a file through the Supabase client.

```text
browser
    |
    | file bytes
    v
Supabase client
    |
    v
Storage
    |
    v
object at a path
```

The upload supplies a path and the file data.

The browser can upload directly because the storage policies determine what
that browser is allowed to do, in the same way RLS policies do for tables.

## Downloading a file

Downloading can work in more than one way.

A public object has a stable public URL:

```text
https://<project>.supabase.co/storage/v1/object/public/avatars/user-42.png
```

A private object can be accessed through a signed URL.

A signed URL is a temporary, capability-based link that grants access for a
limited time.

```text
server
    |
    | create signed URL
    v
browser
    |
    | link valid for e.g. 60 seconds
    v
object
```

Signed URLs let the application hand out temporary access without making the
object public.

## Image transformations

For image files, Storage can serve transformed versions of an object.

```text
original image
    |
    v
transform (resize / crop / format)
    |
    v
transformed image
```

Transformations are requested through the URL rather than by creating new
files.

```text
.../covers/album-abc.png?width=200&height=200
```

This lets the application store one original and serve many display sizes.

The transformation happens at serve time, so the stored object stays
unchanged.

## Metadata in the database, file in Storage

A common Supabase pattern is to keep the file in Storage and the reference
to it in the database.

```text
albums
    |
    +-- id
    +-- title
    +-- cover_path
            |
            v
       covers/{id}.png
```

The application stores the path as a column value.

When the application needs the file, it reads the path and uses Storage to
access the object.

```text
route loader
    |
    v
album record (with cover_path)
    |
    v
Storage lookup for cover_path
```

This keeps relational queries clean while still using Storage for the file
itself.

## Storage and TanStack Start

Storage operations are available from both Supabase clients.

The browser client can upload files directly.

```text
browser
    |
    v
browser Supabase client
    |
    v
Storage upload
```

Server code can create signed URLs or perform storage operations that
require a more privileged context.

```text
server
    |
    v
server Supabase client
    |
    v
signed URL
```

A typical division is:

```text
browser  ->  upload the file
server   ->  control access and produce URLs
```

## Object lifecycle

Objects and their database references have a lifecycle.

When a record is replaced, its old object should usually be removed.

```text
replace cover
    |
    +-- upload new object
    +-- update the path column
    +-- delete the old object
```

When a record is deleted, its objects should usually be deleted too.

```text
delete album
    |
    +-- delete the album row
    +-- delete its cover object
```

Leaving orphaned objects behind wastes storage and can leak data through
predictable paths.

Cleanup is application responsibility: removing the row does not remove the
object automatically.

## Common pitfalls

A few mistakes cause most storage security problems:

```text
putting private data in a public bucket
letting clients choose arbitrary paths
using user-controlled input in a path without validation
forgetting to attach policies to a bucket
```

Path handling deserves particular care.

A client should not be able to derive paths that let it read or overwrite
objects it does not own.

The server or the policies should constrain allowed paths, typically by
including the current user's id or an id the server assigns.

## The complete storage model

```text
bucket
    |
    +-- public  ->  public URL
    |
    +-- private ->  signed URL / policy
    |
    +-- policies (auth.uid())
    |
    v
objects at paths
    |
    |
database record
    |
    +-- object path column
    |
    v
TanStack Start
    |
    +-- browser upload
    +-- server signed URL
```

The database holds the metadata, Storage holds the bytes, and the identity
from Supabase Auth controls who can reach which object.

## Next step

The next lesson covers Supabase Edge Functions, which run server-side
TypeScript on Supabase's Edge Runtime and can act as the server side of
workflows such as processing uploaded files.
