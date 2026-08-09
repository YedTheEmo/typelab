# Vulkan overview - typing

This lesson types the Vulkan execution model: the top-level instance, the
structures that describe it, and the record-then-submit flow of GPU work.

## Start with Vulkan

A Vulkan program includes the Vulkan header, then defines the entry point.

```
// pull in the Vulkan API types and functions
#include <vulkan/vulkan.h>

int main()
{
    return 0;
}
```

## Describe the application

Vulkan describes operations with structures. VkApplicationInfo holds the
application's identity for the driver.

```
// zero-initialize the struct so every field starts cleared
VkApplicationInfo appInfo{};
// identify which structure type this is
appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
// display name for the application
appInfo.pApplicationName = "TypeLab Vulkan";
// application version in Vulkan's version format
appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
// engine name that created this application
appInfo.pEngineName = "TypeLab";
// engine version in the same format
appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
// the Vulkan API version this application targets
appInfo.apiVersion = VK_API_VERSION_1_3;
```

## Create the instance

VkInstanceCreateInfo describes how the instance should be built.

```
// the create-info struct describes how to build the instance
VkInstanceCreateInfo createInfo{};
// identify the instance create-info type
createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
// point at the application description built above
createInfo.pApplicationInfo = &appInfo;

// handle that Vulkan will fill in
VkInstance instance = VK_NULL_HANDLE;

// create the instance; the result code reports success or failure
VkResult result =
    vkCreateInstance(&createInfo, nullptr, &instance);
```

## Think in terms of recorded work

A command buffer records work instead of executing it. The recorded buffer is
submitted to a queue for the GPU to run later.

```
// record a draw command into the buffer (no GPU work yet)
vkCmdDraw(commandBuffer, 3, 1, 0, 0);

// submit the recorded command buffer to a queue for execution
vkQueueSubmit(queue, 1, &submitInfo, fence);
```

## Put the model together

The complete program creates and destroys one instance.

```
// pull in the Vulkan API types and functions
#include <vulkan/vulkan.h>

int main()
{
    // zero-initialize the struct so every field starts cleared
    VkApplicationInfo appInfo{};
    // identify which structure type this is
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    // display name for the application
    appInfo.pApplicationName = "TypeLab Vulkan";
    // application version in Vulkan's version format
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    // engine name that created this application
    appInfo.pEngineName = "TypeLab";
    // engine version in the same format
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    // the Vulkan API version this application targets
    appInfo.apiVersion = VK_API_VERSION_1_3;

    // the create-info struct describes how to build the instance
    VkInstanceCreateInfo createInfo{};
    // identify the instance create-info type
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    // point at the application description built above
    createInfo.pApplicationInfo = &appInfo;

    // handle that Vulkan will fill in
    VkInstance instance = VK_NULL_HANDLE;

    // create the instance; the result code reports success or failure
    VkResult result =
        vkCreateInstance(&createInfo, nullptr, &instance);

    // bail out if instance creation failed
    if (result != VK_SUCCESS)
        return 1;

    // destroy the instance once the program is done with it
    vkDestroyInstance(instance, nullptr);

    return 0;
}
```

## Now type it again

Type the essential creation sequence again.

```
// zero-initialize the application description struct
VkApplicationInfo appInfo{};
// identify which structure type this is
appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

// the create-info struct describes how to build the instance
VkInstanceCreateInfo createInfo{};
// identify the instance create-info type
createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
// point at the application description built above
createInfo.pApplicationInfo = &appInfo;

// handle that Vulkan will fill in
VkInstance instance = VK_NULL_HANDLE;

// create the instance; the result code reports success or failure
VkResult result =
    vkCreateInstance(&createInfo, nullptr, &instance);
```

## Wrap up

The flow: application info -> instance info -> instance -> future GPU work.
