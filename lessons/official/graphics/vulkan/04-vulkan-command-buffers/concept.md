# Vulkan command buffers - concepts

Vulkan does not send individual drawing commands directly to the GPU. Instead,
the application records commands into command buffers and later submits those
buffers to a queue.

This separation is one of Vulkan's most important ideas. Recording prepares
work. Submission schedules that prepared work for execution.

## Command pools

Command buffers are allocated from a command pool.

A command pool belongs to a logical device and is associated with a queue
family:

```
VkCommandPoolCreateInfo poolInfo{};
poolInfo.sType =
    VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
poolInfo.queueFamilyIndex = graphicsFamily;
```

The pool is then created:

```
VkCommandPool commandPool = VK_NULL_HANDLE;

vkCreateCommandPool(
    device,
    &poolInfo,
    nullptr,
    &commandPool);
```

The queue family matters because command buffers are intended for submission to
queues from the corresponding family.

The pool manages the memory used for command buffers. It is not itself a
collection of commands.

## Allocating command buffers

A command buffer is represented by a VkCommandBuffer handle.

```
VkCommandBufferAllocateInfo allocInfo{};
allocInfo.sType =
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
allocInfo.commandPool = commandPool;
allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
allocInfo.commandBufferCount = 1;

VkCommandBuffer commandBuffer;

vkAllocateCommandBuffers(
    device,
    &allocInfo,
    &commandBuffer);
```

A primary command buffer can be submitted directly to a queue.

Secondary command buffers have a different role and can be executed from
primary command buffers. They are not needed for the basic execution model.

## Recording commands

Before commands can be recorded, the command buffer enters the recording
state:

```
VkCommandBufferBeginInfo beginInfo{};
beginInfo.sType =
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

vkBeginCommandBuffer(
    commandBuffer,
    &beginInfo);
```

Commands can now be recorded.

For example, a draw command records a request for three vertices:

```
vkCmdDraw(
    commandBuffer,
    3,
    1,
    0,
    0);
```

The command does not execute the draw immediately. It writes information into
the command buffer.

When recording is finished:

```
vkEndCommandBuffer(commandBuffer);
```

The command buffer now contains executable GPU work.

## Recording is not execution

This distinction is worth making explicit.

```
vkBeginCommandBuffer(...)
    |
    v
vkCmd...
    |
    v
vkEndCommandBuffer(...)
    |
    v
recorded commands
```

Nothing here means that the GPU has necessarily executed the commands.

The application still needs to submit the command buffer to a queue.

## Submitting work

A VkSubmitInfo describes command buffers that should be submitted:

```
VkSubmitInfo submitInfo{};
submitInfo.sType =
    VK_STRUCTURE_TYPE_SUBMIT_INFO;
submitInfo.commandBufferCount = 1;
submitInfo.pCommandBuffers = &commandBuffer;
```

The queue receives that submission:

```
vkQueueSubmit(
    graphicsQueue,
    1,
    &submitInfo,
    fence);
```

The queue now has the recorded work available for GPU execution.

The application can continue doing CPU work while the GPU processes the
submission. This asynchronous relationship is why synchronization becomes
necessary.

## One frame of work

The basic execution sequence is:

```
create command pool
    |
    v
allocate command buffer
    |
    v
begin recording
    |
    v
record commands
    |
    v
end recording
    |
    v
submit to queue
    |
    v
GPU executes
```

The command buffer is therefore a container for a sequence of GPU operations,
while the queue is the mechanism through which those operations are submitted
for execution.

## Reusing command buffers

Command buffers are normally reused rather than permanently recreated for
every frame.

A command buffer can be reset and recorded again when the previous GPU work
using it is finished.

```
vkResetCommandBuffer(
    commandBuffer,
    0);
```

This relationship becomes important when rendering multiple frames. The
application must not modify a command buffer while the GPU is still using it.

That is one of the reasons Vulkan exposes synchronization explicitly.

## Next step

Now type the code version of this lesson.

