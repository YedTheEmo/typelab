# Vulkan first triangle - typing

This lesson types the complete first-triangle draw path: begin rendering,
bind the pipeline and vertex buffer, draw three vertices, and finish rendering.

## Prepare the vertex data

Define the three positions that make the triangle.

```cpp
    // describe one vertex position
    struct Vertex {
        float x;
        float y;
        float z;
    };

    // store three vertices for one triangle
    Vertex vertices[]{
        { 0.0f, -0.5f, 0.0f },
        { 0.5f,  0.5f, 0.0f },
        {-0.5f,  0.5f, 0.0f }
    };
```

## Begin the render operation

Describe the swapchain image that receives the triangle.

```cpp
    // describe the color attachment
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

    // describe the rendering area
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

    // begin rendering to the swapchain image
    vkCmdBeginRendering(commandBuffer, &renderingInfo);
```

## Bind the graphics pipeline

Select the pipeline that defines how the triangle is processed.

```cpp
    // bind the graphics pipeline
    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipeline
    );
```

## Set dynamic state

Provide the viewport and scissor selected for this frame.

```cpp
    // set the viewport used by the draw
    vkCmdSetViewport(
        commandBuffer,
        0,
        1,
        &viewport
    );

    // set the scissor used by the draw
    vkCmdSetScissor(
        commandBuffer,
        0,
        1,
        &scissor
    );
```

## Bind the vertex buffer

Select the buffer containing the triangle's three vertices.

```cpp
    // start reading vertices at the beginning of the buffer
    VkDeviceSize offset = 0;

    // bind the triangle vertex buffer
    vkCmdBindVertexBuffers(
        commandBuffer,
        0,
        1,
        &vertexBuffer,
        &offset
    );
```

## Draw the triangle

Request one triangle by processing three vertices.

```cpp
    // draw three vertices as one triangle
    vkCmdDraw(
        commandBuffer,
        3,
        1,
        0,
        0
    );
```

## End rendering

Close the rendering operation after the draw has been recorded.

```cpp
    // finish the rendering operation
    vkCmdEndRendering(commandBuffer);
```

## Prepare presentation

The rendered image must be transitioned into the state expected by the
presentation engine.

```cpp
    // describe the image transition
    VkImageMemoryBarrier2 presentBarrier{
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        nullptr,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_2_NONE,
        VK_ACCESS_2_NONE,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        queueFamily,
        queueFamily,
        colorImage,
        {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            1,
            0,
            1
        }
    };

    // describe the dependency information
    VkDependencyInfo dependencyInfo{
        VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        nullptr,
        0,
        0,
        nullptr,
        1,
        &presentBarrier,
        0,
        nullptr
    };

    // transition the image for presentation
    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
```

## Submit the frame

Submit the command buffer so the GPU can execute the recorded draw.

```cpp
    // describe the command buffer submission
    VkCommandBufferSubmitInfo commandInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        nullptr,
        commandBuffer,
        0
    };

    // describe the signal operation
    VkSemaphoreSubmitInfo signalInfo{
        VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        nullptr,
        renderFinishedSemaphore,
        0,
        VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        0
    };

    // describe the complete submission
    VkSubmitInfo2 submitInfo{
        VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        nullptr,
        0,
        0,
        nullptr,
        1,
        &commandInfo,
        1,
        &signalInfo
    };

    // submit the recorded triangle
    vkQueueSubmit2(
        graphicsQueue,
        1,
        &submitInfo,
        frameFence
    );
```

## Present the image

Give the completed swapchain image to the presentation queue.

```cpp
    // describe the swapchain image to present
    VkPresentInfoKHR presentInfo{
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        nullptr,
        1,
        &renderFinishedSemaphore,
        1,
        &swapchain,
        &imageIndex,
        nullptr
    };

    // present the rendered image
    vkQueuePresentKHR(
        presentQueue,
        &presentInfo
    );
```

## Now type it again

Re-drill the core rendering sequence without the setup explanations.

```cpp
    // describe the color attachment
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

    // describe the rendering operation
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

    // begin rendering
    vkCmdBeginRendering(commandBuffer, &renderingInfo);

    // bind the graphics pipeline
    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipeline
    );

    // bind the triangle vertex buffer
    vkCmdBindVertexBuffers(
        commandBuffer,
        0,
        1,
        &vertexBuffer,
        &offset
    );

    // draw the three triangle vertices
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    // finish rendering
    vkCmdEndRendering(commandBuffer);
```

Re-drill the final image transition and presentation.

```cpp
    // describe the image transition
    VkImageMemoryBarrier2 presentBarrier{
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        nullptr,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_2_NONE,
        VK_ACCESS_2_NONE,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        queueFamily,
        queueFamily,
        colorImage,
        {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            1,
            0,
            1
        }
    };

    // describe the transition dependency
    VkDependencyInfo dependencyInfo{
        VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        nullptr,
        0,
        0,
        nullptr,
        1,
        &presentBarrier,
        0,
        nullptr
    };

    // transition the image to presentation layout
    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);

    // submit the recorded command buffer
    vkQueueSubmit2(
        graphicsQueue,
        1,
        &submitInfo,
        frameFence
    );

    // present the completed swapchain image
    vkQueuePresentKHR(
        presentQueue,
        &presentInfo
    );
```

## Wrap up

```text
acquire -> begin -> bind pipeline -> bind vertices -> draw -> end -> submit -> present
```
