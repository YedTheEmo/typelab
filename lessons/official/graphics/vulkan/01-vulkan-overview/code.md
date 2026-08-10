# Vulkan overview - typing

This lesson types a compact Vulkan path: create the instance, choose a GPU,
create a device and queue, record a command buffer, submit it, and clean up.

## Create the instance

The instance establishes the Vulkan connection.

```cpp
    // include the Vulkan API
    #include <vulkan/vulkan.h>

    // include dynamic arrays
    #include <vector>

    // include exceptions
    #include <stdexcept>

    // start the program
    int main() {
        // describe the application
        VkApplicationInfo applicationInfo{
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            nullptr,
            "Typelab",
            VK_MAKE_VERSION(1, 0, 0),
            "Typelab",
            VK_MAKE_VERSION(1, 0, 0),
            VK_API_VERSION_1_0
        };

        // describe the instance
        VkInstanceCreateInfo instanceInfo{
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            nullptr,
            0,
            &applicationInfo,
            0,
            nullptr,
            0,
            nullptr
        };

        // store the created instance
        VkInstance instance = VK_NULL_HANDLE;

        // create the Vulkan instance
        if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS) {
            // report failure
            throw std::runtime_error("failed to create instance");
        }
```

## Choose a physical device

The physical device represents available GPU hardware.

```cpp
        // store the number of physical devices
        uint32_t deviceCount = 0;

        // query the physical device count
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        // reject a machine without a Vulkan device
        if (deviceCount == 0) {
            // report failure
            throw std::runtime_error("no Vulkan device found");
        }

        // allocate storage for physical devices
        std::vector<VkPhysicalDevice> devices(deviceCount);

        // retrieve the devices
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // choose the first available physical device
        VkPhysicalDevice physicalDevice = devices[0];

        // use the first queue family for this minimal example
        uint32_t graphicsFamily = 0;
```

This example chooses family zero; real code must inspect queue properties.

## Create the logical device

The logical device provides the GPU interface.

```cpp
        // set the requested queue priority
        float queuePriority = 1.0f;

        // describe the queue requested from the device
        VkDeviceQueueCreateInfo queueInfo{
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            nullptr,
            0,
            graphicsFamily,
            1,
            &queuePriority
        };

        // describe the logical device
        VkDeviceCreateInfo deviceInfo{
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            nullptr,
            0,
            1,
            &queueInfo,
            0,
            nullptr,
            0,
            nullptr,
            nullptr
        };

        // store the logical device
        VkDevice device = VK_NULL_HANDLE;

        // create the logical device
        if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device)
            != VK_SUCCESS) {
            // report failure
            throw std::runtime_error("failed to create device");
        }

        // store the graphics queue
        VkQueue graphicsQueue = VK_NULL_HANDLE;

        // retrieve the first queue from the selected family
        vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
```

## Record a command buffer

A command buffer records GPU work before submission.

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
        vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);

        // describe the command buffer allocation
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
        vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer);

        // describe command recording
        VkCommandBufferBeginInfo beginInfo{
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        };

        // begin recording the command buffer
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        // finish the empty command buffer
        vkEndCommandBuffer(commandBuffer);
```

The buffer is empty because this lesson focuses on execution flow.

## Submit the command buffer

Submission sends recorded work to the queue.

```cpp
        // describe the command buffer submission
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

        // submit the recorded command buffer
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

        // wait until this simple submission finishes
        vkQueueWaitIdle(graphicsQueue);
```

## Clean up the objects

Vulkan uses explicit object lifetimes.

```cpp
        // destroy the command pool
        vkDestroyCommandPool(device, commandPool, nullptr);

        // destroy the logical device
        vkDestroyDevice(device, nullptr);

        // destroy the Vulkan instance
        vkDestroyInstance(instance, nullptr);

        // finish the program successfully
        return 0;
    }
```

## Now type it again

Re-drill the core execution sequence.

```cpp
    // begin recording
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // finish recording
    vkEndCommandBuffer(commandBuffer);

    // submit recorded work
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

    // wait for completion
    vkQueueWaitIdle(graphicsQueue);
```

## Wrap up

```text
instance -> physical device -> device -> queue -> record -> submit -> GPU
```

