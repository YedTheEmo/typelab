# TanStack Start nested routes and layouts - typing

This lesson types a nested dashboard route: create a parent route, render its
shared structure through an outlet, and place child routes inside that layout.

## Create the dashboard route

The dashboard route provides the shared structure for its children.

```tsx
// import the helpers used to create a route and render child routes
import { createFileRoute, Outlet } from "@tanstack/react-router"

// create the route associated with the dashboard URL
export const Route = createFileRoute("/dashboard")({
    // provide the dashboard layout component
    component: Dashboard,
})

// define the shared dashboard structure
function Dashboard() {
    // render the dashboard shell around the active child route
    return (
        <div>
            <h1>Dashboard</h1>
            <nav>Dashboard navigation</nav>
            <Outlet />
        </div>
    )
}
```

## Create the dashboard index

The index child represents the base `/dashboard` URL.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the index route beneath the dashboard route
export const Route = createFileRoute("/dashboard/")({
    // provide the component rendered at the dashboard URL
    component: DashboardHome,
})

// define the dashboard's default content
function DashboardHome() {
    // render the default dashboard page
    return <p>Dashboard home</p>
}
```

The parent provides the shell, while the index route provides the content
inside the parent's outlet.

```text
/dashboard
    |
    +-- Dashboard layout
            |
            +-- DashboardHome
```

## Create a child route

A second child can represent `/dashboard/settings`.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the settings route beneath the dashboard route
export const Route = createFileRoute("/dashboard/settings")({
    // provide the component rendered for the settings route
    component: Settings,
})

// define the settings page
function Settings() {
    // render the settings content inside the dashboard layout
    return <p>Settings</p>
}
```

When `/dashboard/settings` is active, the route hierarchy is:

```text
__root
    |
    +-- dashboard
            |
            +-- settings
```

The dashboard component remains active because it is the parent route.

## Render the child through the outlet

The important connection is the `Outlet` in the parent.

```tsx
// import the helpers used to create a route and render child routes
import { createFileRoute, Outlet } from "@tanstack/react-router"

// create the route associated with the dashboard URL
export const Route = createFileRoute("/dashboard")({
    // provide the dashboard layout component
    component: Dashboard,
})

// define the shared dashboard structure
function Dashboard() {
    // render the dashboard shell around the active child route
    return (
        <div>
            <h1>Dashboard</h1>
            <nav>Dashboard navigation</nav>
            <Outlet />
        </div>
    )
}
```

The outlet is where the router inserts the component belonging to the active
child route.

## Add another level

Nested routes can continue deeper when the application needs them.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create a route beneath the dashboard settings route
export const Route = createFileRoute("/dashboard/settings/account")({
    // provide the account settings component
    component: AccountSettings,
})

// define the account settings page
function AccountSettings() {
    // render the account settings content
    return <p>Account settings</p>
}
```

The hierarchy now becomes:

```text
__root
    |
    +-- dashboard
            |
            +-- settings
                    |
                    +-- account
```

Each parent can provide an outlet for its own child.

## Now type it again

Type the parent layout again.

```tsx
// import the helpers used to create a route and render child routes
import { createFileRoute, Outlet } from "@tanstack/react-router"

// create the route associated with the dashboard URL
export const Route = createFileRoute("/dashboard")({
    // provide the dashboard layout component
    component: Dashboard,
})

// define the shared dashboard structure
function Dashboard() {
    // render the dashboard shell around the active child route
    return (
        <div>
            <h1>Dashboard</h1>
            <nav>Dashboard navigation</nav>
            <Outlet />
        </div>
    )
}
```

Type the index child.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the index route beneath the dashboard route
export const Route = createFileRoute("/dashboard/")({
    // provide the component rendered at the dashboard URL
    component: DashboardHome,
})

// define the dashboard's default content
function DashboardHome() {
    // render the default dashboard page
    return <p>Dashboard home</p>
}
```

Type the settings child.

```tsx
// import the helper used to create a file-based route
import { createFileRoute } from "@tanstack/react-router"

// create the settings route beneath the dashboard route
export const Route = createFileRoute("/dashboard/settings")({
    // provide the component rendered for the settings route
    component: Settings,
})

// define the settings page
function Settings() {
    // render the settings content inside the dashboard layout
    return <p>Settings</p>
}
```

The three modules now express one hierarchy.

```text
/dashboard
    |
    +-- shared dashboard layout
            |
            +-- /dashboard
            |
            +-- /dashboard/settings
```

## Wrap up

The flow: parent route -> Outlet -> matched child route
