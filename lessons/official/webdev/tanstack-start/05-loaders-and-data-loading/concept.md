# TanStack Start loaders and data loading - concepts

A route usually needs data before its UI can become useful.

A products route needs products. A user route needs the user. A dashboard
needs the information that makes up its dashboard.

TanStack Router places this requirement directly on the route through a
loader.

The important relationship is:

```text
route -> loader -> data -> component
```

The route describes where the application is.

The loader describes the data required by that route.

The component consumes the resulting data.

## The route loader

A route can define a `loader` property.

For example:

```tsx
export const Route = createFileRoute("/users")({
    loader: () => {
        return {
            users: ["Ada", "Grace"],
        }
    },
})
```

The loader is associated with the route rather than being an arbitrary
function called somewhere inside the component.

That distinction matters because the router knows that the route has a data
requirement.

The router can therefore coordinate data loading with navigation and
rendering.

## Loader data

The value returned by a loader becomes route data.

A component can retrieve that data through the route:

```tsx
function Users() {
    const { users } = Route.useLoaderData()

    return <p>{users.length} users</p>
}
```

The loader and component therefore have a direct relationship.

The loader produces the data.

The component consumes the data.

This keeps data acquisition separate from the rendering logic that displays
the result.

## Why loaders belong to routes

A component can technically perform data loading itself, but doing so loses
important information about the relationship between the URL and its data.

Consider a route:

```text
/users/42
```

The route knows that `42` identifies the resource being displayed.

A route loader can therefore use the route parameters to load the resource
before the route renders:

```text
/users/42
    |
    v
matched route
    |
    v
loader
    |
    v
user 42
    |
    v
component
```

The data requirement is attached to the same route that defines the URL.

## Route parameters in loaders

Loaders can access information from the matched route.

A dynamic route might look like:

```tsx
export const Route = createFileRoute("/users/$userId")({
    loader: ({ params }) => {
        return getUser(params.userId)
    },
})
```

The `params` value contains the dynamic segments matched from the URL.

For:

```text
/users/42
```

the relevant parameter is:

```text
userId = "42"
```

This lets the loader derive its data requirement directly from the route
being loaded.

## Loader execution

A loader is not simply a component lifecycle callback.

The router invokes loaders as part of its route-loading process.

This means loaders participate in both initial rendering and navigation.

Conceptually:

```text
initial request
    |
    v
match routes
    |
    v
load route data
    |
    v
render

later navigation
    |
    v
match new routes
    |
    v
load required data
    |
    v
render new state
```

The exact execution environment depends on how the application is being
rendered and navigated, which becomes important when server rendering is
introduced.

For now, the important idea is that the router owns the loading lifecycle.

## Dependencies

Sometimes a loader depends on values that are not direct route parameters.

For example, one route may need data associated with another route, or its
result may depend on search parameters.

TanStack Router provides mechanisms for expressing those dependencies so the
router can understand when data should be considered stale or need to be
loaded again.

A simple search dependency might look conceptually like:

```tsx
loaderDeps: ({ search }) => ({
    page: search.page,
})
```

The loader can then use the dependency:

```tsx
loader: ({ deps }) => {
    return getProducts(deps.page)
}
```

The important idea is that the loader declares what changing state affects
its result.

This gives the router information it can use when deciding whether the
loader needs to run again.

## Preloading

A router can often begin loading a destination before the user actually
commits to navigation.

For example, hovering over a link can provide an opportunity to prepare the
destination route.

The important conceptual distinction is:

```text
navigation -> load because the user navigated
preloading -> load because navigation may happen
```

Preloading can make navigation feel faster because some route work may
already be complete when the user activates the link.

The router's knowledge of route loaders makes this possible without requiring
every component to invent its own prefetching system.

## Pending states

Data loading takes time.

A route therefore needs a way to represent the period between starting a
navigation and having the required route data available.

TanStack Router supports pending UI for this purpose.

Conceptually:

```text
navigation begins
      |
      v
pending UI
      |
      v
loader completes
      |
      v
route renders
```

The pending state belongs to the route transition rather than being merely
an arbitrary boolean inside the component.

This makes it possible to provide consistent loading behavior as users move
through the application.

## Errors

Loaders can also fail.

A database request might fail. A resource might not exist. A remote service
might return an error.

The route hierarchy provides a place for that failure to be represented.

Conceptually:

```text
route
  |
  +-- loader
       |
       +-- success -> component
       |
       +-- failure -> error UI
```

Route-level error handling prevents every component from having to implement
the same loading and failure orchestration independently.

A parent route can also provide error behavior for descendants, allowing
applications to establish consistent boundaries.

## Loader data and route hierarchy

Nested routes can each have their own loaders.

For example:

```text
dashboard
    |
    +-- dashboard loader
    |
    +-- settings
            |
            +-- settings loader
```

When the settings route is active, both the parent and child can contribute
data to the matched route hierarchy.

The application can therefore load data at the same level where the
corresponding UI structure is defined.

This is one of the major advantages of route-based data loading.

## Avoiding component waterfalls

Consider a component that loads its own data after rendering:

```text
render component
    |
    v
start request
    |
    v
wait
    |
    v
render data
```

If several nested components independently begin requests after rendering,
the application can accidentally create a sequence of dependent loading
steps.

Route loaders allow the router to know about route data requirements before
the route is rendered.

Conceptually:

```text
match route tree
    |
    v
discover route data requirements
    |
    v
load data
    |
    v
render route tree
```

This does not mean every loader must be serialized. It means the router has
visibility into the route's data requirements instead of discovering them
only after arbitrary components have rendered.

## Loader data is not global state

Loader data belongs to a route.

This is an important distinction from a global state store.

A user route can own the data required by that user route.

A products route can own its products.

A dashboard route can own its dashboard data.

The router manages the relationship between those route-owned data values and
the route lifecycle.

If unrelated parts of the application need the same persistent state,
another state-management strategy may be more appropriate.

## The complete loading model

The concepts introduced so far fit together as:

```text
URL
 |
 v
route matching
 |
 v
loader dependencies
 |
 v
route loaders
 |
 +---- success ----> route data
 |                      |
 |                      v
 |                   component
 |
 +---- pending ----> pending UI
 |
 +---- failure ----> error UI
```

The loader is therefore not merely a convenient place to call `fetch`.

It is a participant in the router's lifecycle.

The router knows which route needs the data, what route state the loader
depends on, when navigation requires that data, and what should happen while
the data is unavailable.

## Next step

Now type the code version of this lesson.
