# Supabase edge functions - typing

This lesson types the Edge Function workflow: scaffold a function with the
Supabase CLI, write an HTTP handler that uses a secret, run it locally, and
invoke it from the application client.

## Scaffold the function

Initialize Supabase configuration in the project.

```text
# initialize Supabase configuration
supabase init
```

Create a new Edge Function.

```text
# create a function named notify
supabase functions new notify
```

The command generates a function folder containing a TypeScript handler.

## Write the handler

Edge Functions use Deno's HTTP handler model.

```ts
// respond to an HTTP request
Deno.serve(async (request) => {
    // read the HTTP method of the request
    const method = request.method

    // read the JSON body of the request
    const body = await request.json()

    // read a secret from the function environment
    const apiKey = Deno.env.get("WEBHOOK_API_KEY")

    // reject requests without the expected secret
    if (body.apiKey !== apiKey) {
        // return an unauthorized response
        return new Response("Unauthorized", {
            // indicate the failure status
            status: 401,
        })
    }

    // perform the work the endpoint is responsible for
    console.log("notification:", body.message)

    // return a successful JSON response
    return new Response(JSON.stringify({ ok: true }), {
        // identify the response as JSON
        headers: {
            "Content-Type": "application/json",
        },
    })
})
```

The function validates its caller before doing anything else.

## Add CORS headers

A browser-invoked function needs the right headers.

```ts
// respond to an HTTP request
Deno.serve(async (request) => {
    // handle the browser's preflight request
    if (request.method === "OPTIONS") {
        // respond to the preflight check
        return new Response("ok", {
            // grant the browser permission to call the function
            headers: {
                "Access-Control-Allow-Origin": "*",
                "Access-Control-Allow-Methods": "POST",
                "Access-Control-Allow-Headers": "authorization, content-type",
            },
        })
    }

    // handle the real request
    const body = await request.json()

    // return a response with the CORS header
    return new Response(JSON.stringify({ received: body }), {
        // grant the browser permission to read the response
        headers: {
            "Content-Type": "application/json",
            "Access-Control-Allow-Origin": "*",
        },
    })
})
```

Without these headers, the browser blocks the response.

## Run the function locally

Start the local Supabase stack and serve the function.

```text
# start the local Supabase services
supabase start

# serve the function locally
supabase functions serve notify
```

The function is now available at a local URL.

## Invoke the function from the client

The Supabase client calls the deployed function with `invoke`.

```tsx
// import the browser-side Supabase client
import { createClient } from "@/lib/supabase/client"

// create a client for browser-side function calls
const supabase = createClient()

// invoke the notify function
const { data, error } = await supabase.functions.invoke("notify", {
    // send the payload to the function body
    body: {
        apiKey: "the-expected-secret",
        message: "The album is ready",
    },
})
```

The client sends the JSON body and receives the function's response.

## Access the database with the service role

Inside the function, a privileged client can perform direct database work.

```ts
// import the server-side Supabase client helper
import { createClient } from "@supabase/supabase-js"

// create a function that writes a notification
Deno.serve(async (request) => {
    // create a service role client for the function
    const supabase = createClient(
        // use the project URL from the function environment
        Deno.env.get("SUPABASE_URL")!,

        // use the service role key from the function environment
        Deno.env.get("SUPABASE_SERVICE_ROLE_KEY")!,
    )

    // read the notification payload
    const { message } = await request.json()

    // insert the notification with the privileged client
    const { error } = await supabase
        .from("notifications")
        .insert({ message })

    // surface database failures
    if (error) {
        // return an internal error response
        return new Response(JSON.stringify({ error }), {
            // indicate the failure status
            status: 500,
        })
    }

    // confirm the insert succeeded
    return new Response(JSON.stringify({ ok: true }), {
        // identify the response as JSON
        headers: {
            "Content-Type": "application/json",
        },
    })
})
```

The service role credential stays in the function environment.

## Deploy the function

Publish the function to the hosted project.

```text
# deploy the notify function
supabase functions deploy notify
```

The function is now callable through the project's function URL.

## Practice

Type the core handler shape:

```ts
Deno.serve(async (request) => {
    const body = await request.json()

    const apiKey = Deno.env.get("WEBHOOK_API_KEY")

    if (body.apiKey !== apiKey) {
        return new Response("Unauthorized", { status: 401 })
    }

    return new Response(JSON.stringify({ ok: true }), {
        headers: { "Content-Type": "application/json" },
    })
})
```

Then type the client invocation:

```tsx
const { data, error } = await supabase.functions.invoke("notify", {
    body: { apiKey: "the-expected-secret", message: "The album is ready" },
})
```

The central pattern is:

```text
supabase functions new
  -> Deno.serve handler
  -> secrets + service role
  -> supabase functions deploy
  -> supabase.functions.invoke
```

Edge Functions are standalone server endpoints the application can call when
the work must happen outside the browser.
