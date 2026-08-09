# TanStack Start SSR and the execution model - typing

This lesson types the two sides of the Start execution model: a route that
can render on the server and a browser component that participates after
hydration.

## Define a server-rendered route

A route can provide the component used to render the initial document.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the route associated with the root URL
export const Route = createFileRoute("/")({
    // provide the component rendered for this route
    component: Home,
})

// define the component rendered by the route
function Home() {
    // render HTML that can be produced during server rendering
    return (
        <main>
            <h1>Hello from the server</h1>
        </main>
    )
}
```

The component does not require browser-only APIs, so it can participate in
server rendering.

## Keep rendering code environment-safe

Code rendered on the server cannot assume that browser APIs exist.

```tsx
// define a component that works in both environments
function Greeting() {
    // render static content that does not require browser APIs
    return <p>This component can render on the server</p>
}
```

The important property is that the render operation does not depend on
`window`, `document`, or another browser-only global.

## Separate browser interaction

Browser-only behavior can happen in response to browser interaction.

```tsx
// import the React hook used for browser-side component state
import { useState } from "react"

// define an interactive browser component
function Counter() {
    // create state that changes when the user interacts
    const [count, setCount] = useState(0)

    // render the interactive counter
    return (
        <button
            onClick={() => {
                // increment the browser-side state
                setCount(count + 1)
            }}
        >
            Count: {count}
        </button>
    )
}
```

The component can be included in server-rendered markup, then become
interactive after hydration.

## Avoid browser APIs during rendering

Browser globals should not be required during server rendering.

```tsx
// define a component whose render output is environment-independent
function SafeComponent() {
    // return content that works during server rendering and hydration
    return <p>Safe during server rendering</p>
}
```

The component can later interact with browser APIs from appropriate
browser-side code rather than assuming those APIs exist everywhere.

## Load data for server rendering

A route loader can provide data before the route component renders.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the route associated with the users URL
export const Route = createFileRoute("/users")({
    // load the data required by the route
    loader: () => {
        // return the users needed by the route component
        return {
            users: ["Ada", "Grace"],
        }
    },

    // provide the component rendered for this route
    component: Users,
})

// define the component that displays the loaded users
function Users() {
    // retrieve the data produced by the route loader
    const { users } = Route.useLoaderData()

    // render the loaded users
    return (
        <main>
            <h1>Users</h1>
            <p>{users.join(", ")}</p>
        </main>
    )
}
```

During an initial server render, the route's data can participate in
producing the HTML that the browser receives.

## Render the interactive application

A server-rendered component can contain interactive components.

```tsx
// import the React hook used for browser-side component state
import { useState } from "react"

// define an interactive component that can hydrate in the browser
function Counter() {
    // create state that the browser can update after hydration
    const [count, setCount] = useState(0)

    // render a button that changes the hydrated component state
    return (
        <button
            onClick={() => {
                // increment the current counter value
                setCount(count + 1)
            }}
        >
            Count: {count}
        </button>
    )
}
```

The server can produce the initial markup, while hydration allows the
browser-side React application to respond to the button interaction.

## Now type it again

Type a server-safe route.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the route associated with the root URL
export const Route = createFileRoute("/")({
    // provide the component rendered for this route
    component: Home,
})

// define the component rendered by the route
function Home() {
    // render HTML that does not require browser APIs
    return (
        <main>
            <h1>Hello from the server</h1>
        </main>
    )
}
```

Type a route with loaded data.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the route associated with the users URL
export const Route = createFileRoute("/users")({
    // load the data required by the route
    loader: () => {
        // return the users needed by the route component
        return {
            users: ["Ada", "Grace"],
        }
    },

    // provide the component rendered for this route
    component: Users,
})

// define the component that displays the loaded users
function Users() {
    // retrieve the data produced by the route loader
    const { users } = Route.useLoaderData()

    // render the loaded users
    return (
        <main>
            <h1>Users</h1>
            <p>{users.join(", ")}</p>
        </main>
    )
}
```

Finally, type the interactive component that can become active after
hydration.

```tsx
// import the React hook used for browser-side component state
import { useState } from "react"

// define an interactive component that can hydrate in the browser
function Counter() {
    // create state that the browser can update after hydration
    const [count, setCount] = useState(0)

    // render a button that changes the hydrated component state
    return (
        <button
            onClick={() => {
                // increment the current counter value
                setCount(count + 1)
            }}
        >
            Count: {count}
        </button>
    )
}
```

## Wrap up

The flow: request -> server render -> HTML -> hydration -> client navigation
