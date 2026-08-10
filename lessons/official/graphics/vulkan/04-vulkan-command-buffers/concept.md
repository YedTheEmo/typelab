# Vulkan command buffers - concepts

A Vulkan application does not normally issue rendering commands directly to a
GPU queue one function call at a time. Instead, it records commands into a
command buffer and later submits that completed command buffer to a queue.

The basic relationship is:

```text
application
    |
    v
command buffer
    |
    v
queue submission
    |
    v
GPU execution
```

This separation is one of the defining characteristics of Vulkan. Recording
commands and executing commands are separate operations, and the application
controls when each happens.

## A command buffer is recorded work

A `VkCommandBuffer` is an object into which Vulkan commands are recorded. It is
not itself a queue, and recording a command does not immediately execute that
command on the GPU.

A command buffer handle can be stored like any other Vulkan object:

```cpp
VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
```

Commands are then written into the buffer:

```cpp
vkCmdBindPipeline(
    commandBuffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    graphicsPipeline
);
```

The `vkCmd` prefix is important. Functions beginning with `vkCmd` generally
record commands into a command buffer rather than executing the requested work
immediately.

The distinction is therefore:

```text
vkCmd...()
    -> records work

vkQueueSubmit(...)
    -> submits recorded work
```

This lets the application construct a sequence of GPU operations before asking
a queue to execute them.

## Command pools own command buffers

Command buffers are allocated from a `VkCommandPool`.

The pool belongs to a queue family:

```cpp
VkCommandPoolCreateInfo poolInfo{
    VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    nullptr,
    0,
    graphicsFamily
};
```

The queue-family association matters because command buffers are intended to
record work for queues from that family.

The pool itself is then created:

```cpp
VkCommandPool commandPool = VK_NULL_HANDLE;

vkCreateCommandPool(
    device,
    &poolInfo,
    nullptr,
    &commandPool
);
```

The relationship is:

```text
logical device
    |
    v
command pool
    |
    +-> command buffer
    +-> command buffer
    +-> command buffer
```

The command pool is therefore an allocation and management context for command
buffers rather than a container of commands that executes work itself.

## Why command pools use queue families

Vulkan exposes several kinds of queue families because a physical device can
provide queues with different capabilities.

A command pool is created for one queue family because command buffers allocated
from that pool are associated with that family's execution capabilities.

For a renderer whose graphics and presentation operations use the selected
graphics family, the same family can commonly be used for the command pool:

```cpp
poolInfo.queueFamilyIndex = graphicsFamily;
```

This connects the device-selection work from Lesson 2 to command recording.

The chain is now:

```text
physical device
    -> queue family
    -> logical device
    -> queue
    -> command pool
    -> command buffer
```

The queue family is therefore not just information needed during device
creation. It continues to influence the resources used to construct GPU work.

## Allocating command buffers

A command buffer is allocated by describing how many buffers are needed and
what kind they should be.

```cpp
VkCommandBufferAllocateInfo allocateInfo{
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    nullptr,
    commandPool,
    VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    1
};
```

The `VK_COMMAND_BUFFER_LEVEL_PRIMARY` value means that the command buffer can
be submitted directly to a queue.

The allocation produces the actual handle:

```cpp
VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

vkAllocateCommandBuffers(
    device,
    &allocateInfo,
    &commandBuffer
);
```

Vulkan also supports secondary command buffers. A secondary buffer is not
submitted directly to a queue in the same way as a primary buffer. Instead, a
primary command buffer can execute it as part of a larger recorded sequence.

For the initial renderer, primary command buffers are enough.

## Begin recording

A newly allocated command buffer must enter the recording state before
commands can be written into it.

```cpp
VkCommandBufferBeginInfo beginInfo{
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    nullptr,
    0,
    nullptr
};

vkBeginCommandBuffer(commandBuffer, &beginInfo);
```

The `VkCommandBufferBeginInfo` structure controls how recording begins.

The simplest case uses no special usage flags. The application begins recording,
writes commands, and eventually ends recording.

The state transition is:

```text
allocated
    |
    v
recording
    |
    v
executable
```

This state model is important because a command buffer cannot simply receive
commands at arbitrary times.

## Recording commands

Once recording has begun, commands can be added to the buffer.

For example, a viewport can be configured:

```cpp
VkViewport viewport{
    0.0f,
    0.0f,
    width,
    height,
    0.0f,
    1.0f
};

vkCmdSetViewport(
    commandBuffer,
    0,
    1,
    &viewport
);
```

A scissor rectangle can also be recorded:

```cpp
VkRect2D scissor{
    {0, 0},
    {width, height}
};

vkCmdSetScissor(
    commandBuffer,
    0,
    1,
    &scissor
);
```

These calls still do not configure the GPU immediately. They append commands
to the command buffer's recorded sequence.

The command buffer can therefore be thought of as a program for the GPU:

```text
command 1
command 2
command 3
command 4
```

The queue will later submit this recorded sequence for execution.

## Commands have ordering

Commands recorded into one command buffer have an explicit order.

Suppose the application records:

```cpp
vkCmdBindPipeline(commandBuffer, ...);
vkCmdBindDescriptorSets(commandBuffer, ...);
vkCmdDraw(commandBuffer, ...);
```

The intended sequence is:

```text
bind pipeline
    ->
bind resources
    ->
draw
```

The GPU does not interpret these as unrelated function calls. They form an
ordered sequence of operations within the command buffer.

This is one reason command buffers are useful: the application constructs the
execution sequence before submitting it.

However, command ordering alone does not solve every synchronization problem.
Resources can be accessed by different commands, queues, or submissions, and
those relationships require explicit synchronization mechanisms. Those belong
to the next lesson.

## Rendering commands have structure

A graphics command buffer is not simply a flat list of arbitrary commands.
Some commands are valid only within particular rendering contexts.

For example, traditional render-pass rendering begins a render pass:

```cpp
vkCmdBeginRenderPass(
    commandBuffer,
    &renderPassInfo,
    VK_SUBPASS_CONTENTS_INLINE
);
```

Rendering commands then occur inside that region:

```cpp
vkCmdBindPipeline(
    commandBuffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    graphicsPipeline
);

vkCmdDraw(
    commandBuffer,
    3,
    1,
    0,
    0
);
```

The rendering region is then closed:

```cpp
vkCmdEndRenderPass(commandBuffer);
```

The exact rendering model will be explored more thoroughly in later lessons.
For now, the important idea is that commands can have contextual validity.

## Command recording is CPU work

Recording a command buffer is performed by the application on the CPU.

The CPU executes calls such as:

```cpp
vkCmdDraw(
    commandBuffer,
    3,
    1,
    0,
    0
);
```

but the triangle is not drawn at that moment.

The call records enough information for the GPU to perform the draw when the
command buffer is eventually submitted.

This distinction is fundamental:

```text
CPU
 |
 | record
 v
command buffer
 |
 | submit
 v
GPU
 |
 | execute
 v
rendered result
```

It explains why Vulkan applications can spend substantial CPU time constructing
command buffers even though the actual graphics work happens later on the GPU.

## Ending recording

Once all desired commands have been recorded, the command buffer is ended.

```cpp
VkResult result = vkEndCommandBuffer(commandBuffer);
```

After successful completion, the command buffer becomes executable.

The lifecycle is therefore:

```text
allocated
    ->
begin recording
    ->
record commands
    ->
end recording
    ->
executable
```

The executable command buffer can then be submitted to a queue.

Ending recording does not execute anything. It only finishes constructing the
sequence that can later be submitted.

## Submitting a command buffer

A queue submission connects the recorded command buffer to actual GPU
execution.

The application describes the command buffer with `VkSubmitInfo`:

```cpp
VkSubmitInfo submitInfo{
    VK_STRUCTURE_TYPE_SUBMIT_INFO,
    nullptr,
    0,
    nullptr,
    nullptr,
    1,
    &commandBuffer,
    0,
    nullptr
};
```

The command buffer is then submitted:

```cpp
vkQueueSubmit(
    graphicsQueue,
    1,
    &submitInfo,
    fence
);
```

This is the point where recorded work enters the queue.

The distinction between recording and submission is now explicit:

```text
vkBeginCommandBuffer
        |
        v
    vkCmd...
        |
        v
vkEndCommandBuffer
        |
        v
vkQueueSubmit
        |
        v
    GPU executes
```

## A queue is not a command buffer

The command buffer and queue have different responsibilities.

The command buffer describes work. The queue accepts work and schedules it for
execution on the device.

A useful mental model is that a command buffer is closer to a prepared command
list, while a queue is the execution path to which that list is submitted.

```text
command buffer
    = recorded instructions

queue
    = execution submission path
```

A single queue can receive many command buffer submissions over time.

Likewise, an application can maintain multiple command buffers and choose which
one to submit for a particular frame.

## One command buffer per frame image

A common renderer design maintains multiple command buffers, often one for each
frame-in-flight or swapchain image.

For example:

```cpp
std::vector<VkCommandBuffer> commandBuffers(
    swapchainImageCount
);
```

The application can then record the commands needed to render each image.

The important distinction is that the number of command buffers is a renderer
design decision. Vulkan does not require exactly one command buffer per
swapchain image.

Multiple buffers can be useful because the CPU may prepare one set of commands
while the GPU is executing another.

This becomes especially important once synchronization and multiple frames in
flight are introduced.

## Resetting command buffers

A command buffer can be reused after its previous execution has completed and
the application has established that it is safe to modify it again.

A command buffer can be reset:

```cpp
vkResetCommandBuffer(
    commandBuffer,
    0
);
```

After resetting, the command buffer returns to an initial state and can be
recorded again.

The important restriction is that the application must not reset and overwrite
a command buffer while the GPU is still executing commands recorded in it.

This is one of the places where command-buffer lifetime and synchronization
intersect.

The next lesson will make that relationship explicit.

## Resetting a command pool

A command pool can also be reset:

```cpp
vkResetCommandPool(
    device,
    commandPool,
    0
);
```

Resetting the pool resets the command buffers allocated from it.

This can be useful when a renderer wants to discard and rebuild a group of
command buffers together.

The pool therefore provides a higher-level lifetime boundary than an individual
command buffer.

```text
command pool
    |
    +-> buffer A
    +-> buffer B
    +-> buffer C
```

Resetting the pool affects the buffers beneath that boundary.

## Freeing command buffers

Command buffers can be explicitly returned to their command pool:

```cpp
vkFreeCommandBuffers(
    device,
    commandPool,
    1,
    &commandBuffer
);
```

The command pool itself can later be destroyed:

```cpp
vkDestroyCommandPool(
    device,
    commandPool,
    nullptr
);
```

Destroying the pool releases its command-buffer allocation resources.

As with other Vulkan objects, destruction must respect GPU usage. The
application cannot destroy command-related resources while the device is still
using them.

## Command buffers and the swapchain

The previous lesson introduced the swapchain images. Command buffers are what
record the operations that eventually render into those images.

The relationship becomes:

```text
swapchain
    |
    +-> image
          |
          v
    rendering commands
          ^
          |
    command buffer
          |
          v
        queue
```

The command buffer does not own the swapchain image. It records commands that
refer to resources such as the image, image view, pipeline, buffers, and
descriptors.

This separation is important because Vulkan resources are generally
independent objects connected through recorded commands.

## A minimal frame command sequence

A simplified graphics command buffer might follow this conceptual sequence:

```text
begin command buffer
    ->
begin rendering
    ->
bind pipeline
    ->
bind resources
    ->
draw
    ->
end rendering
    ->
end command buffer
```

The actual commands depend on the rendering model and resources available to
the renderer.

The important idea is that the command buffer describes the complete sequence
that the GPU should execute for that submission.

## Command buffers are reusable descriptions

A command buffer does not necessarily have to be recorded only once. Many
applications record reusable command structures repeatedly.

For a static scene, a command buffer might be rebuilt only when the renderer
state changes. For a dynamic scene, it may be recorded every frame.

The correct choice depends on what changes between frames.

This is why command recording should be understood as a CPU-side construction
step rather than as an inherently one-time initialization operation.

## The command-buffer model

The complete model introduced in this lesson is:

```text
queue family
    |
    v
command pool
    |
    v
command buffer
    |
    +-> begin
    |
    +-> record vkCmd... operations
    |
    +-> end
    |
    v
VkSubmitInfo
    |
    v
queue
    |
    v
GPU execution
```

The command buffer is the bridge between the application's CPU-side description
of work and the queue's GPU-side execution.

The swapchain gives the renderer images to work with. The command buffer gives
the renderer a place to describe what should happen to those resources.

The next problem is timing: when can a command buffer be reused, when is an
image available, and when has submitted GPU work finished?

That is the purpose of Vulkan synchronization.

## Next step

Now type the code version of this lesson.
````

