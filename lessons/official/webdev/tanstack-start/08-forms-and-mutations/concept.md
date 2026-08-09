# TanStack Start forms and mutations - concepts

A route loader is primarily concerned with obtaining data required to render
a route.

A mutation has a different purpose.

A mutation changes something.

Examples include:

```text
create a user
update a profile
delete a record
submit an order
change a setting
```

The fundamental distinction is:

```text
loader   -> read data needed by a route
mutation -> change server-side state
```

TanStack Start's server functions provide the server boundary through which
these mutations can be performed.

The resulting application flow is:

```text
browser form
    |
    v
validate input
    |
    v
server function
    |
    v
server-side mutation
    |
    v
result
    |
    v
update UI
```

## Forms are browser interactions

A form begins in the browser.

The user enters information:

```text
name
email
password
...
```

The browser therefore owns the immediate interaction with the form.

The server owns the operation that ultimately changes persistent state.

This creates two separate responsibilities:

```text
browser -> collect and submit input
server  -> validate and perform the mutation
```

The browser should not be trusted merely because it produced the form.

## Client validation versus server validation

Client-side validation improves the user experience.

For example, the browser can immediately tell the user that a required field
is empty.

However, client validation is not a security boundary.

A user can bypass browser code entirely and send a request manually.

Therefore:

```text
client validation -> user experience
server validation -> trust boundary
```

The server must validate the input before using it.

A production application commonly performs both.

## A mutation server function

A mutation can be represented by a POST server function.

```tsx
const createUser = createServerFn({
    method: "POST",
})
```

The function receives input and performs the server-side operation.

Conceptually:

```text
form
 |
 v
createUser({ data })
 |
 v
server
 |
 v
database
```

The database operation belongs inside the server-side boundary.

The browser should not receive the database credentials or direct database
connection.

## Form submission

A React form can collect the user's input.

```tsx
<form onSubmit={handleSubmit}>
    <input name="name" />
    <button type="submit">Create</button>
</form>
```

The form's job is to capture the interaction.

The submit handler can then transform the form values into the input expected
by the server function.

This creates a clear boundary between UI representation and server
operation.

## The `FormData` object

Browser forms naturally produce `FormData`.

For example:

```tsx
const formData = new FormData(event.currentTarget)
const name = formData.get("name")
```

The browser represents the submitted fields as key-value entries.

Before sending those values to a server function, the application should
convert them into the shape expected by the server operation.

For example:

```tsx
createUser({
    data: {
        name: String(name),
    },
})
```

This explicit conversion makes the application's data boundary visible.

## Mutation results

A server mutation can return a result.

For example:

```tsx
return {
    id: user.id,
    name: user.name,
}
```

The browser can use the result to update the interface.

A mutation might return:

```text
created record
validation result
operation status
redirect information
```

The returned value should contain what the client actually needs.

## Pending mutation state

A mutation is asynchronous.

The user needs feedback while the server operation is running.

Conceptually:

```text
submit
  |
  v
pending
  |
  +-- disable submit
  +-- show progress
  |
  v
result
```

Without pending state, a user can click a submit button multiple times while
the first request is still executing.

A good mutation interface therefore makes its current state visible.

## Mutation errors

Mutations can fail.

For example:

```text
invalid input
unauthorized request
database failure
conflicting record
network failure
```

These failures should be handled as part of the mutation flow.

The UI can distinguish between:

```text
success -> show result
failure -> show error
```

The server should also avoid exposing internal implementation details in
errors returned to users.

A database stack trace is useful to a developer but is not an appropriate
user-facing response.

## Authentication and authorization

A mutation is often protected.

For example:

```text
update profile
      |
      v
authenticate user
      |
      v
authorize operation
      |
      v
perform update
```

The server function must enforce these rules.

A disabled button in the browser is not authorization.

A hidden UI element is not authorization.

The server must determine whether the caller is allowed to perform the
operation.

## Mutation versus loader

It is useful to keep the responsibilities separate.

A loader answers:

```text
"What data is needed to display this route?"
```

A mutation answers:

```text
"What change does the user want the server to perform?"
```

A common sequence is:

```text
load route
    |
    v
display data
    |
    v
user submits form
    |
    v
mutation
    |
    v
persistent state changes
    |
    v
reload or update displayed data
```

The mutation changes the underlying state.

The application then needs to make the UI reflect that new state.

## Revalidation

Suppose a route displays:

```text
Users: 10
```

and the user creates another user.

The server now contains:

```text
Users: 11
```

but the browser may still display:

```text
Users: 10
```

The application therefore needs some way to synchronize route data with the
new server state.

One approach is to revalidate the affected route data.

Conceptually:

```text
mutation
   |
   v
server state changes
   |
   v
revalidate loader
   |
   v
fresh route data
   |
   v
updated UI
```

This is one reason loaders and mutations should be thought of as two parts
of the same data lifecycle.

## Redirects after mutations

Some mutations naturally lead to another route.

For example:

```text
create account
    |
    v
account created
    |
    v
/dashboard
```

The application can perform navigation after the mutation succeeds.

The important point is that the mutation and navigation are separate
concerns:

```text
mutation -> change server state
navigation -> change application location
```

A successful mutation may cause navigation, but it does not inherently mean
that every mutation should navigate.

## Optimistic interfaces

Some applications update the UI before the server confirms the mutation.

For example:

```text
user clicks Like
    |
    v
UI immediately shows Like
    |
    v
server confirms
```

This is an optimistic update.

It can make interfaces feel faster, but it introduces additional complexity.

The application must know what to do if the server rejects the mutation:

```text
optimistic update
       |
       +-- success -> keep state
       |
       +-- failure -> revert or reconcile
```

Optimistic updates are therefore an advanced optimization, not something that
should be added to every form automatically.

## The complete mutation model

A robust mutation flow looks like:

```text
user input
    |
    v
browser validation
    |
    v
form submission
    |
    v
server function
    |
    v
server validation
    |
    v
authentication
    |
    v
authorization
    |
    v
database / external service
    |
    v
result
    |
    +---- failure ---> error UI
    |
    +---- success ---> revalidate / navigate / update UI
```

The important lesson is that the browser and server have different
responsibilities.

The browser provides the interaction.

The server owns the trusted mutation.

The UI then reconciles itself with the result.

## Next step

Now type the code version of this lesson.
