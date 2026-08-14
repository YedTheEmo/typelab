# Shader IDE overview - concepts

A shader IDE is an application that lets you edit shader source, compile it,
and see the result immediately. The core activity is a loop: change a shader,
compile it, run it on the GPU, and inspect the output. When the loop is fast
and the feedback is visual, shader development becomes experimental instead of
painful.

This course builds a small but complete shader IDE. The application uses a
C++17 host with SDL3 for the window and input, Dear ImGui for the editor
interface, Vulkan for GPU execution, and the Slang shader compiler to turn
shader source into SPIR-V.

For this course, assume:

```text
SDL3            -> window, events, input
Dear ImGui      -> editor UI, panels, text input
Vulkan          -> GPU pipeline, rendering, presentation
Slang           -> shader compilation to SPIR-V
```

The IDE does not replace a full commercial editor. It is a focused tool whose
entire job is to shorten the feedback loop between changing a shader and
seeing its visual result.

## The golden loop

The behavior that defines a shader IDE is a repeated cycle:

```text
edit source
    |
    v
compile with Slang
    |
    v
render with Vulkan
    |
    v
show preview
    |
    v
edit again
```

Every feature in the application either serves this loop or reports on it.
The editor panel changes the source. The compiler turns the source into
SPIR-V. The renderer turns SPIR-V into an image. The preview panel shows that
image back to the user.

Speed matters as much as correctness. A user should be able to edit a line
and see the result without touching anything else. This is what makes shader
development feel interactive.

## The three subsystems

The application is organized around three cooperating subsystems.

The editor manages text. It stores the shader source, tracks the cursor, and
accepts edits. It also keeps the information needed to point at a specific
line when a compile error appears.

The compiler wraps Slang. It receives source text, compiles it to SPIR-V, and
returns either valid SPIR-V or a list of diagnostics. It does not render
anything.

The renderer wraps Vulkan. It takes SPIR-V, builds a pipeline, runs the shader
on the GPU, and produces an image. It does not know how to edit text.

The application shell connects the three. It owns the window, the frame loop,
and the panel layout.

```text
shell (window, loop, panels)
    |
    +--> editor        (text)
    |
    +--> compiler      (Slang -> SPIR-V)
    |
    +--> renderer      (Vulkan -> image)
```

## Source, program, pipeline, image

A shader passes through several distinct states.

The source is text in the Slang language. It is what the user edits.

The program is the compiled result: a linked module with a selected entry
point. Slang produces a component that can be compiled for a target.

The SPIR-V blob is the binary representation that Vulkan understands.

The pipeline is a Vulkan object that combines the compiled shaders with
fixed-state configuration: which attachment the shader writes, how geometry
is assembled, and which descriptor sets are bound.

The image is the final output the user sees in the preview panel.

```text
source
  |
  v
program + entry point
  |
  v
SPIR-V
  |
  v
Vulkan pipeline
  |
  v
rendered image
```

The IDE must keep each state separate so that a failure at one stage does not
destroy the others.

## Compilation can fail

Shader compilation is a routine event, not an emergency. A user writes a line,
and the compiler rejects it. The IDE must handle this gracefully.

The important design decision is that a failed compile is a data result. The
compiler returns diagnostics: a list of messages, each with a location. The
IDE stores the diagnostics, keeps the last successful shader running, and
shows the error to the user.

```text
compile
  |
  +--> success -> SPIR-V -> render
  |
  +--> failure -> diagnostics -> keep last image
```

Keeping the last good image is essential. A preview that goes black on every
typo teaches nothing. The user should see both the error and the most recent
working result.

## The editor is not a language server

A minimal shader IDE does not need to parse the shader it displays. It treats
the source as text: characters, lines, and a cursor.

This boundary keeps the editor simple. Syntax highlighting, autocomplete, and
diagnostics can be layered on top later, but they are separate systems. The
editor's only hard requirement is to preserve the text and report the current
cursor position so the compiler's line-based diagnostics can be displayed.

## The renderer owns GPU state

Vulkan state has explicit lifetimes. The instance, device, swapchain, and
queue are created once and destroyed at shutdown. Per-frame resources are
reused. The renderer must own this lifecycle rather than letting the
application shell see every handle.

A useful separation:

```text
device lifetime      -> instance, device, queues, swapchain
program lifetime     -> SPIR-V, pipeline, descriptor sets
frame lifetime       -> command buffers, synchronization, per-frame data
```

When the user changes the shader, only the program-level objects need to be
rebuilt. The device and swapchain stay untouched.

## Uniforms connect the application to the shader

A shader that always produces the same image is not interactive. The IDE
provides a small set of uniform values that change over time: the current
time, the preview resolution, and the mouse position.

These values live in a uniform buffer that the renderer updates once per
frame. The shader declares a parameter block that matches the layout of the
buffer.

```text
application state      shader side
     |
     v
uniform buffer    <->  parameter block
     |
     v
descriptor set         uniform access
```

The Slang shader and the C++ buffer must agree on the layout. This agreement
is part of the interface between the host and the shader.

## Panels make the loop visible

The window is divided into panels. The editor panel holds the text. The
preview panel shows the rendered image. The diagnostics panel lists compile
errors and warnings.

```text
+-------------------------+---------------------+
| editor panel            | preview panel       |
| (shader source)         | (rendered image)    |
|                         |                     |
|                         |                     |
+-------------------------+---------------------+
| diagnostics panel       |                     |
| (compile messages)      |                     |
+-------------------------+---------------------+
```

Dear ImGui provides the windowing and docking structure. The preview panel is
not an ImGui widget in the ordinary sense: it is a texture produced by Vulkan
that ImGui displays. This is the meeting point of the two major systems.

## The entry point is part of the program

Slang modules can contain many functions. The IDE needs to know which function
is the shader entry point. For this course the entry point is named main.

A fragment shader entry point has this shape:

```slang
[shader("fragment")]
float4 main(float2 uv : SV_Position) : SV_Target
{
    return float4(1.0, 0.0, 0.0, 1.0);
}
```

The IDE compiles the module and selects the entry point by name. This is the
smallest amount of program structure the compiler needs to produce SPIR-V.

## Multiple execution models

A shader IDE can preview more than one kind of shader. The fragment model
renders a full-screen image from a fragment shader. The compute model
dispatches a compute kernel that writes into an image, then shows that image.

```text
fragment shader -> pipeline renders full-screen triangle
compute shader  -> dispatch writes to storage image -> preview
```

This course starts with the fragment model because it matches the idea of a
preview image directly. The compute model is added later as a second
execution path.

## Hot reload is the payoff

Because compilation is fast and the renderer keeps its state separate, the
IDE can recompile automatically after the user stops typing. This is hot
reload: edit, pause, and the preview updates itself.

The mechanism is:

```text
edit event
    |
    v
debounce (wait for pause)
    |
    v
compile
    |
    v
rebuild pipeline
    |
    v
next frame uses new shader
```

Debouncing prevents recompiling on every keystroke. The user pauses briefly,
the IDE notices the pause, and one compile happens.

## Scope discipline

A focused shader IDE deliberately leaves out features that belong elsewhere.
It does not implement its own terminal, package manager, or full language
server. It does not need to. Its value is the tight loop between source,
compile, and preview.

The rules of scope are:

```text
source text        -> editor
compilation        -> compiler
rendering          -> renderer
window and panels  -> shell
```

When a feature does not clearly belong to one of these four boxes, it should
wait.

## What this course builds

The complete application is assembled from these pieces:

```text
window and Vulkan shell
    +
ImGui integration
    +
editor text buffer
    +
Slang compilation
    +
preview pipeline
    +
uniforms and input
    +
hot reload
    +
diagnostics and error highlighting
    +
panels and layout
    +
projects and file I/O
    +
compute preview
    +
final integration
```

Each lesson adds one capability and keeps the existing ones working. By the
final lesson the IDE is a single program that edits, compiles, runs, and
displays shaders in a continuous loop.

## The learning sequence

The sequence of lessons matches the build order of the real application.
Later lessons depend on the vocabulary established earlier, so the ordering is
not arbitrary.

```text
window shell
    ->
ImGui
    ->
text buffer
    ->
Slang compile
    ->
preview pipeline
    ->
uniforms
    ->
hot reload
    ->
diagnostics
    ->
layout
    ->
projects
    ->
compute
    ->
full integration
```

## Next step

Now type the code version of this lesson.
