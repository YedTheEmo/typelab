# Vulkan command buffers - typing

This lesson types the command recording path: create a command pool, allocate
a primary command buffer, record rendering commands, end recording, and submit
the finished buffer to a queue.

## Create the command pool

The command pool provides command-buffer storage for a queue family.

```cpp
    // describe the command pool
    VkCommandPoolCreateInfo poolInfo{
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        0,
        graphicsFamily
    };

    // store the command pool handle
    VkCommandPool commandPool = VK_NULL_HANDLE;

    // create the command pool
    vkCreateCommandPool(
        device,
        &poolInfo,
        nullptr,
        &commandPool
    );
```

The pool is associated with the graphics queue family selected during device
creation.

## Allocate a command buffer

The primary command buffer can be submitted directly to the graphics queue.

```cpp
    // describe the command-buffer allocation
    VkCommandBufferAllocateInfo allocateInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1
    };

    // store the command buffer handle
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    // allocate the command buffer
    vkAllocateCommandBuffers(
        device,
        &allocateInfo,
        &commandBuffer
    );
```

The command buffer now exists, but it is not recording commands yet.

## Begin recording

Beginning recording changes the command buffer into its recording state.

```cpp
    // describe how command recording should begin
    VkCommandBufferBeginInfo beginInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        0,
        nullptr
    };

    // begin recording commands
    vkBeginCommandBuffer(
        commandBuffer,
        &beginInfo
    );
```

Commands can now be appended to the buffer.

## Set the viewport

The viewport converts normalized or framebuffer coordinates into the target
window dimensions.

```cpp
    // describe the viewport dimensions
    VkViewport viewport{
        0.0f,
        0.0f,
        width,
        height,
        0.0f,
        1.0f
    };

    // record the viewport state
    vkCmdSetViewport(
        commandBuffer,
        0,
        1,
        &viewport
    );
```

The call records the viewport operation; it does not execute it immediately.

## Set the scissor

The scissor rectangle limits which framebuffer pixels can be affected.

```cpp
    // describe the scissor rectangle
    VkRect2D scissor{
        {0, 0},
        {width, height}
    };

    // record the scissor state
    vkCmdSetScissor(
        commandBuffer,
        0,
        1,
        &scissor
    );
```

The viewport and scissor are now part of the recorded command sequence.

## Begin rendering

Rendering commands need a rendering context that identifies their attachments.

```cpp
    // describe the rendering region
    VkRenderingInfo renderingInfo{
        VK_STRUCTURE_TYPE_RENDERING_INFO,
        nullptr,
        0,
        renderArea,
        1,
        1,
        0,
        colorAttachment
    };

    // begin the rendering region
    vkCmdBeginRendering(
        commandBuffer,
        &renderingInfo
    );
```

The exact attachment setup belongs to later lessons, so the object here
represents the rendering information already prepared by the renderer.

## Bind the graphics pipeline

The graphics pipeline determines how the GPU processes the draw.

```cpp
    // record the graphics pipeline binding
    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipeline
    );
```

The pipeline is now active for subsequent graphics commands.

## Bind resources

Resources such as buffers and textures are connected to the pipeline through
descriptor sets.

```cpp
    // record the descriptor set binding
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
```

The descriptor system will be developed in detail later in the track.

## Record a draw

A draw command tells the GPU to execute the currently bound graphics state.

```cpp
    // record a triangle draw
    vkCmdDraw(
        commandBuffer,
        3,
        1,
        0,
        0
    );
```

The command is now part of the command buffer and will execute only when the
buffer is submitted.

## End rendering

The rendering region must be closed after its draw commands.

```cpp
    // finish the rendering region
    vkCmdEndRendering(commandBuffer);
```

The command buffer can now continue with commands outside the rendering region
or finish recording entirely.

## End command recording

Ending recording turns the command buffer into an executable command sequence.

```cpp
    // finish recording the command buffer
    vkEndCommandBuffer(commandBuffer);
```

The recorded commands have not necessarily executed yet.

The buffer is now ready to be submitted to the graphics queue.

## Describe the queue submission

A submission structure identifies which command buffers should execute.

```cpp
    // describe the command-buffer submission
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

The submission structure is a CPU-side description of work being handed to the
queue.

## Submit the command buffer

The queue receives the completed command buffer.

```cpp
    // submit the recorded command buffer
    vkQueueSubmit(
        graphicsQueue,
        1,
        &submitInfo,
        fence
    );
```

This is the boundary between recording GPU work and scheduling that work for
execution.

The `fence` represents a synchronization object that will be explained in the
next lesson.

## Wait for completion

For this isolated demonstration, the CPU can wait until the submitted work has
finished.

```cpp
    // wait until the submitted work finishes
    vkWaitForFences(
        device,
        1,
        &fence,
        VK_TRUE,
        UINT64_MAX
    );
```

A real renderer does not normally wait for every submission immediately.
Doing so would unnecessarily serialize CPU and GPU work.

The synchronization lesson will replace this simple waiting model with a
proper frame-in-flight design.

## Reset the command buffer

After the GPU has finished using the command buffer, it can be reset for reuse.

```cpp
    // reset the command buffer for another recording
    vkResetCommandBuffer(
        commandBuffer,
        0
    );
```

Resetting clears its recorded command sequence.

The application must establish that the GPU is finished with the buffer before
performing this operation.

## Record the buffer again

A reset command buffer can return to the recording state.

```cpp
    // begin recording the reusable command buffer
    vkBeginCommandBuffer(
        commandBuffer,
        &beginInfo
    );

    // record the viewport state again
    vkCmdSetViewport(
        commandBuffer,
        0,
        1,
        &viewport
    );

    // record the scissor state again
    vkCmdSetScissor(
        commandBuffer,
        0,
        1,
        &scissor
    );

    // finish recording the reusable command buffer
    vkEndCommandBuffer(commandBuffer);
```

The same command-buffer object can therefore describe a new sequence of work.

## Free the command buffer

When the command buffer is no longer needed, it can be returned to the pool.

```cpp
    // free the command buffer from its pool
    vkFreeCommandBuffers(
        device,
        commandPool,
        1,
        &commandBuffer
    );
```

The command pool still exists after the individual buffer is freed.

## Destroy the command pool

The command pool can finally be destroyed when its buffers are no longer used.

```cpp
    // destroy the command pool
    vkDestroyCommandPool(
        device,
        commandPool,
        nullptr
    );
```

Destroying the pool releases the allocation resources associated with it.

## Now type it again

Re-drill the command-buffer creation and recording lifecycle.

```cpp
    // describe the command pool
    VkCommandPoolCreateInfo poolInfo{
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        nullptr,
        0,
        graphicsFamily
    };

    // store the command pool
    VkCommandPool commandPool = VK_NULL_HANDLE;

    // create the command pool
    vkCreateCommandPool(
        device,
        &poolInfo,
        nullptr,
        &commandPool
    );

    // describe the command-buffer allocation
    VkCommandBufferAllocateInfo allocateInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1
    };

    // store the command buffer
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    // allocate the command buffer
    vkAllocateCommandBuffers(
        device,
        &allocateInfo,
        &commandBuffer
    );
```

Now drill the recording and submission boundary.

```cpp
    // describe how recording begins
    VkCommandBufferBeginInfo beginInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        0,
        nullptr
    };

    // begin recording
    vkBeginCommandBuffer(
        commandBuffer,
        &beginInfo
    );

    // record the graphics pipeline binding
    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        graphicsPipeline
    );

    // record a triangle draw
    vkCmdDraw(
        commandBuffer,
        3,
        1,
        0,
        0
    );

    // finish recording
    vkEndCommandBuffer(commandBuffer);

    // describe the command-buffer submission
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

    // submit the recorded work
    vkQueueSubmit(
        graphicsQueue,
        1,
        &submitInfo,
        fence
    );
```

Finally drill the reuse lifecycle.

```cpp
    // wait until the GPU has finished the command buffer
    vkWaitForFences(
        device,
        1,
        &fence,
        VK_TRUE,
        UINT64_MAX
    );

    // reset the command buffer
    vkResetCommandBuffer(
        commandBuffer,
        0
    );

    // free the command buffer
    vkFreeCommandBuffers(
        device,
        commandPool,
        1,
        &commandBuffer
    );

    // destroy the command pool
    vkDestroyCommandPool(
        device,
        commandPool,
        nullptr
    );
```

## Wrap up

```text
queue family -> command pool -> allocate -> begin -> record -> end -> submit
```

The command buffer records GPU work, while the queue provides the path that
eventually executes that recorded work. The next lesson explains how Vulkan
controls the timing between those operations.
````

