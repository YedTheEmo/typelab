# TanStack Start project structure and routing - typing

This lesson types the core route structure: create the root route, add an
index route, add another URL, and observe how the files form a route tree.

## Create the root route

The root route sits above every other route in the application.

```tsx
// import the helper used to create the root route
import { createRootRoute } from "@tanstack/react-router"

// create the root route for the application
export const Route = createRootRoute({
    // provide the component rendered by the root route
    component: RootComponent,
})

// define the application-wide root component
function RootComponent() {
    // render the root route's content
    return <div>Application</div>
}
```

## Create the index route

The index route represents the root URL of the application.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the route associated with the root URL
export const Route = createFileRoute("/")({
    // provide the component rendered at the root URL
    component: Home,
})

// define the component for the root URL
function Home() {
    // render the application's home content
    return <h1>Home</h1>
}
```

The root route and index route have different responsibilities.

```text
__root.tsx -> root of the route hierarchy
index.tsx  -> content at /
```

## Create another route

A file named `about.tsx` represents the `/about` URL.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the route associated with the about URL
export const Route = createFileRoute("/about")({
    // provide the component rendered at the about URL
    component: About,
})

// define the component for the about URL
function About() {
    // render the about page content
    return <h1>About</h1>
}
```

The filename and route path now describe the same URL.

```text
src/routes/about.tsx
        |
        v
     /about
```

## Add route data

A route module can contain behavior in addition to its component.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the route associated with the about URL
export const Route = createFileRoute("/about")({
    // load data associated with this route
    loader: () => {
        // return the value required by the route component
        return {
            title: "About",
        }
    },

    // provide the component rendered at the about URL
    component: About,
})

// define the component for the about URL
function About() {
    // retrieve the data loaded by this route
    const data = Route.useLoaderData()

    // render the loaded title
    return <h1>{data.title}</h1>
}
```

The route file is therefore a module containing both routing configuration
and the behavior associated with that route.

## Visualize the route tree

The three files now form a simple hierarchy.

```text
src/routes/
    __root.tsx
    index.tsx
    about.tsx
```

The router interprets them conceptually as:

```text
__root
├── index
└── about
```

The root URL and the about URL therefore share the same root route.

## Now type it again

Type the complete route modules again to reinforce the relationship between
the filenames and the route definitions.

The root route provides the application-wide parent.

```tsx
// import the helper used to create the root route
import { createRootRoute } from "@tanstack/react-router"

// create the root route for the application
export const Route = createRootRoute({
    // provide the component rendered by the root route
    component: RootComponent,
})

// define the application-wide root component
function RootComponent() {
    // render the root route's content
    return <div>Application</div>
}
```

The index route provides the root URL.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the route associated with the root URL
export const Route = createFileRoute("/")({
    // provide the component rendered at the root URL
    component: Home,
})

// define the component for the root URL
function Home() {
    // render the application's home content
    return <h1>Home</h1>
}
```

The about route provides another URL beneath the root.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the route associated with the about URL
export const Route = createFileRoute("/about")({
    // provide the component rendered at the about URL
    component: About,
})

// define the component for the about URL
function About() {
    // render the about page content
    return <h1>About</h1>
}
```

## Wrap up

The flow: files -> route modules -> generated route tree -> URL matching
