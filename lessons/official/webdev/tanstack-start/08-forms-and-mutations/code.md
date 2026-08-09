# TanStack Start forms and mutations - typing

This lesson types a basic form, converts its browser `FormData` into an
explicit server-function input, validates that input on the server, and
handles the asynchronous mutation from the browser.

## Create the mutation server function

Start by defining the server-side operation.

```tsx
// import the helper used to create a server-side function
import { createServerFn } from "@tanstack/react-start"

// create a server function for creating a user
export const createUser = createServerFn({
    // declare that this operation changes server-side state
    method: "POST",
})
    // define the expected input shape
    .inputValidator((data: { name: string }) => data)
    // define the server-side mutation
    .handler(async ({ data }) => {
        // represent the result that would normally come from a database
        return {
            name: data.name,
        }
    })
```

The handler is where the trusted server-side operation belongs.

A real application could replace the returned object with a database
operation while keeping the browser-side form unchanged.

## Create the form

The browser can collect the user's input with a normal React form.

```tsx
// import React's form event type
import type { FormEvent } from "react"

// define the component containing the user form
function CreateUserForm() {
    // define the form submission handler
    async function handleSubmit(event: FormEvent<HTMLFormElement>) {
        // prevent the browser from performing a document navigation
        event.preventDefault()

        // read the submitted form fields
        const formData = new FormData(event.currentTarget)

        // retrieve the name field from the submitted form
        const name = formData.get("name")

        // send the form value to the server mutation
        await createUser({
            data: {
                name: String(name),
            },
        })
    }

    // render the form
    return (
        <form onSubmit={handleSubmit}>
            <input name="name" />
            <button type="submit">Create</button>
        </form>
    )
}
```

The form owns the browser interaction.

The server function owns the server-side mutation.

## Add pending state

A mutation can take time, so the UI should communicate that the submission
is in progress.

```tsx
// import React's form event type
import type { FormEvent } from "react"

// import the React hook used for local component state
import { useState } from "react"

// define the component containing the user form
function CreateUserForm() {
    // track whether the mutation is currently running
    const [pending, setPending] = useState(false)

    // define the form submission handler
    async function handleSubmit(event: FormEvent<HTMLFormElement>) {
        // prevent the browser from performing a document navigation
        event.preventDefault()

        // indicate that the mutation has started
        setPending(true)

        // read the submitted form fields
        const formData = new FormData(event.currentTarget)

        // retrieve the name field from the submitted form
        const name = formData.get("name")

        try {
            // send the form value to the server mutation
            await createUser({
                data: {
                    name: String(name),
                },
            })
        } finally {
            // indicate that the mutation has finished
            setPending(false)
        }
    }

    // render the form
    return (
        <form onSubmit={handleSubmit}>
            <input name="name" />
            <button
                type="submit"
                disabled={pending}
            >
                {pending ? "Creating..." : "Create"}
            </button>
        </form>
    )
}
```

The pending state is browser UI state.

It does not determine whether the server accepts the mutation.

It only communicates the current state of the interaction.

## Handle the result

A mutation can return useful information to the browser.

```tsx
// define the form submission handler
async function handleSubmit(
    event: FormEvent<HTMLFormElement>,
) {
    // prevent the browser from performing a document navigation
    event.preventDefault()

    // read the submitted form fields
    const formData = new FormData(event.currentTarget)

    // retrieve the name field from the submitted form
    const name = formData.get("name")

    // invoke the server mutation and wait for its result
    const user = await createUser({
        data: {
            name: String(name),
        },
    })

    // use the server's result in browser code
    console.log(user)
}
```

The server decides what result should be exposed.

The browser then consumes that result.

## Handle errors

The mutation should also account for failure.

```tsx
// define the form submission handler
async function handleSubmit(
    event: FormEvent<HTMLFormElement>,
) {
    // prevent the browser from performing a document navigation
    event.preventDefault()

    // read the submitted form fields
    const formData = new FormData(event.currentTarget)

    // retrieve the name field from the submitted form
    const name = formData.get("name")

    try {
        // invoke the server mutation
        const user = await createUser({
            data: {
                name: String(name),
            },
        })

        // process the successful result
        console.log(user)
    } catch (error) {
        // handle the failed mutation
        console.error(error)
    }
}
```

Production applications would normally translate expected server failures
into useful user-facing messages rather than simply logging the error.

## Add server-side validation

The server function should validate the input independently of the browser.

```tsx
// import the helper used to create a server-side function
import { createServerFn } from "@tanstack/react-start"

// create a server function for creating a user
export const createUser = createServerFn({
    // declare that this operation changes server-side state
    method: "POST",
})
    // validate the input received by the server
    .inputValidator((data: { name: string }) => {
        // reject an empty name
        if (!data.name.trim()) {
            throw new Error("Name is required")
        }

        // return the validated input
        return data
    })
    // perform the server-side mutation
    .handler(async ({ data }) => {
        // return the created user representation
        return {
            name: data.name,
        }
    })
```

The important security boundary is the server validator.

Even if the browser has its own validation, the server must not assume that
the browser's rules were followed.

## Connect the pieces

The complete client-side form can now invoke the server operation.

```tsx
// import React's form event type
import type { FormEvent } from "react"

// import the React hook used for local component state
import { useState } from "react"

// import the server-side mutation
import { createUser } from "./create-user"

// define the component containing the user form
function CreateUserForm() {
    // track whether the mutation is currently running
    const [pending, setPending] = useState(false)

    // define the form submission handler
    async function handleSubmit(event: FormEvent<HTMLFormElement>) {
        // prevent the browser from performing a document navigation
        event.preventDefault()

        // indicate that the mutation has started
        setPending(true)

        // read the submitted form fields
        const formData = new FormData(event.currentTarget)

        // retrieve the name field from the submitted form
        const name = formData.get("name")

        try {
            // invoke the server mutation
            const user = await createUser({
                data: {
                    name: String(name),
                },
            })

            // process the successful server result
            console.log(user)
        } catch (error) {
            // handle the failed mutation
            console.error(error)
        } finally {
            // indicate that the mutation has finished
            setPending(false)
        }
    }

    // render the form
    return (
        <form onSubmit={handleSubmit}>
            <input name="name" />
            <button
                type="submit"
                disabled={pending}
            >
                {pending ? "Creating..." : "Create"}
            </button>
        </form>
    )
}
```

The architecture is now explicit:

```text
<form>
    |
    v
FormData
    |
    v
createUser()
    |
    v
server validator
    |
    v
server handler
    |
    v
mutation
    |
    v
result
    |
    v
browser UI
```

## Now type it again

Type the server mutation.

```tsx
// import the helper used to create a server-side function
import { createServerFn } from "@tanstack/react-start"

// create a server function for creating a user
export const createUser = createServerFn({
    // declare that this operation changes server-side state
    method: "POST",
})
    // validate the input received by the server
    .inputValidator((data: { name: string }) => {
        // reject an empty name
        if (!data.name.trim()) {
            throw new Error("Name is required")
        }

        // return the validated input
        return data
    })
    // perform the server-side mutation
    .handler(async ({ data }) => {
        // return the created user representation
        return {
            name: data.name,
        }
    })
```

Then type the form that invokes it.

```tsx
// import React's form event type
import type { FormEvent } from "react"

// import the React hook used for local component state
import { useState } from "react"

// import the server-side mutation
import { createUser } from "./create-user"

// define the component containing the user form
function CreateUserForm() {
    // track whether the mutation is currently running
    const [pending, setPending] = useState(false)

    // define the form submission handler
    async function handleSubmit(event: FormEvent<HTMLFormElement>) {
        // prevent the browser from performing a document navigation
        event.preventDefault()

        // indicate that the mutation has started
        setPending(true)

        // read the submitted form fields
        const formData = new FormData(event.currentTarget)

        // retrieve the name field from the submitted form
        const name = formData.get("name")

        try {
            // invoke the server mutation
            await createUser({
                data: {
                    name: String(name),
                },
            })
        } finally {
            // indicate that the mutation has finished
            setPending(false)
        }
    }

    // render the form
    return (
        <form onSubmit={handleSubmit}>
            <input name="name" />
            <button
                type="submit"
                disabled={pending}
            >
                {pending ? "Creating..." : "Create"}
            </button>
        </form>
    )
}
```

## Wrap up

The flow: form -> FormData -> server function -> server validation ->
mutation -> result -> UI
