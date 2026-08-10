# Vulkan descriptors and resource binding - typing

This lesson types the descriptor path: define bindings, create a descriptor
pool and set, write a uniform buffer, connect it to the pipeline, and bind it.

## Describe the descriptor binding

Define one uniform buffer visible to the vertex shader.

```cpp
    // describe the frame uniform binding
    VkDescriptorSetLayoutBinding binding{
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_VERTEX_BIT,
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

    // store the descriptor set layout
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

    // create the descriptor set layout
    vkCreateDescriptorSetLayout(
        device,
        &layoutInfo,
        nullptr,
        &descriptorSetLayout
    );
```

## Create the descriptor pool

Give the renderer capacity for one uniform-buffer descriptor.

```cpp
    // describe the uniform buffer pool capacity
    VkDescriptorPoolSize poolSize{
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1
    };

    // describe the descriptor pool
    VkDescriptorPoolCreateInfo poolInfo{
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        nullptr,
        0,
        1,
        1,
        &poolSize
    };

    // store the descriptor pool
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    // create the descriptor pool
    vkCreateDescriptorPool(
        device,
        &poolInfo,
        nullptr,
        &descriptorPool
    );
```

## Allocate the descriptor set

Allocate one set using the layout created above.

```cpp
    // describe the descriptor set allocation
    VkDescriptorSetAllocateInfo allocateInfo{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        descriptorPool,
        1,
        &descriptorSetLayout
    };

    // store the allocated descriptor set
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    // allocate the descriptor set
    vkAllocateDescriptorSets(
        device,
        &allocateInfo,
        &descriptorSet
    );
```

## Describe the buffer resource

Tell the descriptor which buffer and byte range it should expose.

```cpp
    // describe the uniform buffer range
    VkDescriptorBufferInfo bufferInfo{
        uniformBuffer,
        0,
        sizeof(UniformBufferObject)
    };
```

## Write the descriptor

Connect the uniform buffer to binding zero.

```cpp
    // describe the descriptor update
    VkWriteDescriptorSet write{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        nullptr,
        descriptorSet,
        0,
        0,
        1,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        nullptr,
        &bufferInfo,
        nullptr
    };

    // write the resource into the descriptor set
    vkUpdateDescriptorSets(
        device,
        1,
        &write,
        0,
        nullptr
    );
```

## Create the pipeline layout

Connect the descriptor set layout to set zero of the pipeline layout.

```cpp
    // describe the pipeline layout interface
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        1,
        &descriptorSetLayout,
        0,
        nullptr
    };

    // store the pipeline layout
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    // create the pipeline layout
    vkCreatePipelineLayout(
        device,
        &pipelineLayoutInfo,
        nullptr,
        &pipelineLayout
    );
```

## Bind the descriptor set

Make the uniform buffer visible to subsequent graphics commands.

```cpp
    // bind the descriptor set at set zero
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

## Use multiple bindings

A descriptor set can contain more than one resource binding.

```cpp
    // describe a storage buffer binding
    VkDescriptorSetLayoutBinding storageBinding{
        1,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        1,
        VK_SHADER_STAGE_VERTEX_BIT |
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr
    };

    // describe a second resource alongside the uniform buffer
    VkDescriptorSetLayoutBinding bindings[]{
        binding,
        storageBinding
    };

    // describe the combined descriptor set layout
    VkDescriptorSetLayoutCreateInfo combinedLayoutInfo{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        2,
        bindings
    };
```

## Use an image descriptor

A texture binding can provide an image view and sampler to the fragment
shader.

```cpp
    // describe the sampled image resource
    VkDescriptorImageInfo imageInfo{
        sampler,
        imageView,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };

    // describe the sampled image binding
    VkDescriptorSetLayoutBinding imageBinding{
        2,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        1,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        nullptr
    };

    // describe the image descriptor update
    VkWriteDescriptorSet imageWrite{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        nullptr,
        descriptorSet,
        2,
        0,
        1,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        &imageInfo,
        nullptr,
        nullptr
    };
```

## Use a dynamic uniform descriptor

A dynamic descriptor lets command recording choose an offset into a buffer.

```cpp
    // describe a dynamic uniform binding
    VkDescriptorSetLayoutBinding dynamicBinding{
        3,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        1,
        VK_SHADER_STAGE_VERTEX_BIT,
        nullptr
    };

    // describe the dynamic buffer range
    VkDescriptorBufferInfo dynamicBufferInfo{
        objectBuffer,
        0,
        objectAlignment
    };
```

## Bind with a dynamic offset

Supply the object region selected from the larger buffer.

```cpp
    // select the first object's buffer region
    uint32_t dynamicOffset = 0;

    // bind the descriptor with its selected offset
    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout,
        0,
        1,
        &descriptorSet,
        1,
        &dynamicOffset
    );
```

## Use push constants

Push constants provide a separate path for small shader-visible values.

```cpp
    // describe the push constant range
    VkPushConstantRange pushRange{
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(ObjectPushConstants)
    };

    // describe the pipeline layout with push constants
    VkPipelineLayoutCreateInfo pushLayoutInfo{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        nullptr,
        0,
        1,
        &descriptorSetLayout,
        1,
        &pushRange
    };
```

## Push object data

Send small per-draw data directly through the command buffer.

```cpp
    // store the object's small shader parameters
    ObjectPushConstants objectData{};

    // push the object parameters to the vertex shader
    vkCmdPushConstants(
        commandBuffer,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(ObjectPushConstants),
        &objectData
    );
```

## Destroy descriptor objects

Destroy descriptor infrastructure after GPU use has finished.

```cpp
    // destroy the descriptor pool
    vkDestroyDescriptorPool(
        device,
        descriptorPool,
        nullptr
    );

    // destroy the descriptor set layout
    vkDestroyDescriptorSetLayout(
        device,
        descriptorSetLayout,
        nullptr
    );

    // destroy the pipeline layout
    vkDestroyPipelineLayout(
        device,
        pipelineLayout,
        nullptr
    );
```

## Now type it again

Re-drill the basic descriptor layout and allocation path.

```cpp
    // describe the uniform buffer binding
    VkDescriptorSetLayoutBinding binding{
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1,
        VK_SHADER_STAGE_VERTEX_BIT,
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

    // store the descriptor set layout
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

    // create the descriptor set layout
    vkCreateDescriptorSetLayout(
        device,
        &layoutInfo,
        nullptr,
        &descriptorSetLayout
    );

    // describe the uniform buffer pool capacity
    VkDescriptorPoolSize poolSize{
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1
    };

    // describe the descriptor pool
    VkDescriptorPoolCreateInfo poolInfo{
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        nullptr,
        0,
        1,
        1,
        &poolSize
    };

    // store the descriptor pool
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    // create the descriptor pool
    vkCreateDescriptorPool(
        device,
        &poolInfo,
        nullptr,
        &descriptorPool
    );
```

Re-drill the resource write and command binding.

```cpp
    // describe the buffer visible to the shader
    VkDescriptorBufferInfo bufferInfo{
        uniformBuffer,
        0,
        sizeof(UniformBufferObject)
    };

    // describe the descriptor update
    VkWriteDescriptorSet write{
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        nullptr,
        descriptorSet,
        0,
        0,
        1,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        nullptr,
        &bufferInfo,
        nullptr
    };

    // update the descriptor set
    vkUpdateDescriptorSets(
        device,
        1,
        &write,
        0,
        nullptr
    );

    // bind the descriptor set
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

## Wrap up

```text
layout -> pool -> allocate set -> write resource -> pipeline layout -> bind
```
