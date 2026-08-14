# Capstone full IDE - concepts

The final lesson assembles every subsystem into one application. The editor,
the compiler, the renderer, the reload loop, the diagnostics, the layout,
and the file layer have each been built separately. This lesson connects
them and reviews the architecture as a whole.

The result is a complete shader IDE: a window with panels, an editor that
tracks source, a compiler that produces SPIR-V, and a preview that updates
itself as the user types.

## The whole application

The complete application is one program with clearly separated parts.

```text
+-------------------------------------------------------------+
| Shell: window, main loop, swapchain, ImGui                  |
+-------------------------------------------------------------+
|  Editor      |  Compiler     |  Renderer    |  Files        |
|  text buffer |  Slang        |  Vulkan      |  project I/O  |
|  cursor      |  -> SPIR-V    |  pipeline    |  tabs         |
|  highlight   |  diagnostics  |  preview     |  autosave     |
+--------------+---------------+--------------+---------------+
```

Each box has one responsibility, and the dependencies flow in one direction.
The shell drives the frame. The editor produces source. The compiler turns
source into SPIR-V and diagnostics. The renderer turns SPIR-V into an image.
The file layer moves source between the editor and the disk.

## The complete data flow

A single frame moves data along one path.

```text
frame begins
    |
    v
editor -> buffer text
    |
    v
changed? and stable?
    |
    v
compiler -> SPIR-V + diagnostics
    |
    v
renderer -> pipeline + preview image
    |
    v
ImGui -> panels drawn
    |
    v
swapchain presents
```

Every subsystem reads the state produced by the subsystem before it. Nothing
skips a step, and nothing reaches behind another subsystem to change its
state directly.

## Resource lifetimes

The architecture is defined by which objects live at which level.

The shell owns the device-level objects for the whole program:

```text
instance
surface
device
queues
swapchain
```

The renderer owns the program-level objects that change with each shader:

```text
shader modules
pipelines
pipeline layouts
descriptor sets
offscreen target
storage image
```

The frame owns the per-frame state:

```text
command buffers
uniform buffer contents
frame synchronization
```

A recompile touches only the renderer's objects. The shell and the frame keep
working, which is exactly what hot reload requires.

## The reload decision

Hot reload is the behavior that ties everything together. It is a decision
made from four pieces of state:

```text
dirty      -> the editor changed the source
stable     -> the user paused typing
entry point -> which pipeline to rebuild
previous   -> the last successful pipeline to keep
```

When the source is stable, the IDE compiles, rebuilds the correct pipeline on
success, and keeps the previous pipeline on failure. The user never sees a
blank preview and never presses a button.

## The two execution paths

The IDE supports two kinds of shaders, selected by the entry point.

The fragment path renders the compiled shader as the color attachment of the
offscreen target.

```text
fragment SPIR-V -> graphics pipeline -> target image -> preview
```

The compute path dispatches the compiled kernel into a storage image.

```text
compute SPIR-V -> compute pipeline -> storage image -> preview
```

Both paths share the uniform buffer, the preview panel, the reload loop, and
the keep-last-good rule. The difference is contained inside the renderer.

## The uniform contract

The host and the shader agree on one layout:

```text
FrameUniforms
    time
    resolution
    mouse
```

The C++ struct and the Slang parameter block are the only place where the two
languages must match exactly. Everything else in the application is C++ or
Slang alone.

## The feedback loop

The value of the IDE is measured by the length of its feedback loop.

```text
type -> pause -> recompile -> preview
```

The loop has four properties that make it feel good:

```text
fast    -> compile runs in milliseconds
safe    -> failed edits never blank the preview
visual  -> the result appears in the preview panel
explicit -> errors are shown at the exact line
```

Any regression in these properties is a regression in the tool.

## What the developer must know

After this course, a developer should be able to reason about:

- how Slang source becomes SPIR-V through the session hierarchy;
- how SPIR-V becomes a pipeline and an image through Vulkan;
- how the editor maps offsets to lines and columns;
- how diagnostics travel from the compiler back to the editor;
- how hot reload keeps the last good shader alive;
- how the layout drives the preview target size;
- how the uniform buffer is the contract with the shader;
- how fragment and compute shaders reach the same preview;
- how the file layer persists plain text shaders.

The objective is not memorizing API calls. The objective is understanding the
loop: source enters, code and diagnostics come out, and the preview reflects
the current state of the shader.

## The final review

Before calling the application complete, review the whole path:

```text
window shell
    -> ImGui panels
    -> editor text buffer
    -> Slang compile
    -> fragment or compute pipeline
    -> uniforms
    -> hot reload
    -> diagnostics highlight
    -> layout and resize
    -> files and tabs
```

A defect at any link breaks the loop. The editor must preserve text. The
compiler must report failures. The renderer must rebuild without destroying
the last good image. The shell must keep all of it running in one loop.

## The capstone principle

A shader IDE is not a pile of features. It is one loop made fast and safe:

```text
edit -> compile -> render -> preview -> edit
```

Every subsystem exists to shorten or explain that loop. A developer who can
build the loop, keep it alive through errors, and extend it with new
execution models has built the foundation of a real shader tool.

## Next step

Now type the code version of this lesson.
