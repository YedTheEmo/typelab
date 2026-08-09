# Vulkan surface and swapchain - typing

This lesson types the window connection: create a window with GLFW, make a
Vulkan surface, choose swapchain settings, and retrieve the swapchain images.

## Create the window

GLFW creates the operating-system window for Vulkan to draw into.

```
// let GLFW pull in the Vulkan headers for us
#define GLFW_INCLUDE_VULKAN
// GLFW windowing API
#include <GLFW/glfw3.h>
// Vulkan API types and functions
#include <vulkan/vulkan.h>
// printing device properties and errors
#include <iostream>
// dynamic arrays for formats and modes
#include <vector>
// exceptions for fatal errors
#include <stdexcept>
// numeric limits for timeout values
#include <limits>

int main()
{
    // start the GLFW library
    if (!glfwInit())
        return 1;

    // tell GLFW we will use Vulkan, not OpenGL
    glfwWindowHint(
        GLFW_CLIENT_API,
        GLFW_NO_API);

    // create a 1280x720 window titled TypeLab Vulkan
    GLFWwindow* window = glfwCreateWindow(
        1280,
        720,
        "TypeLab Vulkan",
        nullptr,
        nullptr);

    // bail out if the window could not be created
    if (!window)
        return 1;

    // clean up GLFW resources
    glfwTerminate();
    return 0;
}
```

## Create the surface

The window needs a platform-specific surface before it can host Vulkan images.

```
// an instance must already exist before this step
VkInstance instance = VK_NULL_HANDLE;

// handle that will refer to the surface
VkSurfaceKHR surface = VK_NULL_HANDLE;

// ask GLFW to create the Vulkan surface for the window
if (glfwCreateWindowSurface(
    instance,
    window,
    nullptr,
    &surface) != VK_SUCCESS)
{
    // fail loudly if the surface could not be created
    throw std::runtime_error(
        "failed to create window surface");
}
```

## Check presentation support

The queue family must also support presenting to this surface.

```
// selected physical device from the previous lesson
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
// index of the graphics queue family
uint32_t graphicsFamily = 0;

// flag that Vulkan will set
VkBool32 presentSupported = VK_FALSE;

// ask whether this family can present to the surface
vkGetPhysicalDeviceSurfaceSupportKHR(
    physicalDevice,
    graphicsFamily,
    surface,
    &presentSupported);

// require that the graphics queue can also present
if (!presentSupported)
    throw std::runtime_error(
        "graphics queue cannot present");
```

## Query surface capabilities

The surface determines which swapchain configurations are legal.

```
// struct that Vulkan will fill with surface limits
VkSurfaceCapabilitiesKHR capabilities{};

// retrieve the surface's capabilities
vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    physicalDevice,
    surface,
    &capabilities);

// framebuffer size reported by GLFW
int width = 0;
int height = 0;

// read the current framebuffer size
glfwGetFramebufferSize(
    window,
    &width,
    &height);

// build the swapchain extent from the framebuffer size
VkExtent2D extent{
    static_cast<uint32_t>(width),
    static_cast<uint32_t>(height)
};
```

## Choose a surface format

The surface supports a set of image formats.

```
// variable that will hold the format count
uint32_t formatCount = 0;

// first call asks how many formats are supported
vkGetPhysicalDeviceSurfaceFormatsKHR(
    physicalDevice,
    surface,
    &formatCount,
    nullptr);

// storage for that many formats
std::vector<VkSurfaceFormatKHR> formats(formatCount);

// second call fills the vector with the formats
vkGetPhysicalDeviceSurfaceFormatsKHR(
    physicalDevice,
    surface,
    &formatCount,
    formats.data());

// use the first supported format for this lesson
VkSurfaceFormatKHR surfaceFormat = formats[0];
```

## Choose a present mode

The present mode controls how completed images reach the screen.

```
// variable that will hold the mode count
uint32_t presentModeCount = 0;

// first call asks how many present modes exist
vkGetPhysicalDeviceSurfacePresentModesKHR(
    physicalDevice,
    surface,
    &presentModeCount,
    nullptr);

// storage for that many modes
std::vector<VkPresentModeKHR> presentModes(
    presentModeCount);

// second call fills the vector with the modes
vkGetPhysicalDeviceSurfacePresentModesKHR(
    physicalDevice,
    surface,
    &presentModeCount,
    presentModes.data());

// FIFO is guaranteed to exist on every Vulkan device
VkPresentModeKHR presentMode =
    VK_PRESENT_MODE_FIFO_KHR;
```

## Choose the image count

More images let rendering and presentation overlap.

```
// start one above the minimum the surface recommends
uint32_t imageCount = capabilities.minImageCount + 1;

// clamp to the maximum when the surface specifies one
if (capabilities.maxImageCount > 0 &&
    imageCount > capabilities.maxImageCount)
{
    imageCount = capabilities.maxImageCount;
}
```

## Create the swapchain

The swapchain create-info bundles every choice made above.

```
// the create-info struct for the swapchain
VkSwapchainCreateInfoKHR swapchainInfo{};
// identify the swapchain create-info type
swapchainInfo.sType =
    VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
// the surface the swapchain presents to
swapchainInfo.surface = surface;
// the number of images requested
swapchainInfo.minImageCount = imageCount;
// image pixel format chosen above
swapchainInfo.imageFormat = surfaceFormat.format;
// image color space chosen above
swapchainInfo.imageColorSpace =
    surfaceFormat.colorSpace;
// the swapchain image dimensions
swapchainInfo.imageExtent = extent;
// one layer per image
swapchainInfo.imageArrayLayers = 1;
// images will be used as color render targets
swapchainInfo.imageUsage =
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
// images are not shared across queue families
swapchainInfo.imageSharingMode =
    VK_SHARING_MODE_EXCLUSIVE;
// keep the surface's current transform
swapchainInfo.preTransform =
    capabilities.currentTransform;
// the image is opaque
swapchainInfo.compositeAlpha =
    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
// the present mode chosen above
swapchainInfo.presentMode = presentMode;
// hidden pixels may be discarded
swapchainInfo.clipped = VK_TRUE;

// the logical device from the previous lesson
VkDevice device = VK_NULL_HANDLE;
// handle that Vulkan will fill in
VkSwapchainKHR swapchain = VK_NULL_HANDLE;

// create the swapchain on the logical device
if (vkCreateSwapchainKHR(
    device,
    &swapchainInfo,
    nullptr,
    &swapchain) != VK_SUCCESS)
{
    // fail loudly if the swapchain could not be created
    throw std::runtime_error(
        "failed to create swapchain");
}
```

## Retrieve the swapchain images

The swapchain owns images that the application can acquire.

```
// variable that will hold the image count
uint32_t swapchainImageCount = 0;

// first call asks how many images the swapchain has
vkGetSwapchainImagesKHR(
    device,
    swapchain,
    &swapchainImageCount,
    nullptr);

// storage for that many image handles
std::vector<VkImage> swapchainImages(
    swapchainImageCount);

// second call fills the vector with the image handles
vkGetSwapchainImagesKHR(
    device,
    swapchain,
    &swapchainImageCount,
    swapchainImages.data());
```

## Acquire an image

Before rendering, the application grabs an available image.

```
// index of the image handed to us
uint32_t imageIndex = 0;

// acquire an available image (no semaphores yet)
VkResult result = vkAcquireNextImageKHR(
    device,
    swapchain,
    std::numeric_limits<uint64_t>::max(),
    VK_NULL_HANDLE,
    VK_NULL_HANDLE,
    &imageIndex);
```

## Clean up

Destroy objects in reverse order of their dependencies.

```
// destroy the swapchain before the device
vkDestroySwapchainKHR(
    device,
    swapchain,
    nullptr);

// destroy the surface before the instance
vkDestroySurfaceKHR(
    instance,
    surface,
    nullptr);

// destroy the GLFW window
glfwDestroyWindow(window);
// stop the GLFW library
glfwTerminate();
```

## Now type it again

Type the core swapchain creation structure again.

```
// the create-info struct for the swapchain
VkSwapchainCreateInfoKHR swapchainInfo{};
// identify the swapchain create-info type
swapchainInfo.sType =
    VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
// the surface the swapchain presents to
swapchainInfo.surface = surface;
// the number of images requested
swapchainInfo.minImageCount = imageCount;
// image pixel format chosen above
swapchainInfo.imageFormat = surfaceFormat.format;
// image color space chosen above
swapchainInfo.imageColorSpace =
    surfaceFormat.colorSpace;
// the swapchain image dimensions
swapchainInfo.imageExtent = extent;
// one layer per image
swapchainInfo.imageArrayLayers = 1;
// images will be used as color render targets
swapchainInfo.imageUsage =
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

// index of the image handed to us
uint32_t imageIndex = 0;

// acquire an available image (no semaphores yet)
vkAcquireNextImageKHR(
    device,
    swapchain,
    std::numeric_limits<uint64_t>::max(),
    VK_NULL_HANDLE,
    VK_NULL_HANDLE,
    &imageIndex);
```

## Wrap up

The flow: window -> surface -> capabilities -> swapchain -> image -> rendering.
