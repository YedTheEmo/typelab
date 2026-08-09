# TanStack Start overview - typing

This lesson types a small route: define a file-based route, load data, and
render that data through the route's component.

## Define the route

A file-based route associates a URL with route behavior and a component.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the route associated with the root URL
export const Route = createFileRoute("/")({
    // provide the component rendered for this route
    component: Home,
})
```

## Add route data

A route can load data that its component will consume.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the route associated with the root URL
export const Route = createFileRoute("/")({
    // load the data required by this route
    loader: () => {
        // return the value that the route exposes to its component
        return {
            message: "Hello from TanStack Start",
        }
    },

    // provide the component rendered for this route
    component: Home,
})
```

## Read the route data

The component can retrieve the result produced by the loader.

```tsx
// define the component rendered by the route
function Home() {
    // retrieve the data produced by this route's loader
    const data = Route.useLoaderData()

    // render the loaded message
    return <h1>{data.message}</h1>
}
```

The loader and component therefore form a simple data flow.

```text
route -> loader -> data -> component
```

## Build the complete route

The pieces now form one coherent route module.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the route associated with the root URL
export const Route = createFileRoute("/")({
    // load the data required by this route
    loader: () => {
        // return the value that the route exposes to its component
        return {
            message: "Hello from TanStack Start",
        }
    },

    // provide the component rendered for this route
    component: Home,
})

// define the component rendered by the route
function Home() {
    // retrieve the data produced by this route's loader
    const data = Route.useLoaderData()

    // render the loaded message
    return <h1>{data.message}</h1>
}
```

## Understand the application flow

The route owns the URL, the loader produces its data, and the component
renders that data.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the route associated with the root URL
export const Route = createFileRoute("/")({
    // execute the route's data-loading step
    loader: () => {
        // return the data required by the component
        return {
            message: "Hello from TanStack Start",
        }
    },

    // connect the route to its React component
    component: Home,
})

// define the UI associated with the route
function Home() {
    // retrieve the value produced by the loader
    const data = Route.useLoaderData()

    // display the route data
    return <h1>{data.message}</h1>
})
```

The important flow is:

```text
URL -> route -> loader -> data -> component -> UI
```

## Now type it again

Type the complete route once more without referring to the earlier sections.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the route associated with the root URL
export const Route = createFileRoute("/")({
    // load the data required by this route
    loader: () => {
        // return the value that the route exposes to its component
        return {
            message: "Hello from TanStack Start",
        }
    },

    // provide the component rendered for this route
    component: Home,
})

// define the component rendered by the route
function Home() {
    // retrieve the data produced by this route's loader
    const data = Route.useLoaderData()

    // render the loaded message
    return <h1>{data.message}</h1>
})
```

## Wrap up

The flow: URL -> route -> loader -> data -> component -> UI
