# Vulkan synchronization - concepts

Vulkan work is asynchronous. When the CPU submits commands to a queue, the GPU
may execute those commands later.

The CPU and GPU can therefore be working at the same time. This is useful for
performance, but it creates a problem: one operation may depend on another
operation finishing first.

Vulkan makes these dependencies explicit through synchronization primitives.

## Fences synchronize the CPU

A fence is primarily used when the CPU needs to know that GPU work has finished.

Create one with:

```
VkFenceCreateInfo fenceInfo{};
fenceInfo.sType =
    VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

VkFence fence = VK_NULL_HANDLE;

vkCreateFence(
    device,
    &fenceInfo,
    nullptr,
    &fence);
```

Pass it to a queue submission:

```
vkQueueSubmit(
    graphicsQueue,
    1,
    &submitInfo,
    fence);
```

The GPU eventually signals the fence after the submitted work completes.

The CPU can wait for that signal:

```
vkWaitForFences(
    device,
    1,
    &fence,
    VK_TRUE,
    UINT64_MAX);
```

This creates a synchronization relationship between the CPU and GPU:

```
CPU
 |
 | submit
 v
queue
 |
 v
GPU
 |
 | signal fence
 v
CPU continues
```

A fence is therefore useful when the CPU needs to wait for GPU completion.

## Fences can be reused

After a fence has been signaled, it remains signaled until reset.

```
vkResetFences(
    device,
    1,
    &fence);
```

A common frame loop waits for a fence, resets it, and submits new work using
the same fence.

The important state transition is:

```
unsignaled -> GPU completes -> signaled
                                  |
                                  v
                                reset
                                  |
                                  v
                              unsignaled
```

The CPU controls this synchronization.

## Semaphores synchronize GPU work

A semaphore serves a different purpose.

Instead of making the CPU wait, a semaphore allows one GPU operation to signal
a condition that another GPU operation waits for.

For example, acquiring a swapchain image can signal a semaphore:

```
vkAcquireNextImageKHR(
    device,
    swapchain,
    UINT64_MAX,
    imageAvailable,
    VK_NULL_HANDLE,
    &imageIndex);
```

The semaphore can then be attached to a queue submission.

The GPU waits for that semaphore before executing the submitted commands.

This creates a GPU-to-GPU dependency:

```
acquire image
      |
      | signal
      v
imageAvailable
      |
      | wait
      v
graphics submission
```

The CPU does not need to wait between these operations.

## Waiting at a pipeline stage

A submission specifies where the GPU should wait for a semaphore.

```
VkPipelineStageFlags waitStage =
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
```

The wait stage says that the submission must not proceed into the specified
pipeline stage until the semaphore has been signaled.

The command buffer can therefore be submitted with:

```
submitInfo.waitSemaphoreCount = 1;
submitInfo.pWaitSemaphores = &imageAvailable;
submitInfo.pWaitDstStageMask = &waitStage;
```

This is more precise than simply saying "wait before rendering." Vulkan asks
the application to describe where the dependency matters.

## Rendering and presentation

A typical frame uses synchronization in both directions.

First, acquiring an image signals imageAvailable.

```
acquire
   |
   v
imageAvailable
   |
   v
graphics submission
```

The rendering submission can then signal another semaphore when its work is
complete:

```
VkSemaphore renderFinished;
```

The presentation operation waits for that semaphore:

```
render
   |
   | signal
   v
renderFinished
   |
   v
present
```

The resulting frame flow is:

```
acquire
   |
   v
imageAvailable
   |
   v
render
   |
   v
renderFinished
   |
   v
present
```

A fence can additionally tell the CPU that the rendering submission has
finished.

## The three roles

The easiest way to remember the distinction is to ask who needs to wait.

A fence lets the CPU wait for the GPU:

```
GPU -> fence -> CPU
```

A semaphore lets GPU operations wait for other GPU operations:

```
GPU -> semaphore -> GPU
```

A pipeline stage mask specifies where a semaphore dependency applies within
the receiving submission.

These mechanisms are complementary rather than interchangeable.

## Why synchronization matters

Without synchronization, the application could submit work that reads an image
before the previous operation has finished writing it, or present an image
before rendering has completed.

Vulkan does not silently insert all of these dependencies for the application.

The programmer describes the ordering explicitly so the driver and GPU can
execute independent work without unnecessary waiting.

## Next step

Now type the code version of this lesson.

