# TanStack Start nested routes and layouts - concepts

A route tree becomes especially useful when routes are nested.

A nested route is a child of another route. The parent route can provide
structure that remains visible while the child route changes.

For example:

```text
/dashboard
    |
    +-- /dashboard
    +-- /dashboard/settings
    +-- /dashboard/profile
```

The dashboard can provide the shared navigation while each child provides
its own page content.

This avoids duplicating the same navigation and surrounding UI in every
page.

## Parent and child routes

Suppose the application contains:

```text
src/routes/
    dashboard.tsx
    dashboard/
        index.tsx
        settings.tsx
```

The route tree is conceptually:

```text
__root
└── dashboard
    ├── index
    └── settings
```

The parent route represents the `/dashboard` segment.

Its children represent locations beneath that segment.

The important part is that the parent is not discarded when a child matches.
The router matches the entire hierarchy.

## Nested URL paths

The filesystem expresses the URL hierarchy.

A route such as:

```text
src/routes/dashboard.tsx
```

represents the dashboard segment.

A child route such as:

```text
src/routes/dashboard/settings.tsx
```

represents:

```text
/dashboard/settings
```

The directory therefore establishes the relationship between the parent and
child routes.

This is different from having two unrelated routes whose URLs happen to
share a prefix.

The router knows that `settings` belongs underneath `dashboard`.

## The outlet

A parent route needs a place where its matched child can render.

TanStack Router provides this through `Outlet`.

A simplified parent route component looks like:

```tsx
function Dashboard() {
    return (
        <div>
            <nav>Dashboard navigation</nav>
            <Outlet />
        </div>
    )
}
```

The `Outlet` is a placeholder for the child route.

If `/dashboard` is active, the matching index child can render there.

If `/dashboard/settings` is active, the settings child can render there.

The parent therefore owns the surrounding structure while the child owns the
specific content.

## The rendering hierarchy

Consider:

```text
__root
└── dashboard
    └── settings
```

The rendered application can conceptually become:

```text
Root
└── Dashboard
    ├── navigation
    └── Settings
```

The child does not replace the parent.

Instead, the child is rendered through the parent's outlet.

This is the central mechanism behind nested layouts.

## Layout routes

A route that primarily provides shared structure is commonly thought of as
a layout route.

The layout can contain navigation, headers, sidebars, breadcrumbs, or other
UI that should remain present while the child route changes.

For example:

```tsx
function Dashboard() {
    return (
        <section>
            <h1>Dashboard</h1>
            <nav>...</nav>
            <Outlet />
        </section>
    )
}
```

The child route then only needs to describe its own content.

This produces a useful separation:

```text
parent -> shared structure
child  -> location-specific content
```

## The index child

A parent route can have an index child representing the parent's base URL.

For example:

```text
dashboard.tsx
dashboard/
    index.tsx
    settings.tsx
```

The index child represents:

```text
/dashboard
```

while the settings child represents:

```text
/dashboard/settings
```

The parent provides the shared layout for both.

This means `/dashboard` does not need to be implemented by putting all of
the dashboard page content directly into the parent component.

The parent can instead provide the shell and let its index child provide
the default content.

## Why this matters

Without nested routes, shared UI tends to be duplicated.

Imagine three pages that all need:

```text
sidebar
header
page content
```

Without a parent route, each page could independently render the sidebar
and header.

With a layout route, the structure can be rendered once:

```text
Dashboard
├── Sidebar
├── Header
└── Outlet
```

The child route only supplies what belongs inside the outlet.

This is not merely a visual convenience. The route hierarchy also gives the
application a structural representation of which UI belongs to which part
of the URL.

## Route matching and rendering

When a user visits:

```text
/dashboard/settings
```

the router does not simply select the `settings` file.

It matches the hierarchy:

```text
__root
    -> dashboard
        -> settings
```

Each matched route can participate in rendering.

The resulting component hierarchy is conceptually:

```text
Root
    -> Dashboard
        -> Settings
```

This is why nested routing and component composition naturally fit together.

The URL hierarchy and UI hierarchy can describe the same structure.

## Pathless routes

Not every parent route needs to add a URL segment.

A pathless route can provide shared structure without changing the URL.

Conceptually, an application might have:

```text
root
└── authenticated layout
    ├── dashboard
    └── profile
```

The layout can group routes that share behavior or UI while leaving their
URLs unchanged.

This becomes useful for concerns such as authenticated application shells,
shared navigation, or groups of routes that should use the same layout.

The important distinction is:

```text
path route    -> contributes a URL segment
pathless route -> contributes structure without a URL segment
```

Pathless layouts become particularly useful once an application contains
several groups of related routes.

## Nested routes are not nested components

It is tempting to think of nested routing as simply putting one React
component inside another.

The relationship is deeper.

A nested route is part of the router's route tree.

The router knows which parent and child routes match a URL, and the outlet
connects that route hierarchy to the rendered component hierarchy.

This means the route relationship can also influence data loading, errors,
pending states, navigation, and other route-level behavior.

The layout is therefore not merely a React wrapper.

It is a participant in the application's routing structure.

## The complete model

The relationship can be summarized as:

```text
filesystem
    |
    v
route hierarchy
    |
    v
URL matching
    |
    v
matched parent + child routes
    |
    v
parent component
    |
    v
Outlet
    |
    v
child component
```

Once this model is understood, deeper routing features become extensions of
the same idea.

A route represents a location.

A parent route can provide structure for its descendants.

An outlet provides the insertion point for the matched child.

The resulting component hierarchy follows the route hierarchy.

## Next step

Now type the code version of this lesson.
