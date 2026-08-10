# Vulkan synchronization - typing

This lesson types the synchronization path: create fences and semaphores, wait
for GPU completion, acquire an image, submit work with GPU-side dependencies,
and present the completed image.

## Create synchronization objects

A frame commonly owns a fence for CPU completion and semaphores for GPU
ordering.

```cpp
    // describe a default semaphore
    VkSemaphoreCreateInfo semaphoreInfo{
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        nullptr,
        0
    };

    // store the image-available semaphore
    VkSemaphore imageAvailable = VK_NULL_HANDLE;

    // create the image-available semaphore
    vkCreateSemaphore(
        device,
        &semaphoreInfo,
        nullptr,
        &imageAvailable
    );

    // store the render-finished semaphore
    VkSemaphore renderFinished = VK_NULL_HANDLE;

    // create the render-finished semaphore
    vkCreateSemaphore(
        device,
        &semaphoreInfo,
        nullptr,
        &renderFinished
    );
```

The two semaphores represent different GPU-side dependencies.

## Create a frame fence

The fence represents completion of submitted work from the CPU's perspective.

```cpp
    // describe the frame fence
    VkFenceCreateInfo fenceInfo{
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        nullptr,
        0
    };

    // store the frame fence
    VkFence frameFence = VK_NULL_HANDLE;

    // create the frame fence
    vkCreateFence(
        device,
        &fenceInfo,
        nullptr,
        &frameFence
    );
```

The newly created fence starts unsignaled.

## Wait for previous work

Before reusing frame-owned resources, wait for the previous submission to finish.

```cpp
    // wait until the previous frame submission finishes
    vkWaitForFences(
        device,
        1,
        &frameFence,
        VK_TRUE,
        UINT64_MAX
    );
```

The wait prevents the CPU from reusing resources that the GPU may still be
using.

## Reset the frame fence

The fence must be unsignaled before it can track the next submission.

```cpp
    // reset the fence for the next submission
    vkResetFences(
        device,
        1,
        &frameFence
    );
```

The next queue submission will signal this fence when its work completes.

## Acquire a swapchain image

The image-available semaphore receives the signal from the acquisition operation.

```cpp
    // store the acquired swapchain image index
    uint32_t imageIndex = 0;

    // acquire the next available swapchain image
    vkAcquireNextImageKHR(
        device,
        swapchain,
        UINT64_MAX,
        imageAvailable,
        VK_NULL_HANDLE,
        &imageIndex
    );
```

The graphics submission can now wait for `imageAvailable`.

## Describe the wait stage

The wait stage identifies where the semaphore dependency applies in the
submitted graphics work.

```cpp
    // wait before color attachment output begins
    VkPipelineStageFlags waitStage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
```

The stage is part of the submission's GPU-side dependency.

## Describe the graphics submission

The submission waits for image acquisition and signals completion of rendering.

```cpp
    // describe the graphics submission
    VkSubmitInfo submitInfo{
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        1,
        &imageAvailable,
        &waitStage,
        1,
        &commandBuffer,
        1,
        &renderFinished
    };
```

The submission now connects the acquired image to the recorded command buffer
and the later presentation operation.

## Submit the graphics work

The graphics queue receives the synchronized submission.

```cpp
    // submit rendering and track its completion with the fence
    vkQueueSubmit(
        graphicsQueue,
        1,
        &submitInfo,
        frameFence
    );
```

The queue waits for `imageAvailable`, executes `commandBuffer`, signals
`renderFinished`, and eventually signals `frameFence`.

The CPU does not need to wait between these GPU-side stages.

## Describe presentation

Presentation waits for the rendering-complete semaphore.

```cpp
    // describe the presentation dependency
    VkPresentInfoKHR presentInfo{
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        nullptr,
        1,
        &renderFinished,
        1,
        &swapchain,
        &imageIndex,
        nullptr
    };
```

The presentation request now waits for the graphics submission to signal
`renderFinished`.

## Present the rendered image

The presentation queue consumes the rendering-complete signal.

```cpp
    // present the completed swapchain image
    vkQueuePresentKHR(
        presentQueue,
        &presentInfo
    );
```

The basic GPU dependency is now complete:

```text
acquire -> render -> present
```

## Wait for a specific submission

A fence can be used whenever the CPU needs to know that a particular submission
has finished.

```cpp
    // wait until the submitted rendering has finished
    vkWaitForFences(
        device,
        1,
        &frameFence,
        VK_TRUE,
        UINT64_MAX
    );
```

The CPU can now safely reuse resources whose previous GPU use was associated
with this fence.

## Reset the fence for reuse

After observing completion, the same fence can track another submission.

```cpp
    // return the fence to the unsignaled state
    vkResetFences(
        device,
        1,
        &frameFence
    );
```

The fence lifecycle is therefore:

```text
submit -> signal -> wait -> reset -> submit
```

## Create a signaled fence

A renderer sometimes wants a newly created frame context to behave as though
its previous work has already completed.

A fence can be created in the signaled state.

```cpp
    // describe a fence that starts signaled
    VkFenceCreateInfo signaledFenceInfo{
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        nullptr,
        VK_FENCE_CREATE_SIGNALED_BIT
    };

    // store the initially available fence
    VkFence readyFence = VK_NULL_HANDLE;

    // create the signaled fence
    vkCreateFence(
        device,
        &signaledFenceInfo,
        nullptr,
        &readyFence
    );
```

This can simplify the first iteration of a frame loop because the initial wait
does not block on nonexistent previous GPU work.

## Establish an image dependency

Resource transitions can require synchronization inside a command buffer.

A classic image barrier describes the old and new layouts.

```cpp
    // describe the swapchain image transition
    VkImageMemoryBarrier imageBarrier{
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        nullptr,
        0,
        0,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        swapchainImages[imageIndex],
        {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            1,
            0,
            1
        }
    };
```

The barrier identifies the image and the layout transition that should occur.

## Record the image barrier

The barrier is recorded into the command buffer like other `vkCmd` operations.

```cpp
    // record the image synchronization barrier
    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &imageBarrier
    );
```

The barrier establishes an execution dependency between the source and
destination stages.

This is separate from the fence that communicates completion to the CPU.

## Wait for the whole device

A device-wide wait is useful when shutting down or performing broad resource
management.

```cpp
    // wait for all device work to finish
    vkDeviceWaitIdle(device);
```

This is intentionally broader than waiting on one frame fence.

A normal frame loop should prefer narrow dependencies so unrelated GPU work can
continue.

## Destroy the synchronization objects

Synchronization objects are Vulkan resources and must eventually be destroyed.

```cpp
    // destroy the image-available semaphore
    vkDestroySemaphore(
        device,
        imageAvailable,
        nullptr
    );

    // destroy the render-finished semaphore
    vkDestroySemaphore(
        device,
        renderFinished,
        nullptr
    );

    // destroy the frame fence
    vkDestroyFence(
        device,
        frameFence,
        nullptr
    );
```

The objects must not be destroyed while work using them is still active.

## Now type it again

Re-drill the three synchronization objects used by a frame.

```cpp
    // describe a default semaphore
    VkSemaphoreCreateInfo semaphoreInfo{
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        nullptr,
        0
    };

    // store the image-available semaphore
    VkSemaphore imageAvailable = VK_NULL_HANDLE;

    // create the image-available semaphore
    vkCreateSemaphore(
        device,
        &semaphoreInfo,
        nullptr,
        &imageAvailable
    );

    // store the render-finished semaphore
    VkSemaphore renderFinished = VK_NULL_HANDLE;

    // create the render-finished semaphore
    vkCreateSemaphore(
        device,
        &semaphoreInfo,
        nullptr,
        &renderFinished
    );

    // describe the frame fence
    VkFenceCreateInfo fenceInfo{
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        nullptr,
        0
    };

    // store the frame fence
    VkFence frameFence = VK_NULL_HANDLE;

    // create the frame fence
    vkCreateFence(
        device,
        &fenceInfo,
        nullptr,
        &frameFence
    );
```

Now drill the CPU-side frame synchronization.

```cpp
    // wait for the previous frame to finish
    vkWaitForFences(
        device,
        1,
        &frameFence,
        VK_TRUE,
        UINT64_MAX
    );

    // reset the fence for the new submission
    vkResetFences(
        device,
        1,
        &frameFence
    );
```

Now drill the acquire-submit-present chain.

```cpp
    // store the acquired image index
    uint32_t imageIndex = 0;

    // acquire a swapchain image
    vkAcquireNextImageKHR(
        device,
        swapchain,
        UINT64_MAX,
        imageAvailable,
        VK_NULL_HANDLE,
        &imageIndex
    );

    // wait before color attachment output
    VkPipelineStageFlags waitStage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    // describe the synchronized submission
    VkSubmitInfo submitInfo{
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        1,
        &imageAvailable,
        &waitStage,
        1,
        &commandBuffer,
        1,
        &renderFinished
    };

    // submit rendering and signal the frame fence
    vkQueueSubmit(
        graphicsQueue,
        1,
        &submitInfo,
        frameFence
    );

    // describe presentation
    VkPresentInfoKHR presentInfo{
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        nullptr,
        1,
        &renderFinished,
        1,
        &swapchain,
        &imageIndex,
        nullptr
    };

    // present the completed image
    vkQueuePresentKHR(
        presentQueue,
        &presentInfo
    );
```

Finally drill the distinction between CPU and GPU synchronization.

```cpp
    // wait for GPU completion from the CPU
    vkWaitForFences(
        device,
        1,
        &frameFence,
        VK_TRUE,
        UINT64_MAX
    );

    // wait for every outstanding device operation
    vkDeviceWaitIdle(device);
```

## Wrap up

```text
fence -> CPU completion
imageAvailable -> acquire to render
renderFinished -> render to present
barrier -> resource dependency
```

The synchronization layer connects command recording to safe execution. The
next lesson moves into the resources those commands operate on: buffers,
images, memory types, allocations, and resource lifetime.
````

