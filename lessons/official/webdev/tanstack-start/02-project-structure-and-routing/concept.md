# TanStack Start project structure and routing - concepts

A TanStack Start application is organized around a small number of
important directories and generated files.

The most important directory for application navigation is:

```text
src/routes/
```

Files inside this directory describe the application's file-based route tree.

The framework turns those route files into a route hierarchy that the router
can use when matching URLs.

## The project structure

A typical Start project contains application source code alongside the
configuration and package files used to build and run it.

The important application structure looks roughly like this:

```text
src/
    routes/
        __root.tsx
        index.tsx
        about.tsx
```

The exact project contains additional files, but these three route files
illustrate the core idea.

The `routes` directory is special because the routing plugin scans it and
turns its files into the application's route tree.

This means that adding or removing a route is normally done by changing the
contents of this directory rather than manually registering every route in a
central router configuration.

## The root route

The root route is represented by:

```text
src/routes/__root.tsx
```

The double underscore is significant because this file represents the root
of the route hierarchy rather than a normal URL segment.

The root route can provide application-wide structure.

For example:

```tsx
export const Route = createRootRoute({
    component: RootComponent,
})
```

The root route sits above the other routes.

If the application contains `/` and `/about`, both routes are descendants
of this root route.

That makes the root route a natural place for structure shared across the
whole application.

## The index route

The root URL is represented by:

```text
src/routes/index.tsx
```

This route corresponds to:

```text
/
```

Its route path is therefore the empty path beneath the root route.

A simple route can look like:

```tsx
export const Route = createFileRoute("/")({
    component: Home,
})
```

The important distinction is that `__root.tsx` describes the root route
itself, while `index.tsx` describes the application's content at the root
URL.

The two files can therefore exist together without representing the same
thing.

## A normal route

A file such as:

```text
src/routes/about.tsx
```

represents:

```text
/about
```

The filename becomes part of the URL structure.

Its route module can define the component associated with that URL:

```tsx
export const Route = createFileRoute("/about")({
    component: About,
})
```

The route's URL and its source file are therefore connected by convention.

This is the central idea behind file-based routing.

## The route tree

The files do not remain independent modules.

The routing system interprets them as a hierarchy.

For example:

```text
__root
├── index
└── about
```

represents:

```text
/
└── /about
```

The root route is the parent of both application routes.

This hierarchy becomes much more important once nested routes are introduced,
because a URL can then correspond to several matched route modules at once.

For example, a deeper structure might look like:

```text
__root
└── dashboard
    └── settings
```

which represents:

```text
/
/dashboard
/dashboard/settings
```

The router can match the complete chain rather than treating the final URL
as one isolated page.

## Route modules

A route file is called a route module because it can contain more than the
component rendered at that URL.

A route can eventually define things such as:

```tsx
export const Route = createFileRoute("/about")({
    loader: loadAbout,
    component: About,
})
```

The component controls the UI, while the loader describes data required by
the route.

The same route module can also participate in error handling, pending
behavior, search-parameter validation, and other routing concerns.

This is why file-based routing is more than a convenience for avoiding a
large switch statement. The file becomes the natural home for behavior
belonging to that part of the URL tree.

## The generated route tree

The application needs a route tree that the router can use at runtime.

With file-based routing, that tree is generated from the route files.

Conceptually:

```text
route files
    |
    v
generated route tree
    |
    v
TanStack Router
```

The generated file connects the filesystem representation to the router's
runtime representation.

You generally should not think of the generated route tree as the primary
place where routes are authored.

Instead, the route files are the source of truth for the file-based routing
workflow, while the generated tree is the result consumed by the application.

## URL matching

When the browser requests a URL, the router needs to determine which route
matches it.

Suppose the application contains:

```text
src/routes/__root.tsx
src/routes/index.tsx
src/routes/about.tsx
```

A request for:

```text
/
```

matches the root route and the index route.

A request for:

```text
/about
```

matches the root route and the about route.

The router therefore builds a matched route chain:

```text
/about
    |
    +-- __root
    |
    +-- about
```

This matched hierarchy is what allows parent and child routes to cooperate
when rendering the application.

## Why file names matter

File-based routing is convention-driven.

The framework interprets filenames according to routing conventions, so the
name of a file is not arbitrary.

For example:

```text
index.tsx
```

means the index route of its directory, while:

```text
about.tsx
```

creates an `about` path segment.

Later, special filename conventions will represent dynamic parameters,
nested routes, and layouts.

Learning these conventions is therefore equivalent to learning a significant
part of how the route tree is authored.

## The application entry point

The route files describe the application's navigable structure, but they do
not by themselves represent the entire application startup process.

The project also contains the application and build configuration that
connects React, TanStack Router, TanStack Start, and the development server.

For everyday feature work, however, the `src/routes` directory is the part
you will touch most frequently when creating pages and route behavior.

The important distinction is:

```text
project configuration -> controls the application infrastructure
route files            -> define the application's navigation structure
```

Keeping that distinction clear prevents the route directory from being
mistaken for the entire Start application.

## A route is a module in a tree

The central idea of this lesson can be expressed as:

```text
file
  |
  v
route module
  |
  v
route tree
  |
  v
URL matching
  |
  v
rendered route hierarchy
```

The filesystem gives the route module its location.

The route module contributes a node to the route tree.

The router uses that tree to match URLs.

The matched route hierarchy then determines which route modules participate
in rendering the requested location.

Once this relationship is understood, nested routes and layouts become a
natural extension rather than a collection of unrelated filename rules.

## Next step

Now type the code version of this lesson.
