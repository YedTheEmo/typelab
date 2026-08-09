# TanStack Start navigation and URL state - typing

This lesson types route navigation: create links, define a dynamic route,
read its parameter, and represent filter state with search parameters.

## Create application links

The `Link` component creates navigation connected to the route tree.

```tsx
// import the component used for router-aware navigation
import { Link } from "@tanstack/react-router"

// define the navigation shown by the application
function Navigation() {
    // render a link to the home route
    return (
        <nav>
            <Link to="/">Home</Link>
            <Link to="/about">About</Link>
        </nav>
    )
}
```

The destinations are route locations rather than calls to manually change
the browser URL.

## Define a dynamic route

A dynamic segment represents a value that changes between URL instances.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create a route containing a dynamic user identifier
export const Route = createFileRoute("/users/$userId")({
    // provide the component rendered for the user route
    component: User,
})

// define the component for an individual user
function User() {
    // retrieve the parameters matched by this route
    const { userId } = Route.useParams()

    // render the matched user identifier
    return <h1>User {userId}</h1>
}
```

A URL such as:

```text
/users/42
```

therefore produces:

```text
userId = "42"
```

The route parameter belongs to the route's path.

## Link to a dynamic route

A dynamic route requires a value for its parameter.

```tsx
// import the component used for router-aware navigation
import { Link } from "@tanstack/react-router"

// define navigation to a specific user
function UserLink() {
    // render a link containing the required route parameter
    return (
        <Link
            to="/users/$userId"
            params={{ userId: "42" }}
        >
            View user
        </Link>
    )
}
```

The route definition establishes that `userId` is required.

The navigation supplies the value.

## Define search state

Search parameters represent state associated with a route.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the products route
export const Route = createFileRoute("/products")({
    // define the default search state for the route
    validateSearch: (search) => ({
        // convert the page value into a number
        page: Number(search.page ?? 1),
    }),

    // provide the component rendered for the products route
    component: Products,
})
```

The route now describes a `page` value as part of its URL state.

## Read search state

The component can retrieve the validated search state.

```tsx
// define the component for the products route
function Products() {
    // retrieve the validated search parameters
    const { page } = Route.useSearch()

    // render the current page number
    return <h1>Products page {page}</h1>
}
```

A URL such as:

```text
/products?page=2
```

can therefore produce:

```text
page = 2
```

The route has converted URL input into application state.

## Navigate with search state

A link can supply search parameters as part of its destination.

```tsx
// import the component used for router-aware navigation
import { Link } from "@tanstack/react-router"

// define pagination navigation
function Pagination() {
    // render a link that changes the products page
    return (
        <Link
            to="/products"
            search={{ page: 2 }}
        >
            Page 2
        </Link>
    )
}
```

The navigation changes the URL state without changing the route path.

The distinction is:

```text
/products
/products?page=2
```

Both URLs match the same route, but they contain different search state.

## Programmatic navigation

Navigation can also happen in response to application logic.

```tsx
// import the hook used for programmatic navigation
import { useNavigate } from "@tanstack/react-router"

// define a component that navigates after an event
function ContinueButton() {
    // retrieve the router navigation function
    const navigate = useNavigate()

    // render a button that changes the current route
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

The application does not manually assign a new browser URL.

It asks the router to transition to another route.

## Now type it again

Type the dynamic route again.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create a route containing a dynamic user identifier
export const Route = createFileRoute("/users/$userId")({
    // provide the component rendered for the user route
    component: User,
})

// define the component for an individual user
function User() {
    // retrieve the parameters matched by this route
    const { userId } = Route.useParams()

    // render the matched user identifier
    return <h1>User {userId}</h1>
}
```

Type the search-parameter route again.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the products route
export const Route = createFileRoute("/products")({
    // define the default search state for the route
    validateSearch: (search) => ({
        // convert the page value into a number
        page: Number(search.page ?? 1),
    }),

    // provide the component rendered for the products route
    component: Products,
})

// define the component for the products route
function Products() {
    // retrieve the validated search parameters
    const { page } = Route.useSearch()

    // render the current page number
    return <h1>Products page {page}</h1>
}
```

Type the programmatic navigation again.

```tsx
// import the hook used for programmatic navigation
import { useNavigate } from "@tanstack/react-router"

// define a component that navigates after an event
function ContinueButton() {
    // retrieve the router navigation function
    const navigate = useNavigate()

    // render a button that changes the current route
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

## Wrap up

The flow: route tree -> navigation -> path parameters -> search state -> route
