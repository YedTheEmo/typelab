# Preview pipeline - typing

This lesson types the preview renderer: build a shader module from the
compiled SPIR-V, create an offscreen target, build the pipeline, record a
preview frame, and display the target in the ImGui preview panel.

## Define the renderer state

The renderer owns the preview objects.

```cpp
#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

// the Vulkan preview renderer
struct PreviewRenderer {
    // the Vulkan device
    VkDevice device = VK_NULL_HANDLE;

    // the graphics queue
    VkQueue queue = VK_NULL_HANDLE;

    // the command pool for preview commands
    VkCommandPool commandPool = VK_NULL_HANDLE;

    // the render pass for the offscreen target
    VkRenderPass renderPass = VK_NULL_HANDLE;

    // the pipeline layout
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    // the graphics pipeline
    VkPipeline pipeline = VK_NULL_HANDLE;

    // the offscreen target image
    VkImage targetImage = VK_NULL_HANDLE;

    // the target image memory
    VkDeviceMemory targetMemory = VK_NULL_HANDLE;

    // the target image view
    VkImageView targetView = VK_NULL_HANDLE;

    // the sampler used to display the target
    VkSampler targetSampler = VK_NULL_HANDLE;

    // the descriptor set that shows the target to ImGui
    VkDescriptorSet targetDescriptorSet = VK_NULL_HANDLE;

    // the current target size
    uint32_t width = 512;
    uint32_t height = 512;
};
```

## Create a shader module from SPIR-V

The compiled SPIR-V becomes a Vulkan shader module.

```cpp
// create a shader module from compiled SPIR-V
bool createShaderModule(
    VkDevice device,
    const std::vector<uint32_t>& spirv,
    VkShaderModule& module
) {
    // describe the shader module
    VkShaderModuleCreateInfo moduleInfo{
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        nullptr,
        0,
        spirv.size() * sizeof(uint32_t),
        spirv.data()
    };

    // create the shader module
    if (vkCreateShaderModule(device, &moduleInfo, nullptr, &module)
        != VK_SUCCESS) {
        return false;
    }

    return true;
}
```

## Create the offscreen render pass

The target is the single color attachment.

```cpp
// create the render pass for the offscreen target
bool createTargetRenderPass(
    VkDevice device,
    VkRenderPass& renderPass
) {
    // describe the color attachment
    VkAttachmentDescription colorAttachment{
        0,
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    // reference the color attachment
    VkAttachmentReference colorReference{
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    // describe the subpass
    VkSubpassDescription subpass{
        0,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        0,
        nullptr,
        1,
        &colorReference,
        nullptr,
        nullptr,
        0,
        nullptr
    };

    // describe the render pass
    VkRenderPassCreateInfo renderPassInfo{
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        nullptr,
        0,
        1,
        &colorAttachment,
        1,
        &subpass,
        0,
        nullptr
    };

    // create the render pass
    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass)
        != VK_SUCCESS) {
        return false;
    }

    return true;
}
```

## Create the offscreen target image

The target is an image usable as a color attachment and a sampled texture.

```cpp
// create the offscreen target image
bool createTargetImage(
    PreviewRenderer& renderer
) {
    // describe the target image
    VkImageCreateInfo imageInfo{
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        nullptr,
        0,
        VK_IMAGE_TYPE_2D,
        VK_FORMAT_B8G8R8A8_UNORM,
        VkExtent3D{ renderer.width, renderer.height, 1 },
        1,
        1,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
            | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        VK_IMAGE_LAYOUT_UNDEFINED
    };

    // create the target image
    if (vkCreateImage(renderer.device, &imageInfo, nullptr,
        &renderer.targetImage) != VK_SUCCESS) {
        return false;
    }

    // allocate memory for the image
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(
        renderer.device,
        renderer.targetImage,
        &memoryRequirements
    );

    // describe the memory allocation
    VkMemoryAllocateInfo allocateInfo{
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        nullptr,
        memoryRequirements.size,
        0
    };

    // allocate the image memory
    if (vkAllocateMemory(renderer.device, &allocateInfo, nullptr,
        &renderer.targetMemory) != VK_SUCCESS) {
        return false;
    }

    // bind the memory to the image
    vkBindImageMemory(
        renderer.device,
        renderer.targetImage,
        renderer.targetMemory,
        0
    );

    // describe the image view
    VkImageViewCreateInfo viewInfo{
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        nullptr,
        0,
        renderer.targetImage,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_FORMAT_B8G8R8A8_UNORM,
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
        &renderer.targetView) != VK_SUCCESS) {
        return false;
    }

    return true;
}
```

## Create the pipeline layout

The pipeline exposes one descriptor set for the shader uniforms.

```cpp
// create the descriptor set layout for the uniform block
bool createUniformLayout(
    VkDevice device,
    VkDescriptorSetLayout& layout
) {
    // describe the uniform buffer binding
    VkDescriptorSetLayoutBinding binding{
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr
    };

    // describe the descriptor set layout
    VkDescriptorSetLayoutCreateInfo layoutInfo{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        1,
        &binding
    };

    // create the descriptor set layout
    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout)
        != VK_SUCCESS) {
        return false;
    }

    return true;
}

// create the pipeline layout
bool createPipelineLayout(
    VkDevice device,
    VkDescriptorSetLayout uniformLayout,
    VkPipelineLayout& pipelineLayout
) {
    // describe the pipeline layout
    VkPipelineLayoutCreateInfo layoutInfo{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        1,
        &uniformLayout,
        0,
        nullptr
    };

    // create the pipeline layout
    if (vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout)
        != VK_SUCCESS) {
        return false;
    }

    return true;
}
```

## Create the graphics pipeline

The pipeline combines the vertex and fragment stages.

```cpp
// create the preview graphics pipeline
bool createPreviewPipeline(
    PreviewRenderer& renderer,
    VkShaderModule vertexModule,
    VkShaderModule fragmentModule,
    VkDescriptorSetLayout uniformLayout
) {
    // describe the vertex shader stage
    VkPipelineShaderStageCreateInfo vertexStage{
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_VERTEX_BIT,
        vertexModule,
        "main",
        nullptr
    };

    // describe the fragment shader stage
    VkPipelineShaderStageCreateInfo fragmentStage{
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        nullptr,
        0,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        fragmentModule,
        "main",
        nullptr
    };

    // collect the shader stages
    VkPipelineShaderStageCreateInfo stages[] = {
        vertexStage,
        fragmentStage
    };

    // disable the vertex input
    VkPipelineVertexInputStateCreateInfo vertexInput{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        nullptr,
        0,
        0,
        nullptr,
        0,
        nullptr
    };

    // draw triangle lists
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_FALSE
    };

    // describe the viewport state
    VkViewport viewport{
        0.0f,
        0.0f,
        static_cast<float>(renderer.width),
        static_cast<float>(renderer.height),
        0.0f,
        1.0f
    };

    // describe the scissor rectangle
    VkRect2D scissor{
        VkOffset2D{ 0, 0 },
        VkExtent2D{ renderer.width, renderer.height }
    };

    // describe the viewport state
    VkPipelineViewportStateCreateInfo viewportState{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        nullptr,
        0,
        1,
        &viewport,
        1,
        &scissor
    };

    // disable the rasterizer overdraw
    VkPipelineRasterizationStateCreateInfo rasterizer{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_FALSE,
        VK_FALSE,
        VK_POLYGON_MODE_FILL,
        VK_CULL_MODE_NONE,
        VK_FRONT_FACE_COUNTER_CLOCKWISE,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    };

    // disable multisampling
    VkPipelineMultisampleStateCreateInfo multisample{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_SAMPLE_COUNT_1_BIT,
        VK_FALSE,
        0.0f,
        nullptr,
        VK_FALSE,
        VK_FALSE
    };

    // describe the color blend state
    VkPipelineColorBlendAttachmentState blendAttachment{
        VK_FALSE,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_BLEND_FACTOR_ONE,
        VK_BLEND_FACTOR_ZERO,
        VK_BLEND_OP_ADD,
        VK_COLOR_COMPONENT_R_BIT
            | VK_COLOR_COMPONENT_G_BIT
            | VK_COLOR_COMPONENT_B_BIT
            | VK_COLOR_COMPONENT_A_BIT
    };

    // describe the blend state
    VkPipelineColorBlendStateCreateInfo blendState{
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        nullptr,
        0,
        VK_FALSE,
        VK_LOGIC_OP_COPY,
        1,
        &blendAttachment,
        { 0.0f, 0.0f, 0.0f, 0.0f }
    };

    // describe the graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        nullptr,
        0,
        2,
        stages,
        &vertexInput,
        &inputAssembly,
        nullptr,
        &viewportState,
        &rasterizer,
        &multisample,
        nullptr,
        &blendState,
        nullptr,
        renderer.pipelineLayout,
        renderer.renderPass,
        0,
        VK_NULL_HANDLE,
        -1
    };

    // create the graphics pipeline
    if (vkCreateGraphicsPipelines(renderer.device, VK_NULL_HANDLE, 1,
        &pipelineInfo, nullptr, &renderer.pipeline) != VK_SUCCESS) {
        return false;
    }

    return true;
}
```

## Create the sampler

The sampler controls how ImGui reads the target.

```cpp
// create the target sampler
bool createTargetSampler(
    VkDevice device,
    VkSampler& sampler
) {
    // describe the sampler
    VkSamplerCreateInfo samplerInfo{
        VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        nullptr,
        0,
        VK_FILTER_LINEAR,
        VK_FILTER_LINEAR,
        VK_SAMPLER_MIPMAP_MODE_NEAREST,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        0.0f,
        VK_FALSE,
        1.0f,
        VK_FALSE,
        VK_COMPARE_OP_NEVER,
        0.0f,
        0.0f,
        0.0f,
        VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
        VK_FALSE
    };

    // create the sampler
    if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler)
        != VK_SUCCESS) {
        return false;
    }

    return true;
}
```

## Show the target in ImGui

A descriptor set binds the target image and sampler as a texture.

```cpp
#include <imgui.h>

// create a descriptor set that presents the target as a texture
bool createTargetDescriptorSet(
    PreviewRenderer& renderer,
    VkDescriptorSetLayout layout
) {
    // describe the set allocation
    VkDescriptorSetAllocateInfo allocateInfo{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        layout,
        1,
        &renderer.targetDescriptorSet
    };

    // allocate the descriptor set
    vkAllocateDescriptorSets(renderer.device, &allocateInfo,
        &renderer.targetDescriptorSet);

    // describe the image binding
    VkDescriptorImageInfo imageInfo{
        renderer.targetSampler,
        renderer.targetView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    // describe the image write
    VkWriteDescriptorSet write{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        nullptr,
        renderer.targetDescriptorSet,
        0,
        0,
        1,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        &imageInfo,
        nullptr,
        nullptr
    };

    // update the descriptor set
    vkUpdateDescriptorSets(renderer.device, 1, &write, 0, nullptr);

    return true;
}
```

## Record a preview frame

The target is rendered and left readable for ImGui.

```cpp
// record the preview render into a command buffer
void recordPreviewFrame(
    PreviewRenderer& renderer,
    VkCommandBuffer commandBuffer,
    VkDescriptorSet uniformSet
) {
    // describe the clear value
    VkClearValue clear{ { { 0.0f, 0.0f, 0.0f, 1.0f } } };

    // describe the render pass begin
    VkRenderPassBeginInfo beginInfo{
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        nullptr,
        renderer.renderPass,
        VkFramebuffer{ VK_NULL_HANDLE },
        VkRect2D{
            VkOffset2D{ 0, 0 },
            VkExtent2D{ renderer.width, renderer.height }
        },
        1,
        &clear
    };

    // begin the render pass
    vkCmdBeginRenderPass(commandBuffer, &beginInfo,
        VK_SUBPASS_CONTENTS_INLINE);

    // bind the preview pipeline
    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        renderer.pipeline
    );

    // bind the uniform descriptor set
    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        renderer.pipelineLayout,
        0,
        1,
        &uniformSet,
        0,
        nullptr
    );

    // draw the full-screen triangle
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    // end the render pass
    vkCmdEndRenderPass(commandBuffer);
}
```

## Display the target in the preview panel

The target texture is drawn by ImGui.

```cpp
// draw the preview image inside the ImGui preview panel
void drawPreviewPanel(PreviewRenderer& renderer) {
    // open the preview window
    ImGui::Begin("Preview");

    // show the rendered target image
    ImGui::Image(
        reinterpret_cast<ImTextureID>(renderer.targetDescriptorSet),
        ImVec2(
            static_cast<float>(renderer.width),
            static_cast<float>(renderer.height)
        )
    );

    // close the preview window
    ImGui::End();
}
```

## Now type it again

Reconstruct the pipeline stage description.

```cpp
// describe the vertex shader stage
VkPipelineShaderStageCreateInfo vertexStage{
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    nullptr,
    0,
    VK_SHADER_STAGE_VERTEX_BIT,
    vertexModule,
    "main",
    nullptr
};

// describe the fragment shader stage
VkPipelineShaderStageCreateInfo fragmentStage{
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
    nullptr,
    0,
    VK_SHADER_STAGE_FRAGMENT_BIT,
    fragmentModule,
    "main",
    nullptr
};
```

Then reconstruct the preview draw.

```cpp
// bind the preview pipeline
vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

// bind the uniform descriptor set
vkCmdBindDescriptorSets(
    commandBuffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    pipelineLayout,
    0,
    1,
    &uniformSet,
    0,
    nullptr
);

// draw the full-screen triangle
vkCmdDraw(commandBuffer, 3, 1, 0, 0);
```

## Wrap up

The flow:

```text
SPIR-V -> module -> pipeline -> full-screen draw -> target image -> ImGui::Image
```

The shader output is now visible in the preview panel.
