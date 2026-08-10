# Vulkan frame loop and renderer architecture - typing

This lesson types a practical frame loop: wait, acquire, record, submit,
present, and advance through per-frame renderer state.

## Define a frame context

Group the command and synchronization objects reused by one frame slot.

```cpp
    // store resources owned by one frame slot
    struct FrameContext {
        VkCommandBuffer commandBuffer;
        VkSemaphore imageAvailable;
        VkSemaphore renderFinished;
        VkFence inFlightFence;
    };

    // choose how many frames may overlap
    constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    // store the current frame slot
    uint32_t currentFrame = 0;
```

## Wait for the frame

Do not reuse a frame context while its previous GPU submission is running.

```cpp
    // wait until this frame's previous submission has completed
    vkWaitForFences(
        device,
        1,
        &frames[currentFrame].inFlightFence,
        VK_TRUE,
        UINT64_MAX
    );

    // reset the fence for the next submission
    vkResetFences(
        device,
        1,
        &frames[currentFrame].inFlightFence
    );
```

## Acquire a swapchain image

Request an available presentation image.

```cpp
    // store the acquired swapchain image index
    uint32_t imageIndex = 0;

    // acquire an image and signal the frame's semaphore
    VkResult acquireResult = vkAcquireNextImageKHR(
        device,
        swapchain,
        UINT64_MAX,
        frames[currentFrame].imageAvailable,
        VK_NULL_HANDLE,
        &imageIndex
    );
```

## Wait for image ownership

Make sure another frame is not still using the acquired image.

```cpp
    // find the fence currently associated with this image
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        // wait until the previous image owner has finished
        vkWaitForFences(
            device,
            1,
            &imagesInFlight[imageIndex],
            VK_TRUE,
            UINT64_MAX
        );
    }

    // assign the image to the current frame
    imagesInFlight[imageIndex] =
        frames[currentFrame].inFlightFence;
```

## Reset the command buffer

Prepare the frame's command buffer for new recording.

```cpp
    // reset the command buffer from the previous frame
    vkResetCommandBuffer(
        frames[currentFrame].commandBuffer,
        0
    );

    // describe command buffer recording
    VkCommandBufferBeginInfo beginInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        0,
        nullptr
    };

    // begin recording this frame's commands
    vkBeginCommandBuffer(
        frames[currentFrame].commandBuffer,
        &beginInfo
    );
```

## Begin rendering

Record the pipeline, resources, attachments, and draw.

```cpp
    // store the frame command buffer
    VkCommandBuffer commandBuffer =
        frames[currentFrame].commandBuffer;

    // bind the graphics pipeline
    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipeline
    );

    // bind descriptor resources
    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        0,
        1,
        &descriptorSet,
        0,
        nullptr
    );

    // bind the vertex buffer
    VkDeviceSize vertexOffset = 0;

    // attach the vertex buffer to the command buffer
    vkCmdBindVertexBuffers(
        commandBuffer,
        0,
        1,
        &vertexBuffer,
        &vertexOffset
    );
```

## Draw with dynamic rendering

Describe the current swapchain image and depth attachment.

```cpp
    // describe the current color attachment
    VkRenderingAttachmentInfo colorAttachment{
        VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        nullptr,
        swapchainImageViews[imageIndex],
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_RESOLVE_MODE_NONE,
        VK_NULL_HANDLE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        colorClear
    };

    // describe the current depth attachment
    VkRenderingAttachmentInfo depthAttachment{
        VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        nullptr,
        depthView,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        VK_RESOLVE_MODE_NONE,
        VK_NULL_HANDLE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        depthClear
    };

    // describe the active rendering area
    VkRenderingInfo renderingInfo{
        VK_STRUCTURE_TYPE_RENDERING_INFO,
        nullptr,
        0,
        {{0, 0}, {width, height}},
        1,
        0,
        &colorAttachment,
        &depthAttachment,
        nullptr
    };

    // begin the frame's rendering operation
    vkCmdBeginRendering(
        commandBuffer,
        &renderingInfo
    );

    // draw the frame's geometry
    vkCmdDraw(
        commandBuffer,
        vertexCount,
        1,
        0,
        0
    );

    // finish the rendering operation
    vkCmdEndRendering(
        commandBuffer
    );
```

## Finish recording

Close the command buffer so it can be submitted to a queue.

```cpp
    // finish recording the frame
    vkEndCommandBuffer(
        commandBuffer
    );
```

## Submit the frame

Wait for acquisition, execute the command buffer, then signal completion.

```cpp
    // choose the stage that waits for image acquisition
    VkPipelineStageFlags waitStage =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    // describe the graphics submission
    VkSubmitInfo submitInfo{
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        nullptr,
        1,
        &frames[currentFrame].imageAvailable,
        &waitStage,
        1,
        &commandBuffer,
        1,
        &frames[currentFrame].renderFinished
    };

    // submit the frame to the graphics queue
    vkQueueSubmit(
        graphicsQueue,
        1,
        &submitInfo,
        frames[currentFrame].inFlightFence
    );
```

## Present the image

Wait for rendering to finish before presenting the swapchain image.

```cpp
    // describe the presentation request
    VkPresentInfoKHR presentInfo{
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        nullptr,
        1,
        &frames[currentFrame].renderFinished,
        1,
        &swapchain,
        &imageIndex,
        nullptr
    };

    // present the rendered swapchain image
    VkResult presentResult =
        vkQueuePresentKHR(
            presentQueue,
            &presentInfo
        );
```

## Advance the frame

Move to the next frame context.

```cpp
    // advance to the next reusable frame slot
    currentFrame =
        (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
```

## Handle swapchain changes

Acquisition and presentation can indicate that the swapchain must change.

```cpp
    // check whether the swapchain is no longer current
    if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR ||
        presentResult == VK_ERROR_OUT_OF_DATE_KHR ||
        presentResult == VK_SUBOPTIMAL_KHR) {
        // rebuild swapchain-dependent resources
        recreateSwapchain();
    }
```

## Put the loop behind one function

Keep the application-facing renderer operation small.

```cpp
    // render one complete frame
    void drawFrame() {
        // wait for the reusable frame slot
        waitForFrame();

        // acquire the presentation image
        acquireImage();

        // record GPU commands
        recordCommands();

        // submit the recorded work
        submitFrame();

        // present the completed image
        presentFrame();

        // move to the next frame slot
        advanceFrame();
    }
```

## Recreate swapchain-dependent state

Keep device-wide resources alive while rebuilding presentation resources.

```cpp
    // wait until no GPU work references swapchain resources
    vkDeviceWaitIdle(device);

    // destroy old swapchain-dependent resources
    cleanupSwapchain();

    // create the new swapchain
    createSwapchain();

    // create views for the new swapchain images
    createSwapchainImageViews();

    // recreate the depth attachment
    createDepthResources();

    // recreate resources that depend on the new extent
    createFramebuffers();
```

## Create frame synchronization

Give every frame slot its own synchronization objects.

```cpp
    // describe a semaphore
    VkSemaphoreCreateInfo semaphoreInfo{
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        nullptr,
        0
    };

    // describe a fence that starts signaled
    VkFenceCreateInfo fenceInfo{
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        nullptr,
        VK_FENCE_CREATE_SIGNALED_BIT
    };

    // create synchronization for each frame
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        // create the image-available semaphore
        vkCreateSemaphore(
            device,
            &semaphoreInfo,
            nullptr,
            &frames[i].imageAvailable
        );

        // create the render-finished semaphore
        vkCreateSemaphore(
            device,
            &semaphoreInfo,
            nullptr,
            &frames[i].renderFinished
        );

        // create the frame completion fence
        vkCreateFence(
            device,
            &fenceInfo,
            nullptr,
            &frames[i].inFlightFence
        );
    }
```

## Track swapchain image ownership

Associate each acquired image with the fence of the submission using it.

```cpp
    // create one ownership slot for every swapchain image
    std::vector<VkFence> imagesInFlight(
        swapchainImageCount,
        VK_NULL_HANDLE
    );

    // assign the acquired image to this frame's fence
    imagesInFlight[imageIndex] =
        frames[currentFrame].inFlightFence;
```

## Now type it again

Re-drill the central frame sequence.

```cpp
    // wait for the previous use of this frame slot
    vkWaitForFences(
        device,
        1,
        &frames[currentFrame].inFlightFence,
        VK_TRUE,
        UINT64_MAX
    );

    // acquire the next presentation image
    uint32_t imageIndex = 0;

    // request a swapchain image
    vkAcquireNextImageKHR(
        device,
        swapchain,
        UINT64_MAX,
        frames[currentFrame].imageAvailable,
        VK_NULL_HANDLE,
        &imageIndex
    );

    // reset the reusable command buffer
    vkResetCommandBuffer(
        frames[currentFrame].commandBuffer,
        0
    );

    // begin command recording
    vkBeginCommandBuffer(
        frames[currentFrame].commandBuffer,
        &beginInfo
    );

    // record the rendering commands
    recordCommands();

    // finish command recording
    vkEndCommandBuffer(
        frames[currentFrame].commandBuffer
    );
```

Re-drill submission, presentation, and frame advancement.

```cpp
    // submit the recorded command buffer
    vkQueueSubmit(
        graphicsQueue,
        1,
        &submitInfo,
        frames[currentFrame].inFlightFence
    );

    // present after rendering has signaled completion
    vkQueuePresentKHR(
        presentQueue,
        &presentInfo
    );

    // advance to the next frame slot
    currentFrame =
        (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
```

## Wrap up

```text
wait -> acquire -> record -> submit -> present -> advance
```
