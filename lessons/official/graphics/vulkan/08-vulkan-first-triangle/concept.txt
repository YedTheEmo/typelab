# Vulkan first triangle - concepts

The first triangle is useful because it forces the major Vulkan concepts to
work together.

A triangle requires vertex data, shaders, a graphics pipeline, a render target,
a command buffer, synchronization, and a presentation path.

The important lesson is not the triangle itself. It is how the objects introduced
in the previous lessons form one complete frame.

## Vertex data

The simplest triangle contains three positions:

```
(-0.5, -0.5)
( 0.5, -0.5)
( 0.0,  0.5)
```

A vertex shader receives these positions and converts them into clip-space
positions:

```
gl_Position = vec4(position, 0.0, 1.0);
```

The vertex shader runs once for each vertex.

The three resulting vertices form one triangle.

## The graphics pipeline

The pipeline connects the vertex shader to rasterization and the fragment
shader.

The vertex stage transforms the input vertices.

Rasterization determines which pixels the triangle covers.

The fragment shader determines the color of those fragments:

```
color = vec4(1.0, 0.0, 0.0, 1.0);
```

The simplified path is:

```
vertices
    |
    v
vertex shader
    |
    v
rasterization
    |
    v
fragment shader
    |
    v
color attachment
```

The pipeline object contains the configuration needed for this process.

## The swapchain image is the target

The swapchain provides images that can be presented to the window.

The application acquires one:

```
vkAcquireNextImageKHR(..., &imageIndex);
```

That image becomes the target for the current frame.

Before drawing, the command buffer must describe how the rendering operation
uses the image.

With modern Vulkan, dynamic rendering can describe the color attachment
directly rather than requiring a traditional render pass object.

Conceptually:

```
swapchain image
      |
      v
color attachment
      |
      v
   triangle
```

## Record the frame

The command buffer records the operations needed to render the triangle.

First begin recording:

```
vkBeginCommandBuffer(
    commandBuffer,
    &beginInfo);
```

Then begin rendering and bind the pipeline.

```
vkCmdBindPipeline(
    commandBuffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    graphicsPipeline);
```

The vertex buffer is bound next:

```
vkCmdBindVertexBuffers(
    commandBuffer,
    0,
    1,
    &vertexBuffer,
    &offset);
```

Finally, issue the draw:

```
vkCmdDraw(
    commandBuffer,
    3,
    1,
    0,
    0);
```

The command buffer now describes the rendering work.

## Submit the frame

The acquired image must be ready before the graphics work uses it.

The submission therefore waits on the image-available semaphore:

```
submitInfo.waitSemaphoreCount = 1;
submitInfo.pWaitSemaphores = &imageAvailable;
```

The rendering submission signals renderFinished:

```
submitInfo.signalSemaphoreCount = 1;
submitInfo.pSignalSemaphores = &renderFinished;
```

The queue executes the recorded commands:

```
vkQueueSubmit(
    graphicsQueue,
    1,
    &submitInfo,
    inFlightFence);
```

The fence tells the CPU when this submission has finished.

## Present the result

After rendering completes, presentation waits for renderFinished:

```
presentInfo.waitSemaphoreCount = 1;
presentInfo.pWaitSemaphores = &renderFinished;
```

The image is then presented:

```
vkQueuePresentKHR(
    presentQueue,
    &presentInfo);
```

The complete frame is now:

```
acquire
    |
    v
record
    |
    v
submit
    |
    v
render
    |
    v
present
```

Synchronization ensures that each stage sees the resource in the state it
expects.

## What Vulkan is actually doing

The triangle demonstrates the real Vulkan mental model.

The CPU does not tell the GPU "draw a triangle" as one high-level operation.

Instead, the application creates resources and describes state, records
commands into a command buffer, submits that buffer to a queue, and explicitly
coordinates the relationship between rendering and presentation.

The GPU then executes the resulting work.

This is why Vulkan initially feels complicated. The triangle is simple, but
the API exposes the machinery required to make the triangle happen.

## The foundation is complete

At this point, the major concepts fit together:

```
instance
    |
    v
physical device
    |
    v
logical device
    |
    +-- queue
    |
    +-- resources
    |
    +-- pipeline
    |
    +-- command buffer
    |
    +-- synchronization
    |
    v
swapchain
    |
    v
presented frame
```

The next Vulkan topics can now build on this foundation instead of introducing
the API from scratch.

## Next step

Now type the code version of this lesson.

