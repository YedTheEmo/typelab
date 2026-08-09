# TanStack Start loaders and data loading - typing

This lesson types route data loading: return loader data, read it in a
component, load data from route parameters, and declare loader dependencies.

## Return route data

A loader returns the data required by its route.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the route associated with the users URL
export const Route = createFileRoute("/users")({
    // load the data required by the users route
    loader: () => {
        // return the users that the route will expose
        return {
            users: ["Ada", "Grace"],
        }
    },

    // provide the component rendered for this route
    component: Users,
})

// define the component that displays the users
function Users() {
    // retrieve the data produced by the route loader
    const { users } = Route.useLoaderData()

    // render the number of loaded users
    return <p>{users.length} users</p>
}
```

The loader produces route data, and the component consumes that data.

## Load data from parameters

A loader can use dynamic route parameters.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create a route containing a dynamic user identifier
export const Route = createFileRoute("/users/$userId")({
    // load data using the matched route parameters
    loader: ({ params }) => {
        // return data derived from the user identifier
        return {
            userId: params.userId,
        }
    },

    // provide the component rendered for this route
    component: User,
})

// define the component for an individual user
function User() {
    // retrieve the data produced by the route loader
    const { userId } = Route.useLoaderData()

    // render the loaded user identifier
    return <h1>User {userId}</h1>
}
```

For a URL such as `/users/42`, the loader receives `42` through `params`.

## Define loader dependencies

A loader can depend on route state such as search parameters.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the products route
export const Route = createFileRoute("/products")({
    // describe the search value that affects the loader
    loaderDeps: ({ search }) => ({
        // identify the page as a loader dependency
        page: search.page,
    }),

    // load data using the declared dependency
    loader: ({ deps }) => {
        // return data associated with the current page
        return {
            page: deps.page,
        }
    },

    // provide the component rendered for this route
    component: Products,
})

// define the products component
function Products() {
    // retrieve the data produced by the route loader
    const { page } = Route.useLoaderData()

    // render the current page
    return <h1>Products page {page}</h1>
}
```

The dependency tells the router that the loader's result is associated with
the value represented by `page`.

## Combine search validation and loading

A route can validate search parameters before using them as loader
dependencies.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the products route
export const Route = createFileRoute("/products")({
    // convert the incoming page value into route state
    validateSearch: (search) => ({
        // use page one when the URL does not provide a page
        page: Number(search.page ?? 1),
    }),

    // identify the validated page as a loader dependency
    loaderDeps: ({ search }) => ({
        // associate the loader with the current page
        page: search.page,
    }),

    // load data for the current page
    loader: ({ deps }) => {
        // return the page required by the route
        return {
            page: deps.page,
        }
    },

    // provide the component rendered for this route
    component: Products,
})

// define the products component
function Products() {
    // retrieve the data produced by the route loader
    const { page } = Route.useLoaderData()

    // render the loaded page
    return <h1>Products page {page}</h1>
}
```

The resulting relationship is:

```text
URL
 |
 v
search validation
 |
 v
loader dependency
 |
 v
loader
 |
 v
route data
 |
 v
component
```

## Now type it again

Type a complete parameter-driven route.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create a route containing a dynamic user identifier
export const Route = createFileRoute("/users/$userId")({
    // load data using the matched route parameters
    loader: ({ params }) => {
        // return data derived from the user identifier
        return {
            userId: params.userId,
        }
    },

    // provide the component rendered for this route
    component: User,
})

// define the component for an individual user
function User() {
    // retrieve the data produced by the route loader
    const { userId } = Route.useLoaderData()

    // render the loaded user identifier
    return <h1>User {userId}</h1>
}
```

Type the dependency-driven route again.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the products route
export const Route = createFileRoute("/products")({
    // convert the incoming page value into route state
    validateSearch: (search) => ({
        // use page one when the URL does not provide a page
        page: Number(search.page ?? 1),
    }),

    // identify the validated page as a loader dependency
    loaderDeps: ({ search }) => ({
        // associate the loader with the current page
        page: search.page,
    }),

    // load data for the current page
    loader: ({ deps }) => {
        // return the page required by the route
        return {
            page: deps.page,
        }
    },

    // provide the component rendered for this route
    component: Products,
})

// define the products component
function Products() {
    // retrieve the data produced by the route loader
    const { page } = Route.useLoaderData()

    // render the loaded page
    return <h1>Products page {page}</h1>
}
```

## Wrap up

The flow: route -> dependencies -> loader -> route data -> component
