# Vulkan frame loop and renderer architecture - concepts

A Vulkan renderer becomes useful when all of its objects cooperate inside a
repeatable frame loop. The loop acquires an image, records commands, submits
them to a queue, and presents the completed image.

The basic cycle is:

```text
wait -> acquire -> reset -> record -> submit -> present
```

This lesson turns the individual Vulkan systems from the previous lessons into
one renderer architecture.

## A frame is work plus synchronization

The CPU does not simply call draw commands and wait for the GPU after every
operation. The renderer prepares work while the GPU executes earlier work.

```text
CPU: record frame N+1
GPU: execute frame N
```

Synchronization objects connect these independent timelines.

A common frame uses an image-available semaphore and a render-finished
semaphore:

```text
acquire
   |
   v
image available
   |
   v
submit rendering
   |
   v
render finished
   |
   v
present
```

## Frames in flight

A renderer can keep more than one frame in flight.

```cpp
constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
```

Each frame can own its own synchronization objects and command buffer.

```text
frame 0 -> fence + semaphores + command buffer
frame 1 -> fence + semaphores + command buffer
```

This allows CPU work for one frame to overlap GPU work for another.

The number is an architectural choice. Two is a common starting point, not a
Vulkan requirement.

## The in-flight fence

A fence lets the CPU know when submitted GPU work has completed.

```cpp
vkWaitForFences(
    device,
    1,
    &frame.inFlightFence,
    VK_TRUE,
    UINT64_MAX
);
```

The renderer waits before reusing resources associated with that frame.

After the wait, the fence is reset before the next submission:

```cpp
vkResetFences(device, 1, &frame.inFlightFence);
```

This makes the fence represent the next GPU submission.

## Acquire a swapchain image

The renderer asks the presentation system for an image.

```cpp
vkAcquireNextImageKHR(
    device,
    swapchain,
    UINT64_MAX,
    frame.imageAvailable,
    VK_NULL_HANDLE,
    &imageIndex
);
```

The semaphore tells the graphics submission that the acquired image is ready
for the rendering work that follows.

The returned index selects the swapchain image to render into.

## Reset the command buffer

A command buffer can be reused after the previous GPU execution has finished.

```cpp
vkResetCommandBuffer(
    frame.commandBuffer,
    0
);
```

The renderer then begins recording:

```cpp
VkCommandBufferBeginInfo beginInfo{
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
};

vkBeginCommandBuffer(
    frame.commandBuffer,
    &beginInfo
);
```

The recorded commands now describe one complete frame.

## Transition the swapchain image

The acquired image needs a layout suitable for rendering.

```text
PRESENT_SRC_KHR
      |
      v
COLOR_ATTACHMENT_OPTIMAL
      |
      v
render
      |
      v
PRESENT_SRC_KHR
```

The transition is part of command recording and must be synchronized with
presentation and rendering.

With synchronization2, the renderer can describe this explicitly using an
image memory barrier.

## Record rendering commands

The renderer binds the pipeline, resources, and geometry before drawing.

```text
bind pipeline
    |
bind descriptor sets
    |
bind vertex buffer
    |
begin rendering
    |
draw
    |
end rendering
```

The command buffer stores these operations. It does not execute them while the
CPU is recording.

This distinction is fundamental to Vulkan:

```text
recording -> build GPU work
submission -> give GPU work to a queue
```

## Submit the frame

A queue submission packages the command buffer and synchronization.

```cpp
VkSubmitInfo submitInfo{
    VK_STRUCTURE_TYPE_SUBMIT_INFO,
    nullptr,
    1,
    &waitSemaphore,
    &waitStage,
    1,
    &frame.commandBuffer,
    1,
    &frame.renderFinished
};
```

The wait semaphore connects image acquisition to rendering. The signal
semaphore connects rendering to presentation.

The fence tracks completion from the CPU's perspective.

## Present the image

After rendering completes, presentation consumes the finished image.

```cpp
VkPresentInfoKHR presentInfo{
    VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    nullptr,
    1,
    &frame.renderFinished,
    1,
    &swapchain,
    &imageIndex,
    nullptr
};
```

The presentation queue waits for the render-finished semaphore.

The frame has now completed its basic journey:

```text
acquire -> record -> submit -> present
```

## Advance the frame index

After submission and presentation, the renderer selects another frame context.

```cpp
currentFrame =
    (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
```

The frame context stores resources that are safe to reuse only after its fence
signals.

This prevents one frame's CPU recording from accidentally reusing command
buffers or synchronization objects still owned by the GPU.

## Per-frame resources

A renderer commonly keeps a frame structure.

```cpp
struct FrameContext {
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAvailable;
    VkSemaphore renderFinished;
    VkFence inFlightFence;
};
```

This groups synchronization and command recording state.

A larger renderer may add per-frame uniform buffers, temporary allocations,
descriptor pools, and staging resources to the same context.

The key architectural principle is ownership: resources that are reused every
frame should have an explicit lifetime relationship with that frame.

## Per-image state

Frames in flight and swapchain images are different concepts.

The current frame tells the CPU which frame context is being reused:

```text
frame context -> CPU/GPU synchronization
```

The acquired image tells the renderer which presentation image it received:

```text
image index -> swapchain image
```

A renderer may therefore need separate tracking for images that are currently
associated with submitted work.

## Images in flight

One common strategy tracks which fence currently owns each swapchain image.

```cpp
std::vector<VkFence> imagesInFlight(
    swapchainImageCount,
    VK_NULL_HANDLE
);
```

After acquiring an image, the renderer can check whether another submission
is still using it.

```cpp
if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(
        device,
        1,
        &imagesInFlight[imageIndex],
        VK_TRUE,
        UINT64_MAX
    );
}
```

The image can then be associated with the current frame's fence.

This prevents the renderer from recording into an image while an earlier frame
still owns its contents.

## Presentation results

Presentation and acquisition can report conditions that require the swapchain
to change.

A common result is:

```cpp
VK_ERROR_OUT_OF_DATE_KHR
```

Another is:

```cpp
VK_SUBOPTIMAL_KHR
```

The renderer should treat these as signals that swapchain-dependent resources
may need recreation.

The exact policy can vary, but the important architectural boundary is that
swapchain recreation is not the same operation as normal frame rendering.

## Swapchain recreation

Window size changes can invalidate the swapchain's assumptions.

```text
resize
  |
  v
wait for GPU
  |
  v
destroy swapchain-dependent resources
  |
  v
create swapchain
  |
  v
create image views
  |
  v
create depth resources
  |
  v
recreate pipelines if formats changed
```

Not every Vulkan object needs to be recreated. The renderer should distinguish
swapchain-dependent state from device-wide state.

## Renderer ownership

A useful architecture groups objects according to lifetime.

```text
Renderer
 |
 +-- Instance
 +-- Device
 +-- Queues
 +-- Swapchain
 +-- Pipeline
 +-- Descriptors
 +-- Textures
 +-- Depth
 +-- Frames
```

This does not require one giant class. The same ownership model can be
implemented with smaller modules.

The important part is knowing which subsystem creates, uses, and destroys each
resource.

## Device-wide resources

Some objects normally live for the lifetime of the renderer.

```text
instance
physical device
logical device
queues
descriptor layouts
pipeline layouts
```

They should not be recreated every frame.

Their lifetime surrounds the frame loop.

## Swapchain-dependent resources

Other objects depend on the current swapchain configuration.

```text
swapchain
swapchain image views
depth image
framebuffers
swapchain-compatible pipelines
```

These are recreated when the presentation configuration changes.

This separation prevents the frame loop from becoming a giant initialization
routine repeated every time the window changes.

## Per-frame resources

Some objects are deliberately duplicated for concurrent frame work.

```text
command buffer
image-available semaphore
render-finished semaphore
fence
frame uniform data
temporary allocations
```

Their lifetime is tied to the frame context rather than to the swapchain.

This is the third major lifetime category:

```text
device lifetime
swapchain lifetime
frame lifetime
```

## A practical render function

The complete renderer can expose a small public operation:

```cpp
void drawFrame() {
    waitForFrame();
    acquireImage();
    recordCommands();
    submitFrame();
    presentFrame();
}
```

Each operation can then hide synchronization and resource-management details.

The frame loop becomes understandable because each stage has one job.

## The architecture boundary

The renderer should not make the application know every Vulkan handle.

A higher-level application can request:

```cpp
renderer.drawFrame();
```

while the renderer manages:

```text
swapchain
command buffers
descriptors
pipelines
attachments
synchronization
```

This creates a useful boundary between application behavior and GPU
infrastructure.

## What this lesson establishes

A Vulkan renderer is not one API call. It is a coordinated system whose frame
loop connects swapchain acquisition, command recording, synchronization, queue
submission, and presentation.

The architecture becomes manageable when resources are grouped by lifetime:
device-wide objects persist for the renderer, swapchain-dependent objects are
recreated when presentation changes, and per-frame objects are reused only
after their associated GPU work completes.

The complete flow is:

```text
wait -> acquire -> record -> submit -> present -> advance
```

That flow is the foundation on which a larger Vulkan renderer can add cameras,
materials, textures, multiple passes, compute work, streaming, and more
advanced synchronization.

## Next step

Now type the code version of this lesson.
