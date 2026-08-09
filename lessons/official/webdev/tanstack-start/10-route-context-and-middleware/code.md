# TanStack Start route context and middleware - typing

This lesson types the basic shapes used to establish route context and
illustrates the separation between route infrastructure and server
operations.

## Define route context

The router can be configured with a context type.

```tsx
// import the helper used to create the root route
import { createRootRouteWithContext } from "@tanstack/react-router"

// describe the values available through route context
type RouterContext = {
    user: {
        id: string
    } | null
}

// create the root route with the context type
export const Route = createRootRouteWithContext<RouterContext>()({
    // render the root application
    component: Root,
})

// define the root component
function Root() {
    // render the application's child routes
    return <div />
}
```

The type describes what descendants can expect to receive through the
router context.

## Supply context to the router

The actual context value is supplied when the router is created.

```tsx
// create the router from the generated route tree
const router = createRouter({
    routeTree,

    // provide the runtime context available to routes
    context: {
        user: null,
    },
})
```

The important distinction is:

```text
type definition
    -> describes context

router creation
    -> supplies context
```

## Consume context in a route

A route can access the context through its loader.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the dashboard route
export const Route = createFileRoute("/dashboard")({
    // receive route context while loading the route
    loader: ({ context }) => {
        // return whether a user exists
        return {
            authenticated: context.user !== null,
        }
    },

    // provide the dashboard component
    component: Dashboard,
})

// define the dashboard component
function Dashboard() {
    // retrieve the loader result
    const { authenticated } = Route.useLoaderData()

    // render the authentication state
    return <p>Authenticated: {String(authenticated)}</p>
}
```

The loader receives context because the router was configured with the
context type and runtime value.

## Context is not authorization

The previous example only reads authentication-related information.

A sensitive server operation still needs its own server-side authorization
check.

The browser should never be treated as the final security boundary.

## Middleware concept

Middleware wraps execution around another operation.

The exact middleware API depends on the Start layer being configured, but
the conceptual structure remains:

```tsx
const middleware = async ({ next }) => {
    // code before the operation

    const result = await next()

    // code after the operation

    return result
}
```

The important operation is `next()`.

It represents continuing execution into the next middleware or final handler.

## Think in layers

A request can conceptually pass through several layers:

```text
request
   |
   v
logging
   |
   v
authentication
   |
   v
authorization
   |
   v
handler
```

Each layer has one responsibility.

This is preferable to putting all of these concerns into every individual
server function.

## Practice

Type the route context definition again:

```tsx
type RouterContext = {
    user: {
        id: string
    } | null
}
```

Then type the root route:

```tsx
export const Route = createRootRouteWithContext<RouterContext>()({
    component: Root,
})
```

Then type the runtime context:

```tsx
const router = createRouter({
    routeTree,
    context: {
        user: null,
    },
})
```

Finally, type the route loader that consumes it:

```tsx
export const Route = createFileRoute("/dashboard")({
    loader: ({ context }) => ({
        authenticated: context.user !== null,
    }),
    component: Dashboard,
})
```

The important flow is:

```text
router context
      |
      v
route
      |
      v
loader
      |
      v
route data
```

Context supplies information; loaders use that information; middleware
surrounds execution.
