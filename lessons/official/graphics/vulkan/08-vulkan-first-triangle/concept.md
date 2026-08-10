# Vulkan first triangle - concepts

The first triangle is the point where the Vulkan objects from the earlier
lessons become a complete rendering operation. The goal is not to introduce
every part of a renderer at once. The goal is to understand the chain that
turns three vertices into pixels in a swapchain image.

The complete mental model is:

```text
vertices
    -> vertex buffer
    -> vertex shader
    -> triangle assembly
    -> rasterization
    -> fragment shader
    -> color attachment
    -> present
```

Every stage has a specific responsibility. Vulkan does not infer the missing
pieces, so the application must create and connect the objects explicitly.

## The triangle's data

A triangle needs three vertices. For a first triangle, the simplest useful
vertex contains only a position.

```cpp
struct Vertex {
    float x;
    float y;
    float z;
};
```

The positions are normally supplied in the coordinate system expected by the
vertex shader. The vertex buffer then stores those structures in GPU-visible
memory.

The important distinction is between the C++ representation and the GPU
resource. Vertex is a CPU-side description of the bytes, while the vertex
buffer is the Vulkan object that makes those bytes available to a draw.

## The vertex buffer

A vertex buffer is bound to a command buffer before the draw.

```cpp
VkDeviceSize offset = 0;

vkCmdBindVertexBuffers(
    commandBuffer,
    0,
    1,
    &vertexBuffer,
    &offset
);
```

The first argument after the command buffer identifies the vertex binding
slot. The pipeline's vertex input description must agree with the layout of
the data in that buffer.

The binding therefore connects three things:

```text
Vertex structure
      |
      v
vertex buffer bytes
      |
      v
vertex input state
      |
      v
vertex shader input
```

If these descriptions disagree, the shader receives data that does not match
the application's intended interpretation.

## The vertex shader

The vertex shader runs once for each vertex processed by the draw. Its primary
responsibility for this lesson is to produce a position.

Conceptually, the shader performs:

```glsl
gl_Position = vec4(inPosition, 1.0);
```

The resulting position passes through Vulkan's vertex-processing stages before
the primitive reaches rasterization.

For the first triangle, there is no camera, matrix, model transform, or
lighting system. The vertices are deliberately kept in a simple coordinate
range so the pipeline can be understood without introducing unrelated
mathematics.

## Triangle assembly

The draw command specifies three vertices:

```cpp
vkCmdDraw(
    commandBuffer,
    3,
    1,
    0,
    0
);
```

With triangle-list topology, those three vertices become one triangle.

The arguments represent vertex count, instance count, first vertex, and first
instance. For the first triangle, the important value is the vertex count of
three.

The draw command does not contain the triangle's geometry. It tells Vulkan
how much vertex data to process using the currently bound vertex buffer and
pipeline.

## Rasterization creates fragments

The triangle is a geometric primitive. Rasterization determines which sample
locations are covered by that triangle and produces fragments for those
locations.

```text
triangle
    |
    v
rasterizer
    |
    v
covered samples
    |
    v
fragments
```

The fragment shader then runs for the generated fragment work.

This is why a triangle is not simply "three vertices sent to the screen".
There is an intermediate geometric and rasterization process that turns the
three vertices into many potential fragment invocations.

## The fragment shader

The fragment shader determines the color produced by a fragment.

```glsl
outColor = vec4(1.0, 0.2, 0.1, 1.0);
```

For the first triangle, a constant color is useful because it isolates the
pipeline from interpolation, textures, lighting, and descriptor resources.

Later, the fragment shader can consume interpolated values from the vertex
shader or resources supplied through descriptor sets.

The important lesson here is that the fragment shader produces a value; the
rendering attachment and its associated pipeline state determine how that
value participates in the final image.

## The render target

A triangle needs somewhere to write its color. In the swapchain-based renderer,
that destination is normally a swapchain image used as the color attachment.

The render target is therefore part of the path:

```text
fragment shader
      |
      v
color attachment
      |
      v
swapchain image
```

The pipeline must be compatible with the format and rendering configuration of
that target.

A swapchain image is not simply a texture that can always be written to.
Before rendering, the application must have it in the appropriate layout and
must establish the rendering operation that will use it.

## Beginning rendering

Before issuing the draw, the command buffer begins a rendering operation.

With dynamic rendering, the application describes the color attachment
directly:

```cpp
VkRenderingAttachmentInfo colorAttachment{
    VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
    nullptr,
    colorImageView,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    VK_RESOLVE_MODE_NONE,
    VK_NULL_HANDLE,
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_ATTACHMENT_LOAD_OP_CLEAR,
    VK_ATTACHMENT_STORE_OP_STORE,
    clearValue
};
```

The attachment view identifies which image view receives the rendered color.
The load operation says what happens to its previous contents, while the store
operation says whether the resulting contents remain available afterward.

For a first triangle, clearing the attachment at the beginning and storing it
at the end gives a simple predictable flow.

## The rendering area

The rendering operation also needs an extent and layer count.

```cpp
VkRenderingInfo renderingInfo{
    VK_STRUCTURE_TYPE_RENDERING_INFO,
    nullptr,
    0,
    renderArea,
    1,
    0,
    1,
    &colorAttachment,
    nullptr,
    nullptr
};
```

The render area identifies the region affected by the rendering operation.
The layer count describes how many image layers participate.

Once this information is available, the command buffer can begin rendering:

```cpp
vkCmdBeginRendering(commandBuffer, &renderingInfo);
```

This establishes the attachment context in which the draw will execute.

## Binding the graphics pipeline

The pipeline from the previous lesson describes how the triangle should be
processed.

```cpp
vkCmdBindPipeline(
    commandBuffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    graphicsPipeline
);
```

The pipeline must match the rendering configuration and the vertex data being
supplied.

The command buffer now has a rendering operation, a compatible pipeline, and
the resources required to provide vertices.

## Binding the vertex buffer

The vertex buffer is selected immediately before the draw.

```cpp
vkCmdBindVertexBuffers(
    commandBuffer,
    0,
    1,
    &vertexBuffer,
    &offset
);
```

This does not copy vertex data. It changes the state used by subsequent
graphics commands so the vertex fetch stage reads from the selected buffer.

The binding slot must agree with the vertex input binding description used when
the pipeline was created.

## Recording the draw

The actual triangle is requested with one draw command.

```cpp
vkCmdDraw(commandBuffer, 3, 1, 0, 0);
```

The command means that three vertices should be processed, one instance should
be produced, and processing begins at vertex and instance zero.

At this point the important objects have finally converged:

```text
rendering attachment
       +
graphics pipeline
       +
vertex buffer
       +
draw command
       |
       v
    triangle
```

The draw itself is small because most of the work was expressed when the
pipeline and resources were created.

## Ending rendering

After the draw, the rendering operation ends.

```cpp
vkCmdEndRendering(commandBuffer);
```

This closes the attachment context established by vkCmdBeginRendering.

The command buffer can then continue with work outside the rendering
operation, including transitions required before presentation.

## Presenting the image

Rendering produces an image. Presentation makes the selected swapchain image
available to the presentation engine.

Conceptually, the frame has two distinct phases:

```text
render
  |
  v
finished swapchain image
  |
  v
present
```

Rendering and presentation are therefore not the same operation. A renderer
must synchronize them and ensure that the image is in a state suitable for the
next operation.

## Why this is the first real milestone

The first triangle is valuable because it validates the complete graphics path.
If the triangle appears, the application has successfully connected device
selection, queues, swapchain images, rendering state, shaders, vertex data,
command recording, synchronization, and presentation.

The triangle itself is simple. The achievement is the chain of Vulkan objects
that makes the simple geometry visible.

## What changes after the triangle

A triangle is not the end of the renderer. It is the baseline from which more
useful systems are added.

Transforms require uniform or push-constant data. Textures require image views
and samplers. Material data requires descriptors. Depth requires a depth
attachment and depth state. Multiple objects require better resource and
pipeline organization.

Those additions are easier to understand once the basic draw path is already
clear.

## The complete first-triangle flow

The entire lesson can be reduced to this sequence:

```text
acquire image
    -> begin rendering
    -> bind pipeline
    -> bind vertex buffer
    -> draw 3 vertices
    -> end rendering
    -> transition for presentation
    -> present
```

The next lessons will add the resource interfaces that make this basic draw
path useful for real scenes.

## Next step

Now type the code version of this lesson.
