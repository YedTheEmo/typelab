# Supabase auth - typing

This lesson types sign-up, sign-in, and sign-out with Supabase Auth in a
TanStack Start application, using server functions and a protected route.

## Sign up a user

Sign-up creates an identity in `auth.users` and returns a session.

```tsx
// import the helper for creating a server-side function
import { createServerFn } from "@tanstack/react-start"

// import the server-side Supabase client
import { createClient } from "@/lib/supabase/server"

// create a server function for signing up
export const signUp = createServerFn({
    // declare that this operation changes server-side state
    method: "POST",
})
    // validate the expected sign-up input
    .inputValidator((data: { email: string; password: string }) => data)
    // register the user on the server
    .handler(async ({ data }) => {
        // create a Supabase client for the current server request
        const supabase = createClient()

        // register the new identity
        const { data: user, error } = await supabase.auth.signUp({
            // provide the user's email
            email: data.email,

            // provide the user's password
            password: data.password,
        })

        // surface registration failures to the client
        if (error) {
            throw error
        }

        // confirm which identity was created
        return { user: user.user }
    })
```

The server client makes sure the session cookie reaches the response.

## Sign in an existing user

Sign-in exchanges credentials for a session.

```tsx
// create a server function for signing in
export const signIn = createServerFn({
    // declare that this operation changes server-side state
    method: "POST",
})
    // validate the expected sign-in input
    .inputValidator((data: { email: string; password: string }) => data)
    // authenticate the user on the server
    .handler(async ({ data }) => {
        // create a Supabase client for the current server request
        const supabase = createClient()

        // authenticate with the provided credentials
        const { data: session, error } = await supabase.auth.signInWithPassword({
            // provide the user's email
            email: data.email,

            // provide the user's password
            password: data.password,
        })

        // surface authentication failures to the client
        if (error) {
            throw error
        }

        // confirm the signed-in user
        return { user: session.user }
    })
```

After this call, the response carries the session cookies.

## Sign out

Signing out ends the session and clears the cookies.

```tsx
// create a server function for signing out
export const signOut = createServerFn({
    // declare that this operation changes server-side state
    method: "POST",
})
    // end the session on the server
    .handler(async () => {
        // create a Supabase client for the current server request
        const supabase = createClient()

        // destroy the current session
        const { error } = await supabase.auth.signOut()

        // surface sign-out failures to the client
        if (error) {
            throw error
        }
    })
```

## Verify the user in a loader

A route can confirm the user's identity before returning data.

```tsx
// import the helper for creating a file-based TanStack route
import { createFileRoute } from "@tanstack/react-router"

// import the server-side Supabase client
import { createClient } from "@/lib/supabase/server"

// define a route that requires a signed-in user
export const Route = createFileRoute("/dashboard")({
    // verify the session before rendering the route
    loader: async () => {
        // create a Supabase client for the current server request
        const supabase = createClient()

        // validate the request's token and read the user
        const { data, error } = await supabase.auth.getUser()

        // reject the request when no valid user exists
        if (error || !data.user) {
            throw new Error("Sign in required")
        }

        // expose the verified user to the component
        return { user: data.user }
    },

    // render the page after the loader completes
    component: Dashboard,
})

// define the dashboard component
function Dashboard() {
    // retrieve the verified user from the route loader
    const { user } = Route.useLoaderData()

    // greet the signed-in user
    return <p>Welcome, {user.email}</p>
}
```

`getUser` validates the token rather than trusting stored session data.

## Call the auth operations from a form

The browser triggers the server functions from a form component.

```tsx
// import the React hook for managing form state
import { useState } from "react"

// import the server-side auth operations
import { signIn, signUp } from "@/server/auth"

// define an auth form component
export function AuthForm() {
    // track the email entered by the user
    const [email, setEmail] = useState("")

    // track the password entered by the user
    const [password, setPassword] = useState("")

    // sign in with the current form values
    const handleSignIn = () => signIn({ data: { email, password } })

    // sign up with the current form values
    const handleSignUp = () => signUp({ data: { email, password } })

    // render the sign-in form
    return (
        <form>
            <input
                type="email"
                value={email}
                onChange={(event) => setEmail(event.target.value)}
            />
            <input
                type="password"
                value={password}
                onChange={(event) => setPassword(event.target.value)}
            />
            <button type="button" onClick={handleSignIn}>
                Sign in
            </button>
            <button type="button" onClick={handleSignUp}>
                Sign up
            </button>
        </form>
    )
}
```

## Practice

Type the essential auth loader check:

```tsx
const supabase = createClient()

const { data, error } = await supabase.auth.getUser()

if (error || !data.user) {
    throw new Error("Sign in required")
}

return { user: data.user }
```

Then type the sign-in operation:

```tsx
const { data, error } = await supabase.auth.signInWithPassword({
    email,
    password,
})
```

The central pattern is:

```text
form
  -> server function
  -> server Supabase client
  -> Supabase Auth
  -> session cookies
  -> auth.uid() in later requests
```

Authentication produces the identity that the rest of Supabase trusts.
