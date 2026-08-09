# Vulkan synchronization - typing

This lesson types GPU and CPU synchronization: a fence, two semaphores, and
the acquire-render-present chain they wire together.

## Create synchronization objects

A fence tracks CPU-visible completion; semaphores track GPU-side order.

```
// the create-info struct for the fence
VkFenceCreateInfo fenceInfo{};
// identify the fence create-info type
fenceInfo.sType =
    VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

// handle that Vulkan will fill in
VkFence inFlightFence = VK_NULL_HANDLE;

// create the fence (starts unsignaled)
VkResult result = vkCreateFence(
    device,
    &fenceInfo,
    nullptr,
    &inFlightFence);

// bail out if fence creation failed
if (result != VK_SUCCESS)
    return 1;

// the create-info struct for a semaphore
VkSemaphoreCreateInfo semaphoreInfo{};
// identify the semaphore create-info type
semaphoreInfo.sType =
    VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

// signals when the swapchain image is ready
VkSemaphore imageAvailable = VK_NULL_HANDLE;

// create the first semaphore
result = vkCreateSemaphore(
    device,
    &semaphoreInfo,
    nullptr,
    &imageAvailable);

// bail out if semaphore creation failed
if (result != VK_SUCCESS)
    return 1;

// signals when rendering has finished
VkSemaphore renderFinished = VK_NULL_HANDLE;

// create the second semaphore
result = vkCreateSemaphore(
    device,
    &semaphoreInfo,
    nullptr,
    &renderFinished);

// bail out if semaphore creation failed
if (result != VK_SUCCESS)
    return 1;
```

## Wait for the previous frame

Wait before reusing resources tied to the prior submission.

```
// block the CPU until the fence is signaled
vkWaitForFences(
    device,
    1,
    &inFlightFence,
    VK_TRUE,      // wait for all listed fences
    UINT64_MAX);  // wait forever

// prepare the fence for the next submission
vkResetFences(
    device,
    1,
    &inFlightFence);
```

## Acquire a swapchain image

The semaphore is signaled by the GPU when the image is ready.

```
// index of the image handed to us
uint32_t imageIndex = 0;

// acquire an image and signal imageAvailable when ready
result = vkAcquireNextImageKHR(
    device,
    swapchain,
    UINT64_MAX,   // wait forever for an image
    imageAvailable,
    VK_NULL_HANDLE,
    &imageIndex);
```

## Describe the wait

The submission must wait for the image before drawing into it.

```
// only the color attachment stage needs to wait
VkPipelineStageFlags waitStage =
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

// the create-info struct for the submission
VkSubmitInfo submitInfo{};
// identify the submit-info type
submitInfo.sType =
    VK_STRUCTURE_TYPE_SUBMIT_INFO;
// wait on one semaphore
submitInfo.waitSemaphoreCount = 1;
// the semaphore to wait on
submitInfo.pWaitSemaphores = &imageAvailable;
// the stage that must wait
submitInfo.pWaitDstStageMask = &waitStage;

// submit one command buffer
submitInfo.commandBufferCount = 1;
// pointer to the recorded buffer
submitInfo.pCommandBuffers = &commandBuffer;
```

## Signal when rendering finishes

The same submission signals renderFinished when its work completes.

```
// signal one semaphore when done
submitInfo.signalSemaphoreCount = 1;
// the semaphore to signal
submitInfo.pSignalSemaphores = &renderFinished;

// submit and associate the fence with this work
result = vkQueueSubmit(
    graphicsQueue,
    1,
    &submitInfo,
    inFlightFence);

// bail out if the submission failed
if (result != VK_SUCCESS)
    return 1;
```

## Present the image

Presentation waits for renderFinished before showing the image.

```
// the create-info struct for presentation
VkPresentInfoKHR presentInfo{};
// identify the present-info type
presentInfo.sType =
    VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
// wait for rendering before presenting
presentInfo.waitSemaphoreCount = 1;
// the semaphore that says rendering is done
presentInfo.pWaitSemaphores = &renderFinished;
// present to one swapchain
presentInfo.swapchainCount = 1;
// the swapchain to present to
presentInfo.pSwapchains = &swapchain;
// which image to present
presentInfo.pImageIndices = &imageIndex;

// request the presentation
vkQueuePresentKHR(
    presentQueue,
    &presentInfo);
```

## Reuse the fence

Each frame starts by waiting for and resetting the same fence.

```
// block until the previous submission completes
vkWaitForFences(
    device,
    1,
    &inFlightFence,
    VK_TRUE,
    UINT64_MAX);

// prepare the fence for the next submission
vkResetFences(
    device,
    1,
    &inFlightFence);
```

## Clean up

Synchronization objects belong to the logical device.

```
// destroy the image-available semaphore
vkDestroySemaphore(
    device,
    imageAvailable,
    nullptr);

// destroy the render-finished semaphore
vkDestroySemaphore(
    device,
    renderFinished,
    nullptr);

// destroy the fence
vkDestroyFence(
    device,
    inFlightFence,
    nullptr);
```

## Now type it again

Type the CPU-side wait and reset first.

```
// block until the previous submission completes
vkWaitForFences(
    device,
    1,
    &inFlightFence,
    VK_TRUE,
    UINT64_MAX);

// prepare the fence for the next submission
vkResetFences(
    device,
    1,
    &inFlightFence);
```

Then type the GPU-side wait.

```
// only the color attachment stage needs to wait
VkPipelineStageFlags waitStage =
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

// wait on one semaphore
submitInfo.waitSemaphoreCount = 1;
// the semaphore to wait on
submitInfo.pWaitSemaphores = &imageAvailable;
// the stage that must wait
submitInfo.pWaitDstStageMask = &waitStage;
```

Finally, type the signal.

```
// signal one semaphore when done
submitInfo.signalSemaphoreCount = 1;
// the semaphore to signal
submitInfo.pSignalSemaphores = &renderFinished;
```

## Wrap up

The flow: acquire -> wait semaphore -> render -> signal semaphore -> present.
The fence separately tells the CPU when the submitted GPU work is finished.
