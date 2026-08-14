# Panels and layout - concepts

A shader IDE is several tools in one window. The editor holds the source, the
preview shows the output, and the diagnostics list explains failures. This
lesson arranges those panels with ImGui docking, and handles the coupling
between the layout and the preview target: when a panel resizes, the offscreen
target and the shader's resolution must follow.

## The window is a workspace

ImGui windows are drawn inside the application window. ImGui docking lets
the user drag panels into docked regions or float them as independent
windows. The IDE provides a default layout and lets the user rearrange it.

```text
+---------------------------------+---------------------+
| Editor                          | Preview             |
| (source text, error highlight)  | (shader output)     |
|                                 |                     |
|                                 |                     |
+---------------------------------+---------------------+
| Diagnostics                     |                     |
| (compile messages)              |                     |
+---------------------------------+---------------------+
```

The default layout puts the editor on the left, the preview on the right,
and diagnostics at the bottom. The exact arrangement is a starting point, not
a fixed contract.

## The dockspace

ImGui renders a dockspace into the full window, then places each panel into a
dock node. The dockspace is created once when the window is first drawn, and
the panels are opened every frame inside it.

```cpp
ImGui::DockSpace(id);
```

Docking requires a config flag and a version check. Once enabled, panels can
be dragged between dock nodes, tabbed together, or floated. The layout
survives because ImGui stores it in its internal state.

## A shared main menu

A menu bar gives the IDE its global commands: opening a file, saving, toggling
the preview resolution, and so on. The menu bar lives above the dockspace and
is shared by the whole application rather than attached to one panel.

```text
File   Shader   View
```

The menu bar is where the file operations from a later lesson will attach.
For this lesson it proves the structure and hosts the resolution control.

## Panel sizes are user-controlled

Dock splitters let the user resize panels by dragging their edges. The IDE
should not override these sizes, but it must react to them. The preview panel
reports its current size through ImGui's window state each frame.

```cpp
ImVec2 panelSize = ImGui::GetContentRegionAvail();
```

The preview renderer reads this size and, when it differs from the current
target size, recreates the target.

## The preview target follows the panel

The offscreen target must match the preview panel size. The coupling is
one-way: the layout is the source of truth, and the renderer follows.

```text
panel resize
    |
    v
renderer reads panel size
    |
    v
size changed?
    |
    +--> recreate target image
    |
    v
next frame renders at the new resolution
```

Recreating the target is the same pattern as swapchain recreation: destroy
the old image, create a new one at the new size, and rebuild dependent views.
The pipeline does not need to change because the render pass format is the
same.

## Aspect ratio

A preview panel rarely has the exact aspect ratio of the shader's intended
output. The IDE draws the rendered image fitted inside the panel while
preserving the target's aspect ratio.

```text
available panel size
    |
    v
compute fitted rectangle (preserve aspect)
    |
    v
ImGui::SetCursorPos(fitted origin)
    |
    v
ImGui::Image(fitted size)
```

The area outside the fitted rectangle is left empty, giving the preview a
letterboxed look. This keeps shaders that assume a specific aspect ratio
looking correct.

## The resolution control

The IDE exposes a preview resolution setting. It offers a small list of
common sizes and an option to match the panel exactly.

```text
preview resolution:  [Match Panel] [512x512] [1024x1024]
```

When the resolution is fixed, the target keeps that size and the image is
scaled to fit the panel. When it matches the panel, the target resizes with
the panel. Both modes are useful, and both flow through the same target
creation path.

## Resize timing

Panel resizing happens during the ImGui frame, after the panels report their
sizes. The target recreation must therefore happen before the preview is
rendered for the same frame, but it must not fight the layout system.

The safe order inside a frame is:

```text
draw panels -> panels report sizes
    |
    v
resize preview target if needed
    |
    v
render preview
```

The target size used for rendering is the size the panel reported in the same
frame. This guarantees the preview always fills its panel without flicker.

## Docking state persistence

ImGui can save its layout to a config file so the user's arrangement survives
restarts. The IDE enables this with a config flag and an ini file name. This
is a one-line feature that makes the IDE feel polished.

## What this lesson establishes

The interface is now a real workspace: a dockspace, a menu bar, editable
panels, and a preview that resizes with its panel. The renderer follows the
layout, and the layout follows the user. The remaining lessons add files,
the compute path, and the final integration.

## Next step

Now type the code version of this lesson.
