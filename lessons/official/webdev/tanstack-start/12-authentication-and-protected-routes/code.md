# TanStack Start authentication and protected routes - typing

The exact authentication implementation depends on the authentication
library being used. This lesson focuses on the TanStack Start structure
around that system.

## Represent the authenticated user

```tsx
// describe the authenticated user made available to the route tree
type User = {
    id: string
    name: string
}

// describe the router context
type RouterContext = {
    user: User | null
}
```

The important state is:

```text
user
 |
 +-- User -> authenticated
 |
 +-- null -> unauthenticated
```

## Supply authentication state through context

```tsx
// create the router with the route tree
const router = createRouter({
    routeTree,

    // provide the current authenticated user
    context: {
        user: await getCurrentUser(),
    },
})
```

`getCurrentUser()` represents whatever session or authentication mechanism
the application uses.

It is server-side logic; it is not a browser-controlled value.

## Protect a route

A protected route can inspect the current user before loading its content.

```tsx
// import the helper used to create a file-based route
import {
    createFileRoute,
    redirect,
} from "@tanstack/react-router"

// create the protected dashboard route
export const Route = createFileRoute("/dashboard")({
    // execute before loading the dashboard
    beforeLoad: ({ context }) => {
        // reject unauthenticated access
        if (!context.user) {
            throw redirect({
                to: "/login",
            })
        }
    },

    // provide the dashboard component
    component: Dashboard,
})

// define the dashboard component
function Dashboard() {
    // render protected UI
    return <h1>Dashboard</h1>
}
```

The route guard improves navigation behavior by preventing an unauthenticated
user from entering the protected route.

## Protect the mutation too

A mutation should independently enforce authorization.

```tsx
// create a protected server-side mutation
export const updateProfile = createServerFn({
    // this operation changes persistent state
    method: "POST",
})
    // validate the mutation input
    .inputValidator((data: { name: string }) => data)
    // perform the protected operation
    .handler(async ({ data }) => {
        // obtain the authenticated user on the server
        const user = await getCurrentUser()

        // reject unauthenticated requests
        if (!user) {
            throw new Error("Unauthenticated")
        }

        // perform the update for the authenticated user
        return db.user.update({
            where: {
                id: user.id,
            },
            data: {
                name: data.name,
            },
        })
    })
```

The important property is that the database operation uses the authenticated
identity established by the server rather than trusting a user ID supplied by
the browser.

## Practice

Type the security flow:

```text
request
   |
   v
getCurrentUser()
   |
   +-- null -> reject / redirect
   |
   v
authenticated user
   |
   v
authorization
   |
   v
database operation
```

Then remember the distinction:

```text
route protection
    -> controls navigation and UI access

server authorization
    -> protects the actual operation
```

Both can exist, but only the server-side check should be treated as the
security boundary.
