# TanStack Start server functions - typing

This lesson types server functions for retrieving and mutating data, passing
input across the server boundary, and calling the resulting functions from
application code.

## Create a GET server function

A server function can represent a server-side read operation.

```tsx
// import the helper used to create a server-side function
import { createServerFn } from "@tanstack/react-start"

// create a server function for retrieving users
export const getUsers = createServerFn({
    // declare that this operation retrieves data
    method: "GET",
}).handler(() => {
    // return data from the server-side operation
    return ["Ada", "Grace"]
})
```

The handler is the server-side implementation.

The important distinction is that `getUsers` is not merely an ordinary
function exported from a module. It represents an operation that crosses
the application boundary.

## Call the server function

Application code can invoke the server function.

```tsx
// import the server function that retrieves users
import { getUsers } from "./get-users"

// define a component that requests the users
async function Users() {
    // invoke the server-side operation
    const users = await getUsers()

    // render the returned data
    return <p>{users.join(", ")}</p>
}
```

The caller does not need to know how the server operation reaches its
private resources.

## Create a POST server function

A mutation can use a POST server function.

```tsx
// import the helper used to create a server-side function
import { createServerFn } from "@tanstack/react-start"

// create a server function for creating a user
export const createUser = createServerFn({
    // declare that this operation performs a mutation
    method: "POST",
}).handler(async ({ data }) => {
    // return the newly created user representation
    return {
        name: data.name,
    }
})
```

The handler receives the input supplied to the server function.

## Validate input

Server input should be validated before it is used.

```tsx
// import the helper used to create a server-side function
import { createServerFn } from "@tanstack/react-start"

// create a server function for creating a user
export const createUser = createServerFn({
    // declare that this operation performs a mutation
    method: "POST",
})
    // define the input shape accepted by the function
    .inputValidator((data: { name: string }) => data)
    // define the server-side operation
    .handler(async ({ data }) => {
        // return the validated input as the created user representation
        return {
            name: data.name,
        }
    })
```

The validator establishes the expected input shape before the handler uses
the data.

For production applications, a schema validation library can provide
stronger runtime validation than a TypeScript annotation alone.

The important principle is:

```text
TypeScript type -> compile-time information
runtime validator -> runtime protection
```

A TypeScript type by itself does not validate data arriving from a request.

## Call the mutation

Application code can provide the required input.

```tsx
// import the server function used to create users
import { createUser } from "./create-user"

// define a function that creates a user
async function handleCreate() {
    // invoke the server operation with its input
    const user = await createUser({
        data: {
            name: "Ada",
        },
    })

    // use the result returned by the server
    console.log(user)
}
```

The browser supplies the data, while the handler performs the server-side
operation.

## Keep private resources inside the handler

A server function is useful because its handler can access server-only
resources.

```tsx
// import the helper used to create a server-side function
import { createServerFn } from "@tanstack/react-start"

// create a server function for retrieving private data
export const getPrivateData = createServerFn({
    // declare that this operation retrieves data
    method: "GET",
}).handler(async () => {
    // perform the private operation on the server
    const secretValue = process.env.PRIVATE_VALUE

    // return only the information the client is allowed to receive
    return {
        available: Boolean(secretValue),
    }
})
```

The important security property is not the particular environment variable.

The important property is that the private value remains inside the
server-side operation and is not returned to the browser.

## Use a server function from a route loader

A route loader can consume a server function when route data requires
server-side work.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// import the server-side operation required by this route
import { getUsers } from "../server/get-users"

// create the users route
export const Route = createFileRoute("/users")({
    // load the route's data through the server operation
    loader: async () => {
        // invoke the server function
        return getUsers()
    },

    // provide the component rendered for this route
    component: Users,
})

// define the users component
function Users() {
    // retrieve the data returned by the route loader
    const users = Route.useLoaderData()

    // render the loaded users
    return <p>{users.join(", ")}</p>
}
```

The architectural relationship is:

```text
route
  |
  v
loader
  |
  v
server function
  |
  v
server-only operation
```

This separates route orchestration from the actual server operation.

## Now type it again

Type a simple GET server function.

```tsx
// import the helper used to create a server-side function
import { createServerFn } from "@tanstack/react-start"

// create a server function for retrieving users
export const getUsers = createServerFn({
    // declare that this operation retrieves data
    method: "GET",
}).handler(() => {
    // return data from the server-side operation
    return ["Ada", "Grace"]
})
```

Type a POST server function with validated input.

```tsx
// import the helper used to create a server-side function
import { createServerFn } from "@tanstack/react-start"

// create a server function for creating a user
export const createUser = createServerFn({
    // declare that this operation performs a mutation
    method: "POST",
})
    // define the input shape accepted by the function
    .inputValidator((data: { name: string }) => data)
    // define the server-side operation
    .handler(async ({ data }) => {
        // return the validated input as the created user representation
        return {
            name: data.name,
        }
    })
```

Type the invocation.

```tsx
// import the server function used to create users
import { createUser } from "./create-user"

// define a function that creates a user
async function handleCreate() {
    // invoke the server operation with its input
    const user = await createUser({
        data: {
            name: "Ada",
        },
    })

    // use the result returned by the server
    console.log(user)
}
```

## Wrap up

The flow: client input -> server function -> validation -> server operation
-> result
