# Uniforms and input - concepts

A shader that renders the same image forever is boring. The IDE makes shaders
interactive by sending per-frame values into the shader through a uniform
buffer: the current time, the preview resolution, and the mouse position.
This lesson covers that buffer, its layout agreement between C++ and Slang,
and how SDL input reaches the shader.

## Uniforms are the interface to the host

A shader cannot read the window, the mouse, or the clock directly. The host
must pass those values in. The mechanism is a uniform buffer: a block of GPU
memory that the host writes and the shader reads.

```text
host state (time, resolution, mouse)
    |
    v
uniform buffer
    |
    v
shader parameter block
    |
    v
shader uses the values
```

The uniform buffer is updated once per frame and bound to the pipeline through
the descriptor set created in the previous lesson.

## A small set of values

The IDE provides a deliberately small set of uniforms:

```text
time       -> seconds since the app started, and frame delta time
resolution -> the preview size in pixels
mouse      -> the mouse position in pixels
```

These three are enough for a wide range of experimental shaders. Time drives
animation, resolution gives the shader its coordinate system, and the mouse
lets the user interact.

The values are grouped in a struct so that layout is explicit.

## The layout agreement

The C++ struct and the Slang parameter block must describe the same memory.
If the two disagree, the shader reads garbage values.

The safe choice is to use four-component members everywhere:

```cpp
struct FrameUniforms {
    float4 time;       // x = seconds, y = delta time
    float4 resolution; // xy = preview size in pixels
    float4 mouse;      // xy = mouse position in pixels
};
```

And the matching Slang declaration:

```slang
struct FrameUniforms {
    float4 time;
    float4 resolution;
    float4 mouse;
};

ConstantBuffer<FrameUniforms> frame;
```

Every member is aligned to sixteen bytes in both languages, so the layout
cannot drift. This is the contract between the host and the shader, and it
is the one place in the application where two languages must agree exactly.

## Creating the buffer

The uniform buffer is a small GPU allocation. It is created once with a size
matching the struct and is mapped into host memory so the host can write to
it every frame.

```text
VkBuffer (uniform usage)
    +
VkDeviceMemory (host visible)
    +
persistent mapping
```

Mapping the buffer once and writing to the mapping each frame is the
simplest approach for a small, frequently updated buffer. The host writes new
values into the mapped pointer before each frame's rendering.

## The descriptor set

The descriptor set created in the previous lesson references the uniform
buffer. The descriptor write binds the buffer and its size:

```text
descriptor set
    binding 0 -> uniform buffer (FrameUniforms)
```

The same descriptor set is bound during the preview draw. When the shader
declares the parameter block, the driver resolves it to this buffer.

## Updating per frame

Every frame the renderer writes fresh values into the mapped buffer.

```cpp
FrameUniforms* data = static_cast<FrameUniforms*>(mapping);
data->time = { seconds, deltaSeconds, 0.0f, 0.0f };
data->resolution = { width, height, 0.0f, 0.0f };
data->mouse = { mouseX, mouseY, 0.0f, 0.0f };
```

The update is cheap: one small memcpy-equivalent into persistent GPU memory.

## Time

Time comes from a clock owned by the application. The renderer keeps the
start time and computes the elapsed seconds each frame.

```text
seconds     = now - start
delta time  = now - lastFrame
```

Delta time lets shaders animate smoothly even when the frame rate changes.
Both values are written so a shader can choose either.

## Resolution

The resolution is the preview size in pixels. The renderer already knows the
target size from the previous lesson, so it writes those values directly.

The shader usually divides coordinates by the resolution to get normalized
coordinates:

```slang
float2 uv = fragCoord / frame.resolution.xy;
```

The resolution is also a signal: when the preview panel is resized, the new
size reaches the shader on the next frame.

## Mouse

The mouse position comes from SDL. The host queries the current position in
window coordinates and writes it to the buffer.

```cpp
float mouseX = 0.0f;
float mouseY = 0.0f;
SDL_GetMouseState(&mouseX, &mouseY);
```

Because the mouse is queried once per frame, no event plumbing is needed. The
shader always sees the position as of the last frame, which is exactly what a
live preview wants.

## Descriptor update discipline

The descriptor set is bound once during the preview draw. Because the buffer
is persistently mapped and the set is bound every frame, there is no
per-frame descriptor allocation. The driver sees the same set and the same
buffer, with new contents.

```text
frame N    -> write values -> draw -> shader reads frame N values
frame N+1  -> write values -> draw -> shader reads frame N+1 values
```

This discipline avoids the descriptor churn that plagues naive bindings and
keeps the frame simple.

## What this lesson establishes

The shader now receives live input. Time, resolution, and mouse reach the
shader every frame through one uniform buffer. The next lessons build on this
to recompile automatically and to arrange the interface around the preview.

## Next step

Now type the code version of this lesson.
