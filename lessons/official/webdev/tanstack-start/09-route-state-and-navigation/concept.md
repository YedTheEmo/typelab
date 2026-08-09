# TanStack Start route state and navigation - concepts

A web application's URL is not merely an address.

It can contain application state.

For example:

```text
/products/42?tab=reviews
```

contains several distinct pieces of information:

```text
/products
    |
    +-- path
    |
    +-- 42 -> dynamic path parameter
    |
    +-- tab=reviews -> search parameter
```

TanStack Router treats these pieces as structured route state.

This gives the application a type-aware relationship between:

```text
URL
 |
 v
route
 |
 +-- path params
 |
 +-- search params
 |
 +-- route context
 |
 v
components / loaders
```

## Dynamic path parameters

A dynamic segment is represented by `$` in a file-based route.

For example:

```text
src/routes/users/$userId.tsx
```

represents URLs such as:

```text
/users/42
/users/123
/users/abc
```

The `$userId` portion becomes a route parameter.

The route can access it through `params`:

```tsx
export const Route = createFileRoute("/users/$userId")({
    loader: ({ params }) => {
        return getUser(params.userId)
    },
})
```

The parameter is part of the URL's path identity.

Changing:

```text
/users/42
```

to:

```text
/users/43
```

means the application is visiting a different parameterized route state.

## Path parameters versus search parameters

These two kinds of URL state serve different purposes.

A path parameter usually identifies the resource represented by the route:

```text
/users/42
        ^^
        resource identity
```

A search parameter usually modifies how that resource or collection is
displayed:

```text
/users?sort=name&page=2
       ^^^^^^^^^^^^^^^
       view/query state
```

This is not an absolute rule, but it is a useful architectural distinction.

A product ID is naturally a path parameter.

A table's page, sorting, filter, or search term is naturally represented as
search state.

## Search parameters

Search parameters are the portion after `?`.

For example:

```text
/products?page=2&sort=price
```

contains:

```text
page = 2
sort = price
```

TanStack Router can validate and type these values.

A route can define:

```tsx
validateSearch: (search) => ({
    page: Number(search.page ?? 1),
})
```

The route then has a known representation of its search state.

The important concept is that URL parsing and application state conversion
can happen at the route boundary.

## Why validate search parameters?

URL values originate outside the application's trusted internal state.

Even this:

```text
?page=2
```

is ultimately text from a URL.

Without validation, the application would repeatedly need to interpret those
values.

Validation provides a consistent transformation:

```text
URL
 |
 v
raw search values
 |
 v
validated route search
 |
 v
application
```

For example, the application can work with:

```ts
page: number
```

instead of repeatedly handling:

```ts
page: string | undefined
```

## Search state and loaders

Search parameters frequently affect route data.

A products page might use:

```text
/products?page=3
```

to determine which records should be loaded.

The route can declare that relationship:

```tsx
loaderDeps: ({ search }) => ({
    page: search.page,
})
```

The loader then receives the dependency:

```tsx
loader: ({ deps }) => {
    return getProducts(deps.page)
}
```

The resulting architecture is:

```text
URL search
    |
    v
validated search
    |
    v
loader dependency
    |
    v
loader
    |
    v
route data
```

This lets the router understand that changing the page changes the data
requirement.

## Links

Navigation should normally use the router's `Link` component instead of
manually manipulating browser history.

Conceptually:

```tsx
<Link to="/users">Users</Link>
```

The router understands that this is an application route.

It can therefore perform client-side navigation and coordinate route loading.

A normal anchor can still be appropriate when a genuine document navigation
is intended, but application-internal navigation generally belongs to the
router.

## Parameterized links

A dynamic route requires its parameter.

For example:

```tsx
<Link
    to="/users/$userId"
    params={{ userId: "42" }}
>
    Ada
</Link>
```

The route parameter is supplied separately from the URL string.

This is important because it lets the router understand the route structure
instead of treating the destination as an arbitrary string.

In a type-safe application, this relationship also allows incorrect route
parameters to be detected during development.

## Search parameters in links

Search state can also be supplied through navigation.

For example:

```tsx
<Link
    to="/products"
    search={{ page: 2 }}
>
    Page 2
</Link>
```

The destination is conceptually:

```text
/products?page=2
```

The router handles constructing the URL representation from the structured
search object.

This avoids manually concatenating query strings throughout the
application.

## Programmatic navigation

Sometimes navigation occurs as a consequence of code rather than a link.

For example:

```text
form submission succeeds
        |
        v
navigate to dashboard
```

The router provides navigation APIs for this.

The conceptual operation is:

```tsx
navigate({
    to: "/dashboard",
})
```

Programmatic navigation should represent an actual application transition.

It should not be used as a substitute for changing local UI state.

## Navigation is more than changing the URL

A navigation can cause several things to happen:

```text
new location
    |
    +-- route matching
    |
    +-- loader execution
    |
    +-- pending state
    |
    +-- error handling
    |
    +-- rendering
```

This is why TanStack Router is more than a URL switcher.

The router coordinates the application lifecycle associated with changing
location.

## Route context

A route hierarchy may also need shared values that are not URL state.

Examples include:

```text
authenticated user
application configuration
service instances
logging utilities
```

TanStack Router provides route context for values that should be available
through the route hierarchy.

Conceptually:

```text
root route context
        |
        v
child route
        |
        v
grandchild route
```

A child route can consume context established by an ancestor.

This is different from search parameters.

Search parameters describe URL state.

Route context describes values supplied by the application's route
hierarchy.

## Context and loaders

Route context can be particularly useful to loaders.

For example, a root route might provide an authentication-related object,
while a child route's loader consumes it.

The conceptual relationship is:

```text
root
 |
 +-- establish context
 |
 v
child loader
 |
 +-- consume context
 |
 v
route data
```

This allows route loaders to depend on application infrastructure without
forcing every loader to reconstruct that infrastructure independently.

## Search parameters versus context

These concepts should not be confused.

Search state:

```text
/products?page=2
```

is part of the URL and can be shared, bookmarked, or revisited.

Context:

```text
auth/session/service
```

is supplied by the application.

A useful distinction is:

```text
URL state   -> search / path
application -> context
```

The correct choice depends on whether the value should be represented by
the location itself.

## Navigation and data loading

Navigation and loading are closely connected.

Suppose the user starts at:

```text
/products?page=1
```

and navigates to:

```text
/products?page=2
```

The URL changed, but the route path did not.

The router can still determine that the route's loader dependency changed:

```text
page 1
  |
  v
loader dependency
  |
  v
products page 1

page 2
  |
  v
loader dependency
  |
  v
products page 2
```

This is why declaring loader dependencies matters.

The router can reason about what data is associated with the navigation.

## The complete route-state model

The pieces now fit together:

```text
URL
 |
 +-- path
 |    |
 |    +-- dynamic parameters
 |
 +-- search
      |
      +-- validated search state
      |
      +-- loader dependencies
      |
      +-- route data

route hierarchy
 |
 +-- context
 |
 +-- loaders
 |
 +-- components

navigation
 |
 +-- Link
 +-- programmatic navigation
 |
 v
new route state
```

The router therefore acts as the system that coordinates location, route
state, navigation, and route data.

## Next step

Now type the code version of this lesson.
