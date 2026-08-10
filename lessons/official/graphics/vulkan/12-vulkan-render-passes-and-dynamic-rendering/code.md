# Vulkan render passes and dynamic rendering - typing

This lesson types both attachment models: a traditional render pass path and a
modern dynamic-rendering path using the same color and depth images.

## Describe the traditional attachments

Define the attachment descriptions used by a traditional render pass.

```cpp
    // describe the color attachment
    VkAttachmentDescription colorAttachment{
        0,
        swapchainFormat,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    // describe the depth attachment
    VkAttachmentDescription depthAttachment{
        0,
        depthFormat,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    // group the attachment descriptions
    VkAttachmentDescription attachments[]{
        colorAttachment,
        depthAttachment
    };
```

## Reference the attachments

Connect the subpass to the color and depth attachment slots.

```cpp
    // reference the color attachment
    VkAttachmentReference colorReference{
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    // reference the depth attachment
    VkAttachmentReference depthReference{
        1,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    // describe one graphics subpass
    VkSubpassDescription subpass{
        0,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        0,
        nullptr,
        1,
        &colorReference,
        nullptr,
        &depthReference,
        0,
        nullptr
    };
```

## Create the render pass

Create the static rendering description.

```cpp
    // describe the traditional render pass
    VkRenderPassCreateInfo renderPassInfo{
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        nullptr,
        0,
        2,
        attachments,
        1,
        &subpass,
        0,
        nullptr
    };

    // store the traditional render pass
    VkRenderPass renderPass = VK_NULL_HANDLE;

    // create the traditional render pass
    vkCreateRenderPass(
        device,
        &renderPassInfo,
        nullptr,
        &renderPass
    );
```

## Create the framebuffer

Provide the concrete color and depth views for one framebuffer.

```cpp
    // group the framebuffer image views
    VkImageView framebufferAttachments[]{
        swapchainImageView,
        depthView
    };

    // describe the framebuffer
    VkFramebufferCreateInfo framebufferInfo{
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        nullptr,
        0,
        renderPass,
        2,
        framebufferAttachments,
        width,
        height,
        1
    };

    // store the framebuffer
    VkFramebuffer framebuffer = VK_NULL_HANDLE;

    // create the framebuffer
    vkCreateFramebuffer(
        device,
        &framebufferInfo,
        nullptr,
        &framebuffer
    );
```

## Begin the traditional pass

Clear the attachments and execute the draw inside the render pass.

```cpp
    // describe the color clear
    VkClearValue colorClear{};
    colorClear.color = {{0.05f, 0.05f, 0.08f, 1.0f}};

    // describe the depth clear
    VkClearValue depthClear{};
    depthClear.depthStencil = {1.0f, 0};

    // group the clear values
    VkClearValue clearValues[]{
        colorClear,
        depthClear
    };

    // describe the render pass begin operation
    VkRenderPassBeginInfo beginInfo{
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        nullptr,
        renderPass,
        framebuffer,
        {{0, 0}, {width, height}},
        2,
        clearValues
    };

    // begin the traditional render pass
    vkCmdBeginRenderPass(
        commandBuffer,
        &beginInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    // draw geometry inside the pass
    vkCmdDraw(
        commandBuffer,
        vertexCount,
        1,
        0,
        0
    );

    // finish the traditional render pass
    vkCmdEndRenderPass(
        commandBuffer
    );
```

## Describe dynamic color rendering

Describe the actual color image directly.

```cpp
    // describe the dynamic color attachment
    VkRenderingAttachmentInfo dynamicColor{
        VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        nullptr,
        colorView,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_RESOLVE_MODE_NONE,
        VK_NULL_HANDLE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        colorClear
    };
```

## Describe dynamic depth rendering

Describe the depth image directly beside the color attachment.

```cpp
    // describe the dynamic depth attachment
    VkRenderingAttachmentInfo dynamicDepth{
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
```

## Begin dynamic rendering

Point the rendering command directly at its active attachments.

```cpp
    // describe the dynamic rendering operation
    VkRenderingInfo renderingInfo{
        VK_STRUCTURE_TYPE_RENDERING_INFO,
        nullptr,
        0,
        {{0, 0}, {width, height}},
        1,
        0,
        &dynamicColor,
        &dynamicDepth,
        nullptr
    };

    // begin dynamic rendering
    vkCmdBeginRendering(
        commandBuffer,
        &renderingInfo
    );

    // draw geometry during dynamic rendering
    vkCmdDraw(
        commandBuffer,
        vertexCount,
        1,
        0,
        0
    );

    // finish dynamic rendering
    vkCmdEndRendering(
        commandBuffer
    );
```

## Describe dynamic pipeline formats

Tell pipeline creation which attachment formats dynamic rendering expects.

```cpp
    // describe the formats used by dynamic rendering
    VkPipelineRenderingCreateInfo renderingFormats{
        VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        nullptr,
        0,
        1,
        &swapchainFormat,
        depthFormat,
        VK_FORMAT_UNDEFINED
    };

    // connect the rendering formats to pipeline creation
    VkPipelineCreateFlags pipelineFlags = 0;
```

## Add a dependency

Traditional render passes can explicitly describe an external dependency.

```cpp
    // describe the external color dependency
    VkSubpassDependency dependency{
        VK_SUBPASS_EXTERNAL,
        0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        0
    };
```

## Compare the two command paths

The command structures make the architectural difference visible.

```cpp
    // traditional rendering starts with a render pass
    vkCmdBeginRenderPass(
        commandBuffer,
        &beginInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    // traditional rendering issues normal draw commands
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);

    // traditional rendering ends with the render pass
    vkCmdEndRenderPass(commandBuffer);

    // dynamic rendering starts with attachment information
    vkCmdBeginRendering(commandBuffer, &renderingInfo);

    // dynamic rendering also issues normal draw commands
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);

    // dynamic rendering ends directly
    vkCmdEndRendering(commandBuffer);
```

## Destroy traditional objects

Traditional rendering objects can be destroyed after GPU use finishes.

```cpp
    // destroy the framebuffer
    vkDestroyFramebuffer(
        device,
        framebuffer,
        nullptr
    );

    // destroy the render pass
    vkDestroyRenderPass(
        device,
        renderPass,
        nullptr
    );
```

## Now type it again

Re-drill the traditional attachment path.

```cpp
    // describe the color attachment
    VkAttachmentDescription colorAttachment{
        0,
        swapchainFormat,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    // describe the depth attachment
    VkAttachmentDescription depthAttachment{
        0,
        depthFormat,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    // reference the color attachment
    VkAttachmentReference colorReference{
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    // reference the depth attachment
    VkAttachmentReference depthReference{
        1,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    // describe one graphics subpass
    VkSubpassDescription subpass{
        0,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        0,
        nullptr,
        1,
        &colorReference,
        nullptr,
        &depthReference,
        0,
        nullptr
    };
```

Re-drill the dynamic rendering path.

```cpp
    // describe the dynamic color attachment
    VkRenderingAttachmentInfo dynamicColor{
        VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        nullptr,
        colorView,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_RESOLVE_MODE_NONE,
        VK_NULL_HANDLE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        colorClear
    };

    // describe the dynamic depth attachment
    VkRenderingAttachmentInfo dynamicDepth{
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

    // describe the active dynamic attachments
    VkRenderingInfo renderingInfo{
        VK_STRUCTURE_TYPE_RENDERING_INFO,
        nullptr,
        0,
        {{0, 0}, {width, height}},
        1,
        0,
        &dynamicColor,
        &dynamicDepth,
        nullptr
    };

    // begin dynamic rendering
    vkCmdBeginRendering(
        commandBuffer,
        &renderingInfo
    );

    // draw the geometry
    vkCmdDraw(
        commandBuffer,
        vertexCount,
        1,
        0,
        0
    );

    // finish dynamic rendering
    vkCmdEndRendering(
        commandBuffer
    );
```

## Wrap up

```text
render pass -> framebuffer -> begin -> draw -> end
dynamic rendering -> attachments -> begin -> draw -> end
```
