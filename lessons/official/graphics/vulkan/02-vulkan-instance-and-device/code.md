# Vulkan instance and device - typing

This lesson types device initialization: create the instance, inspect a GPU,
choose a graphics queue family, create a logical device, and retrieve a queue.

## Create the instance

The instance establishes the Vulkan environment used for device discovery.

```cpp
    // include the Vulkan API
    #include <vulkan/vulkan.h>

    // include dynamic arrays
    #include <vector>

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
            VK_API_VERSION_1_3
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

        // store the instance handle
        VkInstance instance = VK_NULL_HANDLE;

        // create the Vulkan instance
        vkCreateInstance(&instanceInfo, nullptr, &instance);
```

## Enumerate physical devices

The instance can now discover the physical devices exposed by Vulkan.

```cpp
        // store the physical device count
        uint32_t deviceCount = 0;

        // query the number of devices
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        // allocate storage for device handles
        std::vector<VkPhysicalDevice> devices(deviceCount);

        // retrieve the physical devices
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // select the first device for this example
        VkPhysicalDevice physicalDevice = devices[0];
```

This example selects the first device; production code should evaluate all
candidates against its requirements.

## Inspect the physical device

Physical-device queries reveal the queue families available to the application.

```cpp
        // store the queue family count
        uint32_t queueFamilyCount = 0;

        // query the queue family count
        vkGetPhysicalDeviceQueueFamilyProperties(
            physicalDevice,
            &queueFamilyCount,
            nullptr
        );

        // allocate queue family descriptions
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

        // retrieve queue family descriptions
        vkGetPhysicalDeviceQueueFamilyProperties(
            physicalDevice,
            &queueFamilyCount,
            queueFamilies.data()
        );

        // mark the graphics family as missing
        uint32_t graphicsFamily = UINT32_MAX;

        // inspect every queue family
        for (uint32_t index = 0; index < queueFamilyCount; ++index) {
            // check whether this family supports graphics
            if (queueFamilies[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                // remember the graphics family
                graphicsFamily = index;
                break;
            }
        }
```

The queue flags tell us which operations queues from each family can perform.

## Create the logical device

The logical device exposes the selected physical device to the application.

```cpp
        // set the queue priority
        float queuePriority = 1.0f;

        // describe the requested graphics queue
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
        vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device);
```

The queue request selects a family and asks for one queue from it.

## Retrieve the queue

The requested queue is retrieved from the logical device after creation.

```cpp
        // store the graphics queue
        VkQueue graphicsQueue = VK_NULL_HANDLE;

        // retrieve the first queue from the graphics family
        vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
```

The queue is now the submission destination that later lessons will use.

## Clean up the device

Vulkan requires explicit destruction of the device-level objects created here.

```cpp
        // wait for device work to finish
        vkDeviceWaitIdle(device);

        // destroy the logical device
        vkDestroyDevice(device, nullptr);

        // destroy the Vulkan instance
        vkDestroyInstance(instance, nullptr);

        // finish the program
        return 0;
    }
```

## Now type it again

Re-drill the queue creation and retrieval path.

```cpp
    // set the queue priority
    float queuePriority = 1.0f;

    // describe the requested queue
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
    vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device);

    // store the graphics queue
    VkQueue graphicsQueue = VK_NULL_HANDLE;

    // retrieve the graphics queue
    vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
```

## Wrap up

```text
instance -> enumerate -> select -> device -> queue
```

The physical device describes hardware; the logical device exposes it.

