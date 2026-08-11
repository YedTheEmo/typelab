# Supabase edge functions - concepts

An Edge Function is a server-side TypeScript program running on Supabase's
Deno-compatible Edge Runtime.

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

Supabase distributes these functions globally, so a request can be handled
close to the client.

## When an Edge Function fits

Edge Functions are useful when the application needs a server-side HTTP
endpoint that is not tied to a specific page.

Common examples:

```text
webhook receiver
third-party integration
transactional email
payment callback
short-lived server-side work
```

These operations share a shape:

```text
external event
    |
    v
Edge Function
    |
    v
side effect
```

The function runs once per request, does its work, and returns a response.

## Edge Functions versus TanStack Start server code

TanStack Start already has server-side execution.

An Edge Function is therefore not automatically the answer whenever
server-side code is needed.

The useful distinction is:

```text
TanStack Start server code
    |
    +-- route loaders
    +-- server functions
    +-- application-specific logic
    |
    v
your application server

Supabase Edge Function
    |
    +-- standalone HTTP endpoint
    +-- webhook receiver
    +-- Supabase-centric operation
    +-- globally distributed
    |
    v
Supabase Edge Runtime
```

If an operation is tightly coupled to a route, a Start server function is
likely the natural choice.

If it must exist as an independently callable endpoint, receive a webhook, or
run within Supabase's edge environment, an Edge Function is more
appropriate.

## The request/response model

An Edge Function is an HTTP handler.

It receives a request and returns a response.

```text
request  ->  handler  ->  response
```

The handler can read the method, headers, and body of the request.

```text
method      ->  GET, POST, ...
headers     ->  content type, auth, ...
body        ->  JSON or other data
```

The response carries a status and a body.

```text
status 200  ->  success
status 400  ->  bad input
status 401  ->  unauthorized
status 500  ->  server error
```

Edge Functions do not run a browser. They run server-side TypeScript against
the Deno runtime, so the code must follow the server model.

## Webhooks

A webhook is an HTTP request sent by another system when something happens.

```text
external service
    |
    | event -> HTTP POST
    v
Edge Function
```

A payment provider might notify the function about a successful payment.

A source code host might notify the function about a push.

The Edge Function receives the notification and performs the corresponding
side effect, such as updating a database row.

Webhooks are a natural fit because Edge Functions are standalone HTTP
endpoints.

## Secrets and environment variables

Server-side code often needs secrets.

An Edge Function can access environment variables configured for the
function.

```text
environment variable
    |
    v
Edge Function (server only)
```

A secret needed by an Edge Function belongs in the function's environment,
not in browser-readable code.

This is one of the main reasons to use an Edge Function instead of running
the operation in the browser.

## Service role access

Edge Functions run on the server, which means they can hold a privileged
credential.

Supabase exposes the service role for server-side access that bypasses the
normal client roles.

```text
server credential
    |
    v
service role client
    |
    v
direct database access
```

The service role can perform privileged operations that browser clients
must not be able to perform.

This power must be used carefully: the credential must never be embedded in
browser code or exposed through responses.

## Authenticating callers

A publicly deployed Edge Function can be called by anyone who knows its URL.

That may be fine for a webhook, where the sender is authenticated another
way, or for a public endpoint.

For restricted operations, the function should authenticate the caller.

```text
caller
    |
    | credentials / token
    v
Edge Function
    |
    +-- accept
    +-- reject
```

The function decides whether the incoming request is allowed before
performing its work.

## CORS

A function invoked from browser JavaScript is subject to the browser's
same-origin policy.

If the function lives on a different origin than the application, the
response must include the right CORS headers.

```text
browser
    |
    | preflight + request
    v
Edge Function
    |
    | CORS headers
    v
browser
```

Without CORS headers, a browser will block the request even though the
function runs correctly.

## Local development

Edge Functions are developed and deployed with the Supabase CLI.

The workflow is:

```text
supabase init
    |
    v
supabase functions new <name>
    |
    v
supabase functions serve
    |
    v
supabase functions deploy
```

`init` sets up Supabase configuration in the project.

`functions new` creates a function folder with a stub handler.

`functions serve` runs the function locally for development.

`functions deploy` publishes the function to the project.

## Function folder layout

Each function is a folder inside the project.

```text
supabase
    |
    +-- functions
    |       |
    |       +-- notify
    |       |       |
    |       |       +-- index.ts
    |       |
    |       +-- other-function
    |               |
    |               +-- index.ts
    |
    +-- config.toml
```

The folder contains the function's code, and the CLI knows how to deploy
each folder by name.

A TanStack Start project and its Supabase functions can therefore live in
the same repository.

```text
tanstack-start-app/
    |
    +-- src/
    |
    +-- supabase/functions/
    |       |
    |       +-- notify/index.ts
    |
    +-- package.json
```

The application code and the functions stay versioned together.

## Limits and constraints

Edge Functions run in a constrained environment.

Each function runs for a limited time before it must finish:

```text
request arrives -> handler runs -> respond before the limit
```

Long-running background work is not the model here. Functions are designed
for short, discrete operations.

If a job would take much longer than an HTTP request can stay open, it does
not belong in an Edge Function as a single blocking request.

## Authenticating callers with Supabase Auth

An Edge Function that is called by an authenticated client can verify the
caller.

```text
browser request
    |
    | access token
    v
Edge Function
    |
    v
getUser
    |
    +-- valid user  ->  proceed
    +-- invalid     ->  401
```

The function uses the token from the request to ask Supabase Auth who is
calling, then decides whether that caller may perform the operation.

This is the same identity model used by the rest of the platform.

## Invoking a function from the client

The Supabase client can call a deployed function.

```text
supabase.functions.invoke("hello")
    |
    v
Edge Function
    |
    v
response
```

`invoke` sends an HTTP request to the function and returns the parsed
response.

The client API is the same whether the function runs locally or remotely.

## Edge Functions and Storage

Edge Functions pair naturally with Storage from the previous lesson.

A common shape is:

```text
uploaded file
    |
    v
Edge Function
    |
    +-- validate the object
    +-- transform it
    +-- update a database row
    +-- notify another service
```

For example, when a cover image is uploaded, a function could receive the
event, process the image, and update the album record.

The function holds the server-side context that browser code should not
have, while the upload itself can still happen directly from the browser.

## The complete edge function model

```text
external event / browser
        |
        v
   Edge Function
        |
        +-- secrets
        +-- service role
        +-- external APIs
        +-- database
        +-- Storage
        |
        v
     response
```

Edge Functions extend the application's server-side capabilities without
changing the TanStack Start architecture.

They are one more way to run code that must not live in the browser.

## Next step

The next lesson covers Supabase Realtime, which pushes database changes and
application events to connected clients as they happen.
