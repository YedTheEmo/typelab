# TanStack Start database access - typing

This lesson types the application shape around a database without tying the
lesson to a particular ORM.

## Keep the database client server-side

A database module should remain part of server-side application code.

```tsx
// import the database client from the application's server-side database module
import { db } from "./db"

// define a server-side function that reads users
export async function getUsers() {
    // execute the database query on the server
    return db.user.findMany({
        // select only fields required by the application
        select: {
            id: true,
            name: true,
        },
    })
}
```

The exact `db` API depends on the database library.

The important architecture is that the database client is imported by
server-side code rather than browser components.

## Use database data in a loader

A route loader can call the server-side database operation.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// import the server-side database operation
import { getUsers } from "../server/users"

// create the users route
export const Route = createFileRoute("/users")({
    // load the users required by this route
    loader: () => getUsers(),

    // provide the component rendered for this route
    component: Users,
})

// define the users component
function Users() {
    // retrieve the database-backed route data
    const users = Route.useLoaderData()

    // render the selected user fields
    return (
        <ul>
            {users.map((user) => (
                <li key={user.id}>{user.name}</li>
            ))}
        </ul>
    )
}
```

The component receives the selected representation rather than the database
client or raw database connection.

## Mutate through a server function

A database mutation can be placed behind a server function.

```tsx
// import the helper used to create a server-side function
import { createServerFn } from "@tanstack/react-start"

// import the server-side database client
import { db } from "../server/db"

// create a server function for creating a user
export const createUser = createServerFn({
    // declare that this operation changes server-side state
    method: "POST",
})
    // validate the expected input shape
    .inputValidator((data: { name: string }) => data)
    // perform the database mutation on the server
    .handler(async ({ data }) => {
        // create the record in the database
        return db.user.create({
            // provide the database values
            data: {
                name: data.name,
            },

            // return only the fields the client needs
            select: {
                id: true,
                name: true,
            },
        })
    })
```

The browser invokes the server function; it never receives the database
connection itself.

## Keep authorization next to the protected operation

A database mutation should not assume that the browser already checked
permissions.

```tsx
// define a server-side operation for updating a user
async function updateUser(
    userId: string,
    currentUserId: string,
    name: string,
) {
    // reject an attempt to modify another user's record
    if (userId !== currentUserId) {
        throw new Error("Unauthorized")
    }

    // update the permitted database record
    return db.user.update({
        // identify the record being changed
        where: {
            id: userId,
        },

        // provide the new value
        data: {
            name,
        },
    })
}
```

The database operation is protected by a server-side authorization decision.

## Practice

Type the basic loader architecture:

```tsx
export const Route = createFileRoute("/users")({
    loader: () => getUsers(),
    component: Users,
})
```

Then type the database operation:

```tsx
export async function getUsers() {
    return db.user.findMany({
        select: {
            id: true,
            name: true,
        },
    })
}
```

The resulting architecture is:

```text
route
  |
  v
loader
  |
  v
server database code
  |
  v
database
```

The browser receives the result, not the database connection.
