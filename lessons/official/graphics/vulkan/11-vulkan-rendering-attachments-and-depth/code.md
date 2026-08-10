# Vulkan rendering attachments and depth - typing

This lesson types color and depth attachments, their views, depth state, and
the rendering setup that connects them.

## Describe the color attachment

Define the swapchain image as a clear-and-store color attachment.

```cpp
    // describe the swapchain color attachment
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

    // reference the color attachment during rendering
    VkAttachmentReference colorReference{
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
```

## Describe the depth attachment

Define a depth image that is cleared and used for depth testing.

```cpp
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

    // reference the depth attachment during rendering
    VkAttachmentReference depthReference{
        1,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
```

## Create the depth image

Create an image whose intended use is depth attachment storage.

```cpp
    // describe the depth image
    VkImageCreateInfo depthImageInfo{
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        nullptr,
        0,
        VK_IMAGE_TYPE_2D,
        depthFormat,
        {width, height, 1},
        1,
        1,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        VK_IMAGE_LAYOUT_UNDEFINED
    };

    // store the depth image
    VkImage depthImage = VK_NULL_HANDLE;

    // create the depth image
    vkCreateImage(
        device,
        &depthImageInfo,
        nullptr,
        &depthImage
    );
```

## Create the depth view

Expose the depth aspect of the image to rendering.

```cpp
    // describe the depth image view
    VkImageViewCreateInfo depthViewInfo{
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        depthImage,
        VK_IMAGE_VIEW_TYPE_2D,
        depthFormat,
        {},
        {
            VK_IMAGE_ASPECT_DEPTH_BIT,
            0,
            1,
            0,
            1
        }
    };

    // store the depth image view
    VkImageView depthView = VK_NULL_HANDLE;

    // create the depth image view
    vkCreateImageView(
        device,
        &depthViewInfo,
        nullptr,
        &depthView
    );
```

## Enable depth testing

Configure the graphics pipeline to reject fragments that are farther away.

```cpp
    // describe the depth testing state
    VkPipelineDepthStencilStateCreateInfo depthStencil{
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_TRUE,
        VK_TRUE,
        VK_COMPARE_OP_LESS,
        VK_FALSE,
        VK_FALSE,
        {},
        {}
    };
```

## Describe attachment storage

Put the color and depth descriptions into one attachment array.

```cpp
    // group the color and depth attachments
    VkAttachmentDescription attachments[]{
        colorAttachment,
        depthAttachment
    };
```

## Describe clear values

Supply one clear value for each attachment that is cleared.

```cpp
    // describe the attachment clear values
    VkClearValue clearValues[2]{};

    // clear the color attachment
    clearValues[0].color = {{0.05f, 0.05f, 0.08f, 1.0f}};

    // clear the depth attachment to the far depth
    clearValues[1].depthStencil = {1.0f, 0};
```

## Use the depth attachment

A traditional render pass associates the color and depth references with the
rendering operation.

```cpp
    // describe the color and depth attachment references
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

## Describe the render pass

Package the attachment descriptions and subpass into a render pass.

```cpp
    // describe the render pass attachments
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

    // store the render pass
    VkRenderPass renderPass = VK_NULL_HANDLE;

    // create the render pass
    vkCreateRenderPass(
        device,
        &renderPassInfo,
        nullptr,
        &renderPass
    );
```

## Create a framebuffer

Connect concrete image views to the attachment slots.

```cpp
    // group the views for one framebuffer
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

## Begin rendering

Clear both attachments when the render pass begins.

```cpp
    // describe the render area
    VkRenderPassBeginInfo beginInfo{
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        nullptr,
        renderPass,
        framebuffer,
        {{0, 0}, {width, height}},
        2,
        clearValues
    };

    // begin rendering with color and depth attachments
    vkCmdBeginRenderPass(
        commandBuffer,
        &beginInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    // draw geometry using the depth-tested pipeline
    vkCmdDraw(
        commandBuffer,
        vertexCount,
        1,
        0,
        0
    );

    // finish the render pass
    vkCmdEndRenderPass(
        commandBuffer
    );
```

## Use culling with depth

Configure back-face culling alongside depth testing.

```cpp
    // enable back-face culling
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;

    // define counter-clockwise front faces
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
```

## Now type it again

Re-drill the core color and depth attachment declarations.

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
```

Re-drill the depth state and rendering command sequence.

```cpp
    // enable depth testing and depth writes
    VkPipelineDepthStencilStateCreateInfo depthStencil{
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_TRUE,
        VK_TRUE,
        VK_COMPARE_OP_LESS,
        VK_FALSE,
        VK_FALSE,
        {},
        {}
    };

    // begin the color and depth rendering operation
    vkCmdBeginRenderPass(
        commandBuffer,
        &beginInfo,
        VK_SUBPASS_CONTENTS_INLINE
    );

    // draw the geometry
    vkCmdDraw(
        commandBuffer,
        vertexCount,
        1,
        0,
        0
    );

    // finish the rendering operation
    vkCmdEndRenderPass(
        commandBuffer
    );
```

## Wrap up

```text
color image + depth image -> attachments -> depth test -> draw
```
