# Vulkan textures and samplers - typing

This lesson types the texture resource path: create an image, create its view
and sampler, transition it for sampling, and describe it with a descriptor.

## Describe the texture image

Create a 2D image that can receive uploaded pixels and be sampled by shaders.

```cpp
    // describe the texture image
    VkImageCreateInfo imageInfo{
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        nullptr,
        0,
        VK_IMAGE_TYPE_2D,
        VK_FORMAT_R8G8B8A8_SRGB,
        {width, height, 1},
        1,
        mipLevels,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        VK_IMAGE_LAYOUT_UNDEFINED
    };

    // store the texture image
    VkImage textureImage = VK_NULL_HANDLE;

    // create the texture image
    vkCreateImage(
        device,
        &imageInfo,
        nullptr,
        &textureImage
    );
```

## Create the image view

Expose the texture's color subresource to shaders.

```cpp
    // describe the texture image view
    VkImageViewCreateInfo viewInfo{
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        textureImage,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_R8G8B8A8_SRGB,
        {},
        {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            mipLevels,
            0,
            1
        }
    };

    // store the texture view
    VkImageView textureView = VK_NULL_HANDLE;

    // create the texture view
    vkCreateImageView(
        device,
        &viewInfo,
        nullptr,
        &textureView
    );
```

## Create the sampler

Configure filtering, addressing, and mipmap selection.

```cpp
    // describe the texture sampler
    VkSamplerCreateInfo samplerInfo{
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        nullptr,
        0,
        VK_FILTER_LINEAR,
        VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        0.0f,
        VK_FALSE,
        1.0f,
        VK_FALSE,
        VK_COMPARE_OP_ALWAYS,
        0.0f,
        float(mipLevels - 1),
        VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
        VK_FALSE
    };

    // store the texture sampler
    VkSampler sampler = VK_NULL_HANDLE;

    // create the texture sampler
    vkCreateSampler(
        device,
        &samplerInfo,
        nullptr,
        &sampler
    );
```

## Transition for upload

Prepare the image to receive pixels from a staging buffer.

```cpp
    // describe the upload transition
    VkImageMemoryBarrier2 uploadBarrier{
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        nullptr,
        VK_PIPELINE_STAGE_2_NONE,
        VK_ACCESS_2_NONE,
        VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        VK_ACCESS_2_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        queueFamily,
        queueFamily,
        textureImage,
        {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            mipLevels,
            0,
            1
        }
    };

    // describe the upload dependency
    VkDependencyInfo uploadDependency{
        VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        nullptr,
        0,
        0,
        nullptr,
        1,
        &uploadBarrier,
        0,
        nullptr
    };

    // transition the image for transfer writes
    vkCmdPipelineBarrier2(
        commandBuffer,
        &uploadDependency
    );
```

## Copy the pixels

Copy staging-buffer data into the texture image.

```cpp
    // describe the copied texture region
    VkBufferImageCopy region{
        0,
        0,
        0,
        {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            0,
            1
        },
        {0, 0, 0},
        {width, height, 1}
    };

    // copy texture pixels into the image
    vkCmdCopyBufferToImage(
        commandBuffer,
        stagingBuffer,
        textureImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
```

## Transition for shader reads

Prepare the uploaded texture for fragment-shader sampling.

```cpp
    // describe the shader-read transition
    VkImageMemoryBarrier2 sampleBarrier{
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        nullptr,
        VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        VK_ACCESS_2_TRANSFER_WRITE_BIT,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        queueFamily,
        queueFamily,
        textureImage,
        {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            mipLevels,
            0,
            1
        }
    };

    // describe the sampling dependency
    VkDependencyInfo sampleDependency{
        VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        nullptr,
        0,
        0,
        nullptr,
        1,
        &sampleBarrier,
        0,
        nullptr
    };

    // transition the image for shader reads
    vkCmdPipelineBarrier2(
        commandBuffer,
        &sampleDependency
    );
```

## Describe the sampled resource

Combine the sampler and image view into descriptor information.

```cpp
    // describe the sampled texture
    VkDescriptorImageInfo imageInfo{
        sampler,
        textureView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };
```

## Write the texture descriptor

Connect the sampled image to a fragment-shader binding.

```cpp
    // describe the texture descriptor update
    VkWriteDescriptorSet imageWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        nullptr,
        descriptorSet,
        0,
        0,
        1,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        &imageInfo,
        nullptr,
        nullptr
    };

    // write the sampled texture into the descriptor set
    vkUpdateDescriptorSets(
        device,
        1,
        &imageWrite,
        0,
        nullptr
    );
```

## Bind the texture

Make the descriptor-backed texture available to the draw.

```cpp
    // bind the texture descriptor set
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

## Sample the texture

The fragment shader uses interpolated texture coordinates to fetch a color.

```cpp
    // declare the interpolated texture coordinate
    layout(location = 1) in vec2 uv;

    // declare the sampled texture
    layout(set = 0, binding = 0) uniform sampler2D albedo;

    // declare the fragment color
    layout(location = 0) out vec4 outColor;

    // sample the texture at the interpolated coordinate
    void main() {
        // write the sampled texture color
        outColor = texture(albedo, uv);
    }
```

## Create mip levels

Calculate the number of levels needed for a complete mip chain.

```cpp
    // start with the largest image dimension
    uint32_t largestDimension = std::max(width, height);

    // calculate the number of mip levels
    uint32_t mipLevels = 1;

    // reduce the dimension once for each additional level
    while (largestDimension > 1) {
        // move to the next smaller mip dimension
        largestDimension /= 2;

        // count the new mip level
        ++mipLevels;
    }
```

## Now type it again

Re-drill the image, view, and sampler relationship.

```cpp
    // describe the texture image
    VkImageCreateInfo imageInfo{
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        nullptr,
        0,
        VK_IMAGE_TYPE_2D,
        VK_FORMAT_R8G8B8A8_SRGB,
        {width, height, 1},
        1,
        mipLevels,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT |
        VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        VK_IMAGE_LAYOUT_UNDEFINED
    };

    // store the texture image
    VkImage textureImage = VK_NULL_HANDLE;

    // create the texture image
    vkCreateImage(
        device,
        &imageInfo,
        nullptr,
        &textureImage
    );

    // describe the texture view
    VkImageViewCreateInfo viewInfo{
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        textureImage,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_R8G8B8A8_SRGB,
        {},
        {
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            mipLevels,
            0,
            1
        }
    };

    // store the texture view
    VkImageView textureView = VK_NULL_HANDLE;

    // create the texture view
    vkCreateImageView(
        device,
        &viewInfo,
        nullptr,
        &textureView
    );

    // describe the texture sampler
    VkSamplerCreateInfo samplerInfo{
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        nullptr,
        0,
        VK_FILTER_LINEAR,
        VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        0.0f,
        VK_FALSE,
        1.0f,
        VK_FALSE,
        VK_COMPARE_OP_ALWAYS,
        0.0f,
        float(mipLevels - 1),
        VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
        VK_FALSE
    };
```

Re-drill the texture upload transition and descriptor write.

```cpp
    // transition the image into transfer destination layout
    vkCmdPipelineBarrier2(
        commandBuffer,
        &uploadDependency
    );

    // copy the staging pixels into the texture
    vkCmdCopyBufferToImage(
        commandBuffer,
        stagingBuffer,
        textureImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    // transition the image into shader read layout
    vkCmdPipelineBarrier2(
        commandBuffer,
        &sampleDependency
    );

    // describe the sampled texture
    VkDescriptorImageInfo imageInfo{
        sampler,
        textureView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    // describe the texture descriptor
    VkWriteDescriptorSet imageWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        nullptr,
        descriptorSet,
        0,
        0,
        1,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        &imageInfo,
        nullptr,
        nullptr
    };

    // update the descriptor set
    vkUpdateDescriptorSets(
        device,
        1,
        &imageWrite,
        0,
        nullptr
    );
```

## Destroy texture objects

Destroy the texture objects after GPU execution can no longer reference them.

```cpp
    // destroy the texture sampler
    vkDestroySampler(
        device,
        sampler,
        nullptr
    );

    // destroy the texture view
    vkDestroyImageView(
        device,
        textureView,
        nullptr
    );

    // destroy the texture image
    vkDestroyImage(
        device,
        textureImage,
        nullptr
    );
```

## Wrap up

```text
image -> view -> sampler -> descriptor -> shader sample
```
