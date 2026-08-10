# Input validation and output encoding - typing

This lesson types server-side schemas with Zod, parameterized Supabase
queries, safe HTML rendering, redirect validation, and controlled file
uploads.

## Install Zod

Use Zod to define explicit input contracts.

```bash
# install the schema validation library
bun add zod
```

## Define an input schema

A profile operation should accept only the fields it actually needs.

```typescript
// import the Zod schema builder
import { z } from "zod";

// define the accepted profile input
const profileSchema = z.object({
    // require a bounded display name
    displayName: z.string().trim().min(1).max(80),
});
```

The schema describes acceptable input. It does not grant authorization.

## Parse untrusted input

Parse the browser value before application logic uses it.

```typescript
// validate and transform untrusted profile input
function parseProfileInput(input: unknown) {
    // reject malformed values and return validated data
    return profileSchema.parse(input);
}
```

The important boundary is:

```text
unknown input
    |
    v
schema
    |
    v
validated value
```

## Validate enumerated values

Use explicit allowed values when a field has a fixed vocabulary.

```typescript
// define the only accepted order states
const orderStatus = z.enum([
    "pending",
    "processing",
    "completed",
    "failed",
]);
```

An attacker cannot introduce an arbitrary status through this schema.

## Validate numeric ranges

Numeric input should be explicitly parsed and bounded.

```typescript
// define a bounded page number
const pageSchema = z.object({
    // accept an integer between one and one hundred
    page: z.number().int().min(1).max(100),
});
```

Do not assume that a browser string such as `"10"` is automatically the
correct application type.

## Validate a request before the database

The application should validate before performing the database operation.

```typescript
// define the complete document creation input
const createDocumentSchema = z.object({
    // require a valid organization identifier
    organizationId: z.string().uuid(),

    // require a bounded document title
    title: z.string().trim().min(1).max(200),
});
```

Then parse the request.

```typescript
// create a document from untrusted request data
async function createDocument(input: unknown) {
    // parse the untrusted request first
    const data = createDocumentSchema.parse(input);

    // continue only with validated values
    return data;
}
```

Authorization must still occur before the database write.

## Parameterized Supabase queries

Supabase's query builder accepts values separately from the query structure.

```typescript
// retrieve a document using a parameterized equality filter
const result = await supabase
    .from("documents")
    .select("id, title")
    .eq("id", documentId)
    .maybeSingle();
```

The supplied identifier is treated as a value rather than concatenated into
SQL syntax.

## Avoid raw SQL concatenation

Do not construct SQL statements from attacker-controlled strings.

```typescript
// do not build SQL by concatenating request data
const unsafeSql = `select * from documents where id = '${documentId}'`;
```

This turns data into SQL source.

Use parameterized queries or properly parameterized database functions
instead.

## Authorization still matters

A parameterized query can safely retrieve the wrong person's record if the
authorization boundary is missing.

```typescript
// query by identifier without assuming that the identifier proves ownership
const result = await supabase
    .from("documents")
    .select("id, title")
    .eq("id", documentId)
    .maybeSingle();
```

The database policy must independently enforce ownership or tenant access.

```sql
-- allow only owners to read documents
create policy "users read own documents"
on documents
for select
using (auth.uid() = owner_id);
```

## Safe React output

Normal React text interpolation escapes HTML-sensitive characters.

```tsx
// render user-controlled text as text
export function ProfileName({ name }: { name: string }) {
    // React treats the value as text rather than executable HTML
    return <span>{name}</span>;
}
```

A value such as `<script>...</script>` is rendered as text rather than
executed through ordinary JSX interpolation.

## Avoid raw HTML

Do not use raw HTML rendering for arbitrary user input.

```tsx
// avoid rendering untrusted content as raw HTML
export function UnsafeContent({ content }: { content: string }) {
    // this bypasses ordinary React text escaping
    return <div dangerouslySetInnerHTML={{ __html: content }} />;
}
```

If an application genuinely requires user-authored HTML, use a dedicated
sanitization design for the exact supported HTML subset.

## Validate redirect targets

Never treat a client-provided redirect destination as automatically safe.

```typescript
// allow only local application paths as redirect targets
function safeRedirectTarget(value: string) {
    // parse the supplied target as a URL relative to the application
    const url = new URL(value, "https://app.example.com");

    // reject redirects to another origin
    if (url.origin !== "https://app.example.com") {
        throw new Error("invalid redirect target");
    }

    // return the trusted relative path
    return `${url.pathname}${url.search}${url.hash}`;
}
```

This prevents a client from turning an application redirect into an external
open redirect.

## Generate safe storage paths

Do not use arbitrary filenames as trusted storage paths.

```typescript
// create a storage path from trusted identity and a generated identifier
function createStoragePath(userId: string, objectId: string) {
    // construct a controlled path namespace
    return `users/${userId}/objects/${objectId}`;
}
```

The user identifier should come from authenticated identity rather than from
an arbitrary browser field.

## Validate upload metadata

An upload request should enforce a maximum size and allowed content types.

```typescript
// define upload constraints
const uploadSchema = z.object({
    // allow only the expected media types
    contentType: z.enum([
        "image/jpeg",
        "image/png",
        "application/pdf",
    ]),

    // reject files larger than ten megabytes
    size: z.number().int().positive().max(10_000_000),
});
```

Metadata validation is only one part of file security. For high-risk
workflows, inspect the actual file content as well.

## Reject unexpected file extensions

Extensions should not be treated as proof of content type.

```typescript
// permit only known extensions for a specific workflow
const allowedExtensions = new Set([
    ".jpg",
    ".jpeg",
    ".png",
    ".pdf",
]);
```

The application should combine extension checks with content validation where
the risk requires it.

## Validate external webhook payloads

Webhook data is also untrusted input.

```typescript
// define the expected webhook structure
const webhookSchema = z.object({
    // require a known event identifier
    id: z.string().min(1),

    // accept only known event types
    type: z.enum([
        "order.created",
        "order.completed",
        "order.failed",
    ]),

    // require a structured object payload
    data: z.record(z.string(), z.unknown()),
});
```

Signature verification should happen before trusting the event as authentic.

Schema validation should then establish that the authenticated payload has
the expected structure.

## Return safe validation errors

Do not expose internal implementation details in ordinary API responses.

```typescript
// convert a validation failure into a controlled response
function validationResponse(error: z.ZodError) {
    // expose only field-level validation information
    return {
        error: "invalid_request",
        fields: error.issues.map((issue) => ({
            path: issue.path,
            message: issue.message,
        })),
    };
}
```

Internal stack traces and database diagnostics belong in controlled server
logging, not arbitrary browser responses.

## Test rejected input

Validation should be tested with malformed and boundary values.

```typescript
// demonstrate rejection of an empty display name
const result = profileSchema.safeParse({
    displayName: "",
});

// verify that invalid input was rejected
if (result.success) {
    throw new Error("invalid input accepted");
}
```

Test values outside the allowed range as well.

```typescript
// demonstrate rejection of an excessive display name
const result = profileSchema.safeParse({
    displayName: "a".repeat(81),
});

// verify that the boundary was enforced
if (result.success) {
    throw new Error("oversized input accepted");
}
```

## Test an unsafe redirect

The redirect helper should reject external destinations.

```typescript
// demonstrate rejection of an external redirect
function testExternalRedirect() {
    // attempt to redirect to an attacker-controlled origin
    safeRedirectTarget("https://evil.example");

    // reaching this line means the security check failed
    throw new Error("external redirect accepted");
}
```

The helper should throw before the final error is reached.

## Now type it again

Reconstruct the validation boundary.

```typescript
// define a bounded display name
const profileSchema = z.object({
    displayName: z.string().trim().min(1).max(80),
});

// parse untrusted input before using it
const data = profileSchema.parse(input);
```

Then reconstruct the parameterized database operation.

```typescript
// query using a value rather than SQL string concatenation
const result = await supabase
    .from("documents")
    .select("id, title")
    .eq("id", documentId)
    .maybeSingle();
```

Finally, reconstruct safe redirect handling.

```typescript
// parse the redirect against the application's trusted origin
const url = new URL(value, "https://app.example.com");

// reject another origin
if (url.origin !== "https://app.example.com") {
    throw new Error("invalid redirect target");
}
```

## Wrap up

The flow: untrusted input -> parse -> validate -> authorize -> operate ->
encode for the output context.
