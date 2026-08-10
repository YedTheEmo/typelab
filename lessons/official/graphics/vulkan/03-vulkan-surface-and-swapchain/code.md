# Vulkan surface and swapchain - typing

This lesson types the presentation path: create a surface, inspect its support,
create a swapchain, retrieve its images, and create image views.

## Create the surface

The surface connects the Vulkan instance to a window-system presentation target.

```cpp
    // create the window-system surface through the window library
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    // assume the window library has created the Vulkan surface
    createWindowSurface(instance, &surface);
```

The helper represents the platform-specific operation supplied by a window
library such as SDL or GLFW.

## Check presentation support

The selected queue family must be able to present to this particular surface.

```cpp
    // store whether the queue family can present
    VkBool32 presentSupport = VK_FALSE;

    // query presentation support for the selected queue family
    vkGetPhysicalDeviceSurfaceSupportKHR(
        physicalDevice,
        graphicsFamily,
        surface,
        &presentSupport
    );
```

A real renderer should reject the queue family when presentation is unsupported.

## Query surface capabilities

The surface reports the constraints that the swapchain must satisfy.

```cpp
    // store the surface capabilities
    VkSurfaceCapabilitiesKHR capabilities{};

    // query the surface capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physicalDevice,
        surface,
        &capabilities
    );
```

## Query surface formats

The surface determines which image formats can be presented.

```cpp
    // store the number of supported formats
    uint32_t formatCount = 0;

    // query the number of formats
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice,
        surface,
        &formatCount,
        nullptr
    );

    // allocate storage for surface formats
    std::vector<VkSurfaceFormatKHR> formats(formatCount);

    // retrieve the supported formats
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice,
        surface,
        &formatCount,
        formats.data()
    );

    // choose the first supported format
    VkSurfaceFormatKHR surfaceFormat = formats[0];
```

The first format is used here to keep the example focused on the Vulkan
mechanism rather than format-selection policy.

## Query present modes

Present modes describe how completed images are scheduled for presentation.

```cpp
    // store the number of present modes
    uint32_t presentModeCount = 0;

    // query the number of present modes
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice,
        surface,
        &presentModeCount,
        nullptr
    );

    // allocate storage for present modes
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);

    // retrieve the supported present modes
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice,
        surface,
        &presentModeCount,
        presentModes.data()
    );

    // use the widely supported FIFO mode
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
```

FIFO is used because it is required to be supported for a Vulkan surface.

## Choose the swapchain extent

The extent determines the dimensions of each swapchain image.

```cpp
    // start with the surface's current extent
    VkExtent2D extent = capabilities.currentExtent;
```

A production renderer must handle platforms where the extent needs to be
clamped to the surface's reported minimum and maximum dimensions.

## Choose the image count

The swapchain contains multiple images for rendering and presentation.

```cpp
    // request one more image than the minimum
    uint32_t imageCount = capabilities.minImageCount + 1;

    // respect a nonzero maximum image count
    if (capabilities.maxImageCount > 0 &&
        imageCount > capabilities.maxImageCount) {
        // clamp the request to the surface maximum
        imageCount = capabilities.maxImageCount;
    }
```

## Create the swapchain

The selected surface properties are assembled into the swapchain description.

```cpp
    // describe the swapchain
    VkSwapchainCreateInfoKHR swapchainInfo{
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        nullptr,
        0,
        surface,
        imageCount,
        surfaceFormat.format,
        surfaceFormat.colorSpace,
        extent,
        1,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        capabilities.currentTransform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        presentMode,
        VK_TRUE,
        VK_NULL_HANDLE
    };

    // store the swapchain handle
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;

    // create the swapchain
    vkCreateSwapchainKHR(
        device,
        &swapchainInfo,
        nullptr,
        &swapchain
    );
```

The swapchain now owns the set of images that can be presented to the surface.

## Retrieve swapchain images

The swapchain exposes its images to the application for use during rendering.

```cpp
    // store the number of swapchain images
    uint32_t swapchainImageCount = 0;

    // query the image count
    vkGetSwapchainImagesKHR(
        device,
        swapchain,
        &swapchainImageCount,
        nullptr
    );

    // allocate storage for image handles
    std::vector<VkImage> swapchainImages(swapchainImageCount);

    // retrieve the swapchain images
    vkGetSwapchainImagesKHR(
        device,
        swapchain,
        &swapchainImageCount,
        swapchainImages.data()
    );
```

The application stores the handles but does not destroy these images itself.

## Create image views

Image views provide a specific interpretation of the swapchain images.

```cpp
    // allocate one view for every swapchain image
    std::vector<VkImageView> swapchainImageViews(swapchainImageCount);

    // process every swapchain image
    for (uint32_t index = 0; index < swapchainImageCount; ++index) {
        // describe the image view
        VkImageViewCreateInfo viewInfo{
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            nullptr,
            0,
            swapchainImages[index],
            VK_IMAGE_VIEW_TYPE_2D,
            surfaceFormat.format,
            {},
            {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
        };

        // create the image view
        vkCreateImageView(
            device,
            &viewInfo,
            nullptr,
            &swapchainImageViews[index]
        );
    }
```

The views are application-owned objects and must be destroyed separately.

## Acquire an image

Rendering begins by acquiring an available swapchain image.

```cpp
    // store the acquired image index
    uint32_t imageIndex = 0;

    // acquire the next available swapchain image
    vkAcquireNextImageKHR(
        device,
        swapchain,
        UINT64_MAX,
        imageAvailable,
        VK_NULL_HANDLE,
        &imageIndex
    );
```

The synchronization object will later coordinate acquisition with command
submission.

## Present the image

After rendering and synchronization, the acquired image can be presented.

```cpp
    // describe the presentation request
    VkPresentInfoKHR presentInfo{
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        nullptr,
        1,
        &renderFinished,
        1,
        &swapchain,
        &imageIndex,
        nullptr
    };

    // submit the image for presentation
    vkQueuePresentKHR(presentQueue, &presentInfo);
```

Presentation is performed through a queue that supports the surface.

## Destroy the presentation objects

Swapchain-dependent objects must be destroyed before the swapchain itself.

```cpp
    // destroy every swapchain image view
    for (VkImageView imageView : swapchainImageViews) {
        // destroy the image view
        vkDestroyImageView(device, imageView, nullptr);
    }

    // destroy the swapchain
    vkDestroySwapchainKHR(device, swapchain, nullptr);

    // destroy the window-system surface
    vkDestroySurfaceKHR(instance, surface, nullptr);
```

The swapchain images are not destroyed individually because their lifetime is
managed by the swapchain.

## Now type it again

Re-drill the surface-to-swapchain configuration.

```cpp
    // query surface capabilities
    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physicalDevice,
        surface,
        &capabilities
    );

    // request one more image than the minimum
    uint32_t imageCount = capabilities.minImageCount + 1;

    // describe the swapchain
    VkSwapchainCreateInfoKHR swapchainInfo{
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        nullptr,
        0,
        surface,
        imageCount,
        surfaceFormat.format,
        surfaceFormat.colorSpace,
        extent,
        1,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        0,
        nullptr,
        capabilities.currentTransform,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        presentMode,
        VK_TRUE,
        VK_NULL_HANDLE
    };

    // store the swapchain
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;

    // create the swapchain
    vkCreateSwapchainKHR(
        device,
        &swapchainInfo,
        nullptr,
        &swapchain
    );
```

## Wrap up

```text
surface -> capabilities -> format -> present mode -> swapchain -> images -> views
```

The next lesson uses these presentation resources as the destination for

