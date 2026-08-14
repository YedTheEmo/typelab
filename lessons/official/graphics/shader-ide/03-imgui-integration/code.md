# Dear ImGui integration - typing

This lesson types the ImGui integration: create the context, initialize the
SDL and Vulkan backends, upload the font atlas, run the per-frame sequence,
and shut down cleanly.

## Create the context and platform backend

The context and the SDL backend are created first.

```cpp
#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

// initialize the ImGui context and backends
bool initImGui(
    SDL_Window* window,
    VkInstance instance,
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    uint32_t queueFamily,
    VkQueue queue,
    VkDescriptorPool descriptorPool,
    VkRenderPass renderPass
) {
    // create the ImGui context
    ImGui::CreateContext();

    // configure the ImGui style
    ImGui::StyleColorsDark();

    // connect ImGui to the SDL window
    if (!ImGui_ImplSDL3_InitForVulkan(window)) {
        return false;
    }

    // describe the Vulkan backend initialization
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = instance;
    initInfo.PhysicalDevice = physicalDevice;
    initInfo.Device = device;
    initInfo.QueueFamily = queueFamily;
    initInfo.Queue = queue;
    initInfo.DescriptorPool = descriptorPool;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = 2;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    // initialize the Vulkan backend
    if (!ImGui_ImplVulkan_Init(&initInfo, renderPass)) {
        return false;
    }

    return true;
}
```

## Create a descriptor pool for ImGui

ImGui allocates descriptor sets from a shared pool.

```cpp
#include <vector>

// create the descriptor pool used by ImGui
VkDescriptorPool createImGuiDescriptorPool(VkDevice device) {
    // describe the descriptor pool sizes
    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
    };

    // describe the descriptor pool
    VkDescriptorPoolCreateInfo poolInfo{
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        nullptr,
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        1000,
        2,
        poolSizes
    };

    // store the created pool
    VkDescriptorPool pool = VK_NULL_HANDLE;

    // create the descriptor pool
    vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool);

    return pool;
}
```

## Upload the font atlas

The font texture is uploaded with a one-time command buffer submission.

```cpp
// upload the ImGui font texture to the GPU
bool uploadFontTexture(
    VkDevice device,
    VkQueue queue,
    VkCommandPool commandPool
) {
    // allocate a command buffer for the upload
    VkCommandBufferAllocateInfo allocateInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        nullptr,
        commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1
    };

    // store the upload command buffer
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    // allocate the upload command buffer
    vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer);

    // describe command recording
    VkCommandBufferBeginInfo beginInfo{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        nullptr
    };

    // begin recording the upload
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // create the font texture inside the upload
    ImGui_ImplVulkan_CreateFontsTexture();

    // finish recording the upload
    vkEndCommandBuffer(commandBuffer);

    // describe the upload submission
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

    // submit the upload to the graphics queue
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

    // wait until the upload completes
    vkDeviceWaitIdle(device);

    // free the CPU-side staging data
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    // free the upload command buffer
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

    return true;
}
```

## Run the ImGui frame

Each frame begins with the NewFrame sequence and widget calls.

```cpp
// draw the ImGui interface for one frame
void drawImGuiFrame() {
    // update the Vulkan backend state
    ImGui_ImplVulkan_NewFrame();

    // update the SDL backend input state
    ImGui_ImplSDL3_NewFrame();

    // begin the ImGui frame
    ImGui::NewFrame();

    // show a simple window to prove the integration
    ImGui::Begin("Shader IDE");
    ImGui::Text("Hello, shader editor.");
    ImGui::End();
}
```

## Record ImGui into a command buffer

After the widgets are drawn, ImGui produces draw data.

```cpp
// record the ImGui draw data into a command buffer
void recordImGui(VkCommandBuffer commandBuffer) {
    // finish the ImGui frame and produce draw data
    ImGui::Render();

    // record the ImGui draw commands
    ImGui_ImplVulkan_RenderDrawData(
        ImGui::GetDrawData(),
        commandBuffer
    );
}
```

## Shut down ImGui

ImGui is destroyed before the Vulkan objects it uses.

```cpp
// destroy all ImGui objects
void shutdownImGui() {
    // shut down the Vulkan backend
    ImGui_ImplVulkan_Shutdown();

    // shut down the SDL backend
    ImGui_ImplSDL3_Shutdown();

    // destroy the ImGui context
    ImGui::DestroyContext();
}
```

## Now type it again

Reconstruct the initialization order.

```cpp
// create the ImGui context
ImGui::CreateContext();

// connect ImGui to the SDL window
ImGui_ImplSDL3_InitForVulkan(window);

// initialize the Vulkan backend
ImGui_ImplVulkan_Init(&initInfo, renderPass);

// upload the font texture
ImGui_ImplVulkan_CreateFontsTexture();
```

Then reconstruct the per-frame sequence.

```cpp
// update the Vulkan backend state
ImGui_ImplVulkan_NewFrame();

// update the SDL backend input state
ImGui_ImplSDL3_NewFrame();

// begin the ImGui frame
ImGui::NewFrame();

// draw widgets
ImGui::Begin("Shader IDE");
ImGui::Text("Hello, shader editor.");
ImGui::End();

// finish the frame and produce draw data
ImGui::Render();

// record the ImGui draw commands
ImGui_ImplVulkan_RenderDrawData(
    ImGui::GetDrawData(),
    commandBuffer
);
```

## Wrap up

The flow:

```text
context -> SDL backend -> Vulkan backend -> fonts
    -> per frame: NewFrame -> widgets -> Render -> draw data
```

ImGui now draws on the Vulkan swapchain; the panels can be added.
