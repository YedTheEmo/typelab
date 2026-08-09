# Vulkan instance and device - typing

This lesson types the device setup: enumerate physical devices, pick a queue
family, create a logical device, and retrieve a graphics queue.

## Enumerate physical devices

An existing instance hands out the GPUs as physical device handles.

```
// handle that Vulkan will fill in
VkInstance instance = VK_NULL_HANDLE;

// variable that will hold the number of devices
uint32_t deviceCount = 0;

// first call asks Vulkan how many devices exist
vkEnumeratePhysicalDevices(
    instance,
    &deviceCount,
    nullptr);

// storage for that many device handles
std::vector<VkPhysicalDevice> devices(deviceCount);

// second call fills the vector with the handles
vkEnumeratePhysicalDevices(
    instance,
    &deviceCount,
    devices.data());

// use the first device for this lesson
VkPhysicalDevice physicalDevice = devices[0];
```

## Inspect the physical device

The selected GPU exposes properties that can be read back.

```
// struct that Vulkan will fill with device properties
VkPhysicalDeviceProperties properties{};

// ask Vulkan for the device's properties
vkGetPhysicalDeviceProperties(
    physicalDevice,
    &properties);

// print the device name to confirm what was selected
std::cout << properties.deviceName << '\n';
```

## Find a graphics queue family

Queue families group queues that share capabilities.

```
// variable that will hold the number of families
uint32_t queueFamilyCount = 0;

// first call asks how many queue families exist
vkGetPhysicalDeviceQueueFamilyProperties(
    physicalDevice,
    &queueFamilyCount,
    nullptr);

// storage for that many family descriptions
std::vector<VkQueueFamilyProperties> families(
    queueFamilyCount);

// second call fills the vector with the properties
vkGetPhysicalDeviceQueueFamilyProperties(
    physicalDevice,
    &queueFamilyCount,
    families.data());

// index of the family we want to use
uint32_t graphicsFamily = 0;

// walk each family until one supports graphics
for (uint32_t i = 0; i < queueFamilyCount; ++i)
{
    // graphics families set the GRAPHICS capability bit
    if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
        // remember the matching family index
        graphicsFamily = i;
        break;
    }
}
```

## Describe the queue

The logical device needs to know which queues to create.

```
// priority of the queue within its family (0.0 to 1.0)
float priority = 1.0f;

// the create-info struct for a queue
VkDeviceQueueCreateInfo queueInfo{};
// identify the queue create-info type
queueInfo.sType =
    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
// which queue family this queue belongs to
queueInfo.queueFamilyIndex = graphicsFamily;
// number of queues to create from this family
queueInfo.queueCount = 1;
// pointer to the priority array
queueInfo.pQueuePriorities = &priority;
```

## Create the logical device

The device create-info references the queue information.

```
// the create-info struct for the logical device
VkDeviceCreateInfo deviceInfo{};
// identify the device create-info type
deviceInfo.sType =
    VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
// one queue create-info is supplied
deviceInfo.queueCreateInfoCount = 1;
// pointer to the queue create-info
deviceInfo.pQueueCreateInfos = &queueInfo;

// handle that Vulkan will fill in
VkDevice logicalDevice = VK_NULL_HANDLE;

// create the logical device from the physical device
VkResult result = vkCreateDevice(
    physicalDevice,
    &deviceInfo,
    nullptr,
    &logicalDevice);

// bail out if device creation failed
if (result != VK_SUCCESS)
    return 1;
```

## Retrieve the queue

The logical device hands back a queue handle.

```
// handle that Vulkan will fill in
VkQueue graphicsQueue = VK_NULL_HANDLE;

// fetch the queue at index zero from the graphics family
vkGetDeviceQueue(
    logicalDevice,
    graphicsFamily,
    0,
    &graphicsQueue);
```

## Clean up

Destroy the logical device before the instance it depends on.

```
// destroy the logical device
vkDestroyDevice(logicalDevice, nullptr);
// destroy the instance
vkDestroyInstance(instance, nullptr);
```

## Now type it again

Type the discovery and creation sequence again.

```
// variable that will hold the number of devices
uint32_t deviceCount = 0;

// first call asks Vulkan how many devices exist
vkEnumeratePhysicalDevices(
    instance,
    &deviceCount,
    nullptr);

// storage for that many device handles
std::vector<VkPhysicalDevice> devices(deviceCount);

// second call fills the vector with the handles
vkEnumeratePhysicalDevices(
    instance,
    &deviceCount,
    devices.data());

// handle that Vulkan will fill in
VkQueue graphicsQueue = VK_NULL_HANDLE;

// fetch the queue at index zero from the graphics family
vkGetDeviceQueue(
    logicalDevice,
    graphicsFamily,
    0,
    &graphicsQueue);
```

## Wrap up

The flow: instance -> physical device -> queue family -> logical device -> queue.
