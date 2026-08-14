# Uniforms and input - typing

This lesson types the uniform buffer: define the layout that C++ and Slang
share, create the buffer, bind it to the descriptor set, update it every
frame from SDL input, and read it in a shader.

## Define the uniform layout

The struct is the contract between the host and the shader.

```cpp
#include <cstdint>

// per-frame values sent to the shader
struct FrameUniforms {
    // x = seconds since start, y = delta time
    float time[4];

    // xy = preview size in pixels
    float resolution[4];

    // xy = mouse position in pixels
    float mouse[4];
};
```

Every member is four floats, so the layout cannot drift between C++ and
Slang.

## The matching Slang side

The shader declares the same block.

```slang
// per-frame values sent from the host
struct FrameUniforms {
    float4 time;
    float4 resolution;
    float4 mouse;
};

// the block bound to the uniform buffer
ConstantBuffer<FrameUniforms> frame;
```

The shader reads the values through the block:

```slang
[shader("fragment")]
float4 main(float4 position : SV_Position) : SV_Target
{
    // convert the pixel position into normalized coordinates
    float2 uv = position.xy / frame.resolution.xy;

    // write a simple color driven by time
    float3 color = float3(uv.x, uv.y, 0.5 + 0.5 * sin(frame.time.x));
    return float4(color, 1.0);
}
```

## Create the uniform buffer

The buffer is created and mapped once.

```cpp
#include <vulkan/vulkan.h>

// the uniform buffer state
struct UniformBuffer {
    // the GPU buffer
    VkBuffer buffer = VK_NULL_HANDLE;

    // the buffer memory
    VkDeviceMemory memory = VK_NULL_HANDLE;

    // the persistent host mapping
    void* mapping = nullptr;
};

// create a host-visible uniform buffer
bool createUniformBuffer(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    UniformBuffer& uniformBuffer
) {
    // describe the buffer
    VkBufferCreateInfo bufferInfo{
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        nullptr,
        0,
        sizeof(FrameUniforms),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr
    };

    // create the buffer
    if (vkCreateBuffer(device, &bufferInfo, nullptr,
        &uniformBuffer.buffer) != VK_SUCCESS) {
        return false;
    }

    // query the memory requirements
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(
        device,
        uniformBuffer.buffer,
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
    if (vkAllocateMemory(device, &allocateInfo, nullptr,
        &uniformBuffer.memory) != VK_SUCCESS) {
        return false;
    }

    // bind the memory to the buffer
    vkBindBufferMemory(
        device,
        uniformBuffer.buffer,
        uniformBuffer.memory,
        0
    );

    // map the memory persistently
    vkMapMemory(
        device,
        uniformBuffer.memory,
        0,
        sizeof(FrameUniforms),
        0,
        &uniformBuffer.mapping
    );

    return true;
}
```

## Create the descriptor set

A descriptor set references the uniform buffer.

```cpp
// create the descriptor pool for the uniform set
VkDescriptorPool createUniformDescriptorPool(VkDevice device) {
    // describe the pool size
    VkDescriptorPoolSize poolSize{
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        1
    };

    // describe the pool
    VkDescriptorPoolCreateInfo poolInfo{
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        nullptr,
        0,
        1,
        1,
        &poolSize
    };

    // store the created pool
    VkDescriptorPool pool = VK_NULL_HANDLE;

    // create the pool
    vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool);

    return pool;
}

// allocate and bind the uniform descriptor set
bool createUniformDescriptorSet(
    VkDevice device,
    VkDescriptorPool pool,
    VkDescriptorSetLayout layout,
    UniformBuffer& uniformBuffer,
    VkDescriptorSet& descriptorSet
) {
    // describe the set allocation
    VkDescriptorSetAllocateInfo allocateInfo{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        nullptr,
        pool,
        1,
        &layout
    };

    // allocate the descriptor set
    if (vkAllocateDescriptorSets(device, &allocateInfo, &descriptorSet)
        != VK_SUCCESS) {
        return false;
    }

    // describe the buffer binding
    VkDescriptorBufferInfo bufferInfo{
        uniformBuffer.buffer,
        0,
        sizeof(FrameUniforms)
    };

    // describe the buffer write
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

    // write the buffer binding into the set
    vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

    return true;
}
```

## Update the buffer every frame

SDL input and the clock are written into the mapping.

```cpp
#include <SDL3/SDL.h>

// the per-frame application clock
struct FrameClock {
    // the time of the last frame
    double lastSeconds = 0.0;
};

// update the uniform buffer from current input
void updateUniforms(
    UniformBuffer& uniformBuffer,
    FrameClock& clock,
    uint32_t width,
    uint32_t height
) {
    // read the current time in seconds
    double now = SDL_GetTicks() / 1000.0;

    // compute the delta since the last frame
    double delta = clock.lastSeconds > 0.0
        ? now - clock.lastSeconds
        : 0.0;

    // remember the current time
    clock.lastSeconds = now;

    // query the mouse position
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    SDL_GetMouseState(&mouseX, &mouseY);

    // write the values into the mapped buffer
    FrameUniforms* data =
        static_cast<FrameUniforms*>(uniformBuffer.mapping);

    // write the time values
    data->time[0] = static_cast<float>(now);
    data->time[1] = static_cast<float>(delta);
    data->time[2] = 0.0f;
    data->time[3] = 0.0f;

    // write the resolution values
    data->resolution[0] = static_cast<float>(width);
    data->resolution[1] = static_cast<float>(height);
    data->resolution[2] = 0.0f;
    data->resolution[3] = 0.0f;

    // write the mouse values
    data->mouse[0] = mouseX;
    data->mouse[1] = mouseY;
    data->mouse[2] = 0.0f;
    data->mouse[3] = 0.0f;
}
```

## Now type it again

Reconstruct the layout contract.

```cpp
// the C++ side
struct FrameUniforms {
    float time[4];
    float resolution[4];
    float mouse[4];
};
```

```slang
// the Slang side
struct FrameUniforms {
    float4 time;
    float4 resolution;
    float4 mouse;
};
```

Then reconstruct the per-frame update.

```cpp
// query the mouse position
float mouseX = 0.0f;
float mouseY = 0.0f;
SDL_GetMouseState(&mouseX, &mouseY);

// write the values into the mapped buffer
FrameUniforms* data =
    static_cast<FrameUniforms*>(uniformBuffer.mapping);

data->time[0] = static_cast<float>(now);
data->time[1] = static_cast<float>(delta);
data->resolution[0] = static_cast<float>(width);
data->resolution[1] = static_cast<float>(height);
data->mouse[0] = mouseX;
data->mouse[1] = mouseY;
```

## Wrap up

The flow:

```text
SDL input + clock -> FrameUniforms -> uniform buffer -> descriptor set -> shader
```

Time, resolution, and mouse now reach the shader every frame.
