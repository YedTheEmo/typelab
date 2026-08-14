# Window and Vulkan shell - typing

This lesson types the application shell: create an SDL window, a Vulkan
instance with SDL-provided extensions, a window surface, a logical device,
and a swapchain, then run the main loop and clean up in the correct order.

## Create the SDL window

The window is the application boundary.

```cpp
#include <SDL3/SDL.h>
#include <vulkan/vulkan.h>
#include <iostream>

// create the application window
bool createWindow(SDL_Window*& window) {
    // initialize the SDL video subsystem
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        // report the SDL failure
        std::cerr << "failed to initialize SDL: "
                  << SDL_GetError() << std::endl;
        return false;
    }

    // create a window that supports a Vulkan surface
    window = SDL_CreateWindow(
        "Shader IDE",
        1280,
        800,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );

    // reject window creation failure
    if (!window) {
        // report the SDL failure
        std::cerr << "failed to create window: "
                  << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}
```

## Create the Vulkan instance

The instance uses the extensions SDL requires.

```cpp
// create the Vulkan instance
bool createInstance(VkInstance& instance) {
    // request the extension list from SDL
    uint32_t extensionCount = 0;
    const char* const* extensions =
        SDL_Vulkan_GetInstanceExtensions(&extensionCount);

    // describe the application to the driver
    VkApplicationInfo applicationInfo{
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        nullptr,
        "Shader IDE",
        VK_MAKE_VERSION(1, 0, 0),
        "Shader IDE",
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_3
    };

    // describe the instance creation
    VkInstanceCreateInfo instanceInfo{
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        nullptr,
        0,
        &applicationInfo,
        extensionCount,
        extensions,
        0,
        nullptr
    };

    // create the Vulkan instance
    if (vkCreateInstance(&instanceInfo, nullptr, &instance)
        != VK_SUCCESS) {
        // report the failure
        std::cerr << "failed to create instance" << std::endl;
        return false;
    }

    return true;
}
```

## Create the window surface

The surface connects the window to Vulkan presentation.

```cpp
// create the window surface
bool createSurface(
    SDL_Window* window,
    VkInstance instance,
    VkSurfaceKHR& surface
) {
    // ask SDL to create the Vulkan surface
    if (!SDL_Vulkan_CreateSurface(
        window,
        instance,
        nullptr,
        &surface
    )) {
        // report the SDL failure
        std::cerr << "failed to create surface: "
                  << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}
```

## Choose a physical device

The IDE needs a device that supports graphics, compute, and presentation.

```cpp
#include <vector>

// select a suitable physical device
bool pickPhysicalDevice(
    VkInstance instance,
    VkSurfaceKHR surface,
    VkPhysicalDevice& physicalDevice
) {
    // query the number of physical devices
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    // reject machines without a Vulkan device
    if (deviceCount == 0) {
        return false;
    }

    // store the available devices
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    // inspect each device
    for (VkPhysicalDevice device : devices) {
        // query the device properties
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        // query the queue families
        uint32_t familyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(
            device,
            &familyCount,
            nullptr
        );

        std::vector<VkQueueFamilyProperties> families(familyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(
            device,
            &familyCount,
            families.data()
        );

        // scan for a family that supports graphics, compute, and present
        for (uint32_t i = 0; i < familyCount; i++) {
            // check the graphics and compute flags
            bool graphics = families[i].queueFlags
                & VK_QUEUE_GRAPHICS_BIT;
            bool compute = families[i].queueFlags
                & VK_QUEUE_COMPUTE_BIT;

            // check whether the family can present to the surface
            VkBool32 present = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(
                device,
                i,
                surface,
                &present
            );

            // prefer one family for everything
            if (graphics && compute && present) {
                physicalDevice = device;
                return true;
            }
        }
    }

    // no suitable device was found
    return false;
}
```

## Create the logical device

The logical device requests a queue from the selected family.

```cpp
// create the logical device and retrieve its queue
bool createDevice(
    VkPhysicalDevice physicalDevice,
    uint32_t queueFamily,
    VkDevice& device,
    VkQueue& queue
) {
    // set the requested queue priority
    float queuePriority = 1.0f;

    // describe the requested queue
    VkDeviceQueueCreateInfo queueInfo{
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        nullptr,
        0,
        queueFamily,
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

    // create the logical device
    if (vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device)
        != VK_SUCCESS) {
        return false;
    }

    // retrieve the queue from the selected family
    vkGetDeviceQueue(device, queueFamily, 0, &queue);

    return true;
}
```

## Choose a surface format

The swapchain needs a surface format and a present mode.

```cpp
// choose a surface format
VkSurfaceFormatKHR chooseSurfaceFormat(
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface
) {
    // query the supported surface formats
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice,
        surface,
        &formatCount,
        nullptr
    );

    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice,
        surface,
        &formatCount,
        formats.data()
    );

    // prefer the standard non-linear format
    for (const VkSurfaceFormatKHR& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM
            && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }

    // fall back to the first reported format
    return formats[0];
}
```

## Create the swapchain

The swapchain owns the presentation images.

```cpp
// create the swapchain
bool createSwapchain(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    VkSurfaceKHR surface,
    uint32_t width,
    uint32_t height,
    VkSwapchainKHR& swapchain
) {
    // choose the color format
    VkSurfaceFormatKHR surfaceFormat =
        chooseSurfaceFormat(physicalDevice, surface);

    // choose a present mode that waits for vertical sync
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    // describe the swapchain
    VkSwapchainCreateInfoKHR swapchainInfo{
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        nullptr,
        0,
        surface,
        2,
        surfaceFormat.format,
        surfaceFormat.colorSpace,
        VK_EXTENT_2D{ width, height },
        1,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        VK_PRESENT_MODE_FIFO_KHR,
        VK_FALSE,
        VK_NULL_HANDLE
    };

    // create the swapchain
    if (vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain)
        != VK_SUCCESS) {
        return false;
    }

    return true;
}
```

## Run the main loop

The shell polls events and lets the subsystems draw.

```cpp
#include <SDL3/SDL.h>

// run the application until the window is closed
void runMainLoop(SDL_Window* window) {
    // track whether the window is open
    bool running = true;

    // loop until the user closes the window
    while (running) {
        // process all pending events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // stop when the window is closed
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
    }
}
```

## Clean up in order

Objects are destroyed in reverse order of creation.

```cpp
// destroy every Vulkan object and the window
void cleanup(
    VkSwapchainKHR swapchain,
    VkDevice device,
    VkSurfaceKHR surface,
    VkInstance instance,
    SDL_Window* window
) {
    // destroy the swapchain
    vkDestroySwapchainKHR(device, swapchain, nullptr);

    // destroy the logical device
    vkDestroyDevice(device, nullptr);

    // destroy the window surface
    vkDestroySurfaceKHR(instance, surface, nullptr);

    // destroy the Vulkan instance
    vkDestroyInstance(instance, nullptr);

    // destroy the SDL window
    SDL_DestroyWindow(window);

    // shut down the SDL subsystem
    SDL_Quit();
}
```

## Now type it again

Reconstruct the shell creation order.

```cpp
// create the application window
SDL_Window* window = SDL_CreateWindow(
    "Shader IDE",
    1280,
    800,
    SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
);

// request the SDL instance extensions
uint32_t extensionCount = 0;
const char* const* extensions =
    SDL_Vulkan_GetInstanceExtensions(&extensionCount);

// create the Vulkan instance
vkCreateInstance(&instanceInfo, nullptr, &instance);

// create the window surface
SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface);

// create the logical device
vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device);

// create the swapchain
vkCreateSwapchainKHR(device, &swapchainInfo, nullptr, &swapchain);
```

Then reconstruct the cleanup order.

```cpp
// destroy objects in reverse order of creation
vkDestroySwapchainKHR(device, swapchain, nullptr);
vkDestroyDevice(device, nullptr);
vkDestroySurfaceKHR(instance, surface, nullptr);
vkDestroyInstance(instance, nullptr);
SDL_DestroyWindow(window);
SDL_Quit();
```

## Wrap up

The flow:

```text
window -> instance -> surface -> device -> queue -> swapchain -> loop
```

The shell establishes the environment; the renderer will build on it.
