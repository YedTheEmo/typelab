# TanStack Start route state and navigation - typing

This lesson types dynamic route parameters, validated search parameters,
loader dependencies, internal links, parameterized links, search links, and
programmatic navigation.

## Read a path parameter

A dynamic route declares its parameter through `$`.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create a route containing a dynamic user identifier
export const Route = createFileRoute("/users/$userId")({
    // load data using the matched route parameter
    loader: ({ params }) => {
        // return the identifier extracted from the URL
        return {
            userId: params.userId,
        }
    },

    // provide the component rendered for this route
    component: User,
})

// define the user component
function User() {
    // retrieve the data produced by the loader
    const { userId } = Route.useLoaderData()

    // render the current user identifier
    return <h1>User {userId}</h1>
}
```

For:

```text
/users/42
```

the parameter is:

```text
userId = "42"
```

## Validate search parameters

A route can transform URL search values into application values.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the products route
export const Route = createFileRoute("/products")({
    // transform the incoming URL search state
    validateSearch: (search) => ({
        // convert the page value into a number
        page: Number(search.page ?? 1),
    }),

    // provide the component rendered for this route
    component: Products,
})

// define the products component
function Products() {
    // read the validated search state
    const { page } = Route.useSearch()

    // render the current page
    return <h1>Products page {page}</h1>
}
```

The URL:

```text
/products?page=3
```

produces:

```ts
{
    page: 3,
}
```

rather than requiring the component to repeatedly parse the string `"3"`.

## Use search state as a loader dependency

Search state can determine which data the loader should retrieve.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the products route
export const Route = createFileRoute("/products")({
    // validate the URL search state
    validateSearch: (search) => ({
        // convert the page into a number
        page: Number(search.page ?? 1),
    }),

    // identify which search values affect the loader
    loaderDeps: ({ search }) => ({
        // declare page as a loader dependency
        page: search.page,
    }),

    // load data using the declared dependency
    loader: ({ deps }) => {
        // return the page represented by the current route
        return {
            page: deps.page,
        }
    },

    // provide the component rendered for this route
    component: Products,
})

// define the products component
function Products() {
    // retrieve the loader data
    const { page } = Route.useLoaderData()

    // render the loaded page
    return <h1>Products page {page}</h1>
}
```

The important chain is:

```text
URL
 |
 v
validateSearch
 |
 v
loaderDeps
 |
 v
loader
 |
 v
route data
```

## Create an internal link

Use the router's `Link` component for application-internal navigation.

```tsx
// import the router link component
import { Link } from "@tanstack/react-router"

// define navigation between application routes
function Navigation() {
    // render a link to the users route
    return <Link to="/users">Users</Link>
}
```

The router knows that `/users` is an application route and can coordinate
the resulting navigation.

## Create a parameterized link

A dynamic route needs its parameter.

```tsx
// import the router link component
import { Link } from "@tanstack/react-router"

// define a link to a particular user
function UserLink() {
    // render the user route with its dynamic parameter
    return (
        <Link
            to="/users/$userId"
            params={{
                userId: "42",
            }}
        >
            Ada
        </Link>
    )
}
```

The destination is conceptually:

```text
/users/42
```

The parameter is represented structurally rather than manually inserted
into a string.

## Provide search state to a link

Search parameters can also be represented structurally.

```tsx
// import the router link component
import { Link } from "@tanstack/react-router"

// define a link to the second products page
function ProductsLink() {
    // render the products route with search state
    return (
        <Link
            to="/products"
            search={{
                page: 2,
            }}
        >
            Page 2
        </Link>
    )
}
```

The resulting location is conceptually:

```text
/products?page=2
```

## Navigate programmatically

Navigation can also happen in code.

```tsx
// import the hook used for programmatic router navigation
import { useNavigate } from "@tanstack/react-router"

// define a component that performs navigation
function ContinueButton() {
    // obtain the navigation function
    const navigate = useNavigate()

    // define the button's navigation behavior
    return (
        <button
            onClick={() => {
                // navigate to the dashboard route
                navigate({
                    to: "/dashboard",
                })
            }}
        >
            Continue
        </button>
    )
}
```

This is useful when navigation is a consequence of an application event,
such as a successful mutation.

## Combine mutation and navigation

A common pattern is:

```text
submit form
    |
    v
server mutation
    |
    v
success
    |
    v
navigate
```

A simplified implementation looks like:

```tsx
// import the React hook used for programmatic navigation
import { useNavigate } from "@tanstack/react-router"

// import the server-side mutation
import { createUser } from "../server/create-user"

// define a component that creates a user and then navigates
function CreateUser() {
    // obtain the navigation function
    const navigate = useNavigate()

    // define the mutation workflow
    async function handleCreate() {
        // perform the server-side mutation
        await createUser({
            data: {
                name: "Ada",
            },
        })

        // navigate after successful completion
        await navigate({
            to: "/users",
        })
    }

    // render the action button
    return (
        <button onClick={handleCreate}>
            Create user
        </button>
    )
}
```

The mutation changes server state.

The navigation changes application location.

Keeping those responsibilities conceptually separate makes the workflow
easier to reason about.

## Route context

Route context can provide shared values to descendant routes.

The exact context setup depends on the application's route configuration,
but the conceptual structure is:

```text
root route
    |
    +-- context
          |
          v
      child route
```

A child loader can consume context supplied by an ancestor.

This is different from URL state because the value does not come from the
location.

## Now type it again

Type the parameterized route.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create a route containing a dynamic user identifier
export const Route = createFileRoute("/users/$userId")({
    // return data derived from the matched route parameter
    loader: ({ params }) => ({
        // expose the URL parameter as route data
        userId: params.userId,
    }),

    // provide the route component
    component: User,
})

// define the user component
function User() {
    // retrieve the data produced by the loader
    const { userId } = Route.useLoaderData()

    // render the user identifier
    return <h1>User {userId}</h1>
}
```

Type a validated search route.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the products route
export const Route = createFileRoute("/products")({
    // transform URL search state into application state
    validateSearch: (search) => ({
        // convert the page value into a number
        page: Number(search.page ?? 1),
    }),

    // provide the component rendered for the route
    component: Products,
})

// define the products component
function Products() {
    // retrieve the validated search state
    const { page } = Route.useSearch()

    // render the current page
    return <h1>Products page {page}</h1>
}
```

Finally, type a parameterized internal link.

```tsx
// import the router link component
import { Link } from "@tanstack/react-router"

// define a link to a specific user
function UserLink() {
    // render the dynamic route with its parameter
    return (
        <Link
            to="/users/$userId"
            params={{
                userId: "42",
            }}
        >
            Ada
        </Link>
    )
}
```

## Wrap up

The flow: URL -> structured route state -> navigation -> loaders ->
components
