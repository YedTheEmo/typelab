# Compute shader preview - typing

This lesson types the compute path: create the storage image, build the
compute pipeline with its descriptor bindings, dispatch the shader, insert
the synchronization barrier, and show the storage image in the preview.

## Define the compute state

The compute renderer holds the storage image and pipeline.

```cpp
#include <vulkan/vulkan.h>

// the compute preview state
struct ComputeRenderer {
    // the Vulkan device
    VkDevice device = VK_NULL_HANDLE;

    // the storage image the shader writes
    VkImage storageImage = VK_NULL_HANDLE;

    // the storage image memory
    VkDeviceMemory storageMemory = VK_NULL_HANDLE;

    // the storage image view
    VkImageView storageView = VK_NULL_HANDLE;

    // the sampler used to display the storage image
    VkSampler storageSampler = VK_NULL_HANDLE;

    // the descriptor set that presents the image
    VkDescriptorSet storageDescriptorSet = VK_NULL_HANDLE;

    // the compute pipeline
    VkPipeline computePipeline = VK_NULL_HANDLE;

    // the current image size
    uint32_t width = 512;
    uint32_t height = 512;
};
```

## Create the storage image

The image supports storage writes and sampled reads.

```cpp
// create the storage image
bool createStorageImage(
    ComputeRenderer& renderer
) {
    // describe the storage image
    VkImageCreateInfo imageInfo{
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        nullptr,
        0,
        VK_IMAGE_TYPE_2D,
        VK_FORMAT_R8G8B8A8_UNORM,
        VkExtent3D{ renderer.width, renderer.height, 1 },
        1,
        1,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_STORAGE_BIT
            | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        VK_IMAGE_LAYOUT_UNDEFINED
    };

    // create the storage image
    if (vkCreateImage(renderer.device, &imageInfo, nullptr,
        &renderer.storageImage) != VK_SUCCESS) {
        return false;
    }

    // query the image memory requirements
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(
        renderer.device,
        renderer.storageImage,
        &requirements
    );

    // describe the memory allocation
    VkMemoryAllocateInfo allocateInfo{
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        requirements.size,
        0
    };

    // allocate the memory
    if (vkAllocateMemory(renderer.device, &allocateInfo, nullptr,
        &renderer.storageMemory) != VK_SUCCESS) {
        return false;
    }

    // bind the memory to the image
    vkBindImageMemory(
        renderer.device,
        renderer.storageImage,
        renderer.storageMemory,
        0
    );

    // describe the image view
    VkImageViewCreateInfo viewInfo{
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        renderer.storageImage,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_R8G8B8A8_UNORM,
        VkComponentMapping{
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY
        },
        VkImageSubresourceRange{
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            1,
            0,
            1
        }
    };

    // create the image view
    if (vkCreateImageView(renderer.device, &viewInfo, nullptr,
        &renderer.storageView) != VK_SUCCESS) {
        return false;
    }

    return true;
}
```

## Build the compute pipeline

The pipeline binds the uniform buffer and the storage image.

```cpp
#include <vector>

// create the compute pipeline
bool createComputePipeline(
    ComputeRenderer& renderer,
    VkShaderModule computeModule,
    VkDescriptorSetLayout computeLayout,
    VkPipelineLayout pipelineLayout
) {
    // describe the compute shader stage
    VkPipelineShaderStageCreateInfo computeStage{
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_COMPUTE_BIT,
        computeModule,
        "main",
        nullptr
    };

    // describe the compute pipeline
    VkComputePipelineCreateInfo pipelineInfo{
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        nullptr,
        0,
        computeStage,
        pipelineLayout,
        VK_NULL_HANDLE,
        -1
    };

    // create the compute pipeline
    if (vkCreateComputePipelines(renderer.device, VK_NULL_HANDLE, 1,
        &pipelineInfo, nullptr, &renderer.computePipeline)
        != VK_SUCCESS) {
        return false;
    }

    return true;
}
```

## Describe the storage image binding

The compute descriptor layout includes the storage image.

```cpp
// create the compute descriptor set layout
bool createComputeLayout(
    VkDevice device,
    VkDescriptorSetLayout& computeLayout
) {
    // describe the uniform buffer binding
    VkDescriptorSetLayoutBinding uniformBinding{
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_COMPUTE_BIT,
        nullptr
    };

    // describe the storage image binding
    VkDescriptorSetLayoutBinding storageBinding{
        1,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        1,
        VK_SHADER_STAGE_COMPUTE_BIT,
        nullptr
    };

    // collect the bindings
    VkDescriptorSetLayoutBinding bindings[] = {
        uniformBinding,
        storageBinding
    };

    // describe the descriptor set layout
    VkDescriptorSetLayoutCreateInfo layoutInfo{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        2,
        bindings
    };

    // create the descriptor set layout
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
        &computeLayout) != VK_SUCCESS) {
        return false;
    }

    return true;
}
```

## Bind the storage image to the descriptor set

The set references the storage image.

```cpp
// bind the storage image into a descriptor set
void bindStorageImage(
    ComputeRenderer& renderer,
    VkDescriptorSet descriptorSet
) {
    // describe the storage image binding
    VkDescriptorImageInfo imageInfo{
        VK_NULL_HANDLE,
        renderer.storageView,
        VK_IMAGE_LAYOUT_GENERAL
    };

    // describe the image write
    VkWriteDescriptorSet write{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        nullptr,
        descriptorSet,
        1,
        0,
        1,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        &imageInfo,
        nullptr,
        nullptr
    };

    // update the descriptor set
    vkUpdateDescriptorSets(renderer.device, 1, &write, 0, nullptr);
}
```

## Record a compute dispatch

The dispatch launches the thread grid and synchronizes the image.

```cpp
// record the compute dispatch into a command buffer
void recordComputeDispatch(
    ComputeRenderer& renderer,
    VkCommandBuffer commandBuffer,
    VkDescriptorSet computeSet,
    uint32_t workgroupSize
) {
    // transition the storage image into the general layout
    VkImageMemoryBarrier toGeneral{
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_NONE,
        VK_ACCESS_SHADER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        renderer.storageImage,
        VkImageSubresourceRange{
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            1,
            0,
            1
        }
    };

    // insert the layout transition
    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &toGeneral
    );

    // bind the compute pipeline
    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        renderer.computePipeline
    );

    // bind the compute descriptor set
    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        computePipelineLayout,
        0,
        1,
        &computeSet,
        0,
        nullptr
    );

    // compute the dispatch size
    uint32_t dispatchX = (renderer.width + workgroupSize - 1)
        / workgroupSize;
    uint32_t dispatchY = (renderer.height + workgroupSize - 1)
        / workgroupSize;

    // dispatch the compute grid
    vkCmdDispatch(commandBuffer, dispatchX, dispatchY, 1);

    // make the writes visible to sampled reads
    VkImageMemoryBarrier toRead{
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_SHADER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        renderer.storageImage,
        VkImageSubresourceRange{
            VK_IMAGE_ASPECT_COLOR_BIT,
            0,
            1,
            0,
            1
        }
    };

    // insert the write-to-read barrier
    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &toRead
    );
}
```

## Display the storage image

The preview panel shows the computed image.

```cpp
#include <imgui.h>

// draw the compute result in the preview panel
void drawComputePreview(ComputeRenderer& renderer) {
    // open the preview window
    ImGui::Begin("Preview");

    // compute a fitted size
    ImVec2 panel = ImGui::GetContentRegionAvail();
    float scale = std::min(
        panel.x / renderer.width,
        panel.y / renderer.height
    );

    // draw the storage image
    ImGui::Image(
        reinterpret_cast<ImTextureID>(renderer.storageDescriptorSet),
        ImVec2(renderer.width * scale, renderer.height * scale)
    );

    // close the preview window
    ImGui::End();
}
```

## Now type it again

Reconstruct the dispatch sizing.

```cpp
// compute the dispatch size
uint32_t dispatchX = (renderer.width + workgroupSize - 1)
    / workgroupSize;
uint32_t dispatchY = (renderer.height + workgroupSize - 1)
    / workgroupSize;

// dispatch the compute grid
vkCmdDispatch(commandBuffer, dispatchX, dispatchY, 1);
```

Then reconstruct the synchronization barrier.

```cpp
// make the writes visible to sampled reads
VkImageMemoryBarrier toRead{
    VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    nullptr,
    VK_ACCESS_SHADER_WRITE_BIT,
    VK_ACCESS_SHADER_READ_BIT,
    VK_IMAGE_LAYOUT_GENERAL,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    VK_QUEUE_FAMILY_IGNORED,
    VK_QUEUE_FAMILY_IGNORED,
    renderer.storageImage,
    VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
};

// insert the write-to-read barrier
vkCmdPipelineBarrier(
    commandBuffer,
    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
    0,
    0,
    nullptr,
    0,
    nullptr,
    1,
    &toRead
);
```

## Wrap up

The flow:

```text
storage image -> compute pipeline -> dispatch -> barrier -> preview panel
```

The compute path reaches the same preview as the fragment path.
