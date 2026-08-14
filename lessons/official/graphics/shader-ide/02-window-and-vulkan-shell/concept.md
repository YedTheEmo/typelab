# Window and Vulkan shell - concepts

Every GUI application starts with a window, and every Vulkan application
starts with a set of global objects. The shader IDE combines both in one
shell: SDL creates the window and the event loop, and Vulkan creates the
instance, surface, device, and swapchain that the renderer will use.

The shell owns nothing that renders shaders. It only establishes the
environment in which the renderer works. This separation keeps later lessons
focused on the shader-specific work.

## The window is the application boundary

SDL3 provides the window, the event queue, and the input state. The window
must be created with the Vulkan flag so that SDL knows a Vulkan surface will
be attached to it.

```cpp
SDL_Window* window = SDL_CreateWindow(
    "Shader IDE",
    1280,
    800,
    SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
);
```

The window is resizable. The IDE must handle the swapchain changing size
whenever the window changes size, which is covered in a later lesson. For now
the shell establishes the window and the main loop.

The event loop is the heartbeat of the application:

```text
poll events
    |
    v
process events
    |
    v
draw frame
    |
    v
present
```

The shell reads events, updates application state, and then lets the
subsystems draw. Shader work happens inside the draw stage.

## The Vulkan instance

The instance is the root Vulkan object. It describes the application to the
driver and loads the instance-level extensions. A desktop application must
request the surface extension and the platform extensions that SDL requires.

SDL can provide the exact extension list:

```cpp
uint32_t extensionCount = 0;
const char* const* extensions =
    SDL_Vulkan_GetInstanceExtensions(&extensionCount);
```

Using SDL's list is more reliable than hard-coding platform extensions,
because the required names differ on Windows, Linux, and macOS.

The instance is created once and destroyed at shutdown. It is a device-level
object with the longest lifetime in the application.

## The window surface

The surface is the bridge between the window and Vulkan. Presentation works
by rendering into images that the surface presents to the window.

SDL creates the surface for us:

```cpp
VkSurfaceKHR surface;
SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface);
```

The surface is window-dependent. When the window is destroyed, the surface
must be destroyed before the instance.

## Choosing a device

The physical device is the GPU. An application usually has more than one,
and the IDE must choose one that supports the operations it needs: graphics,
compute, and presentation on the same queue family if possible.

```text
physical devices
    |
    v
filter for graphics + compute + present support
    |
    v
select one device
```

The IDE should prefer a device where one queue family supports graphics and
presentation together, because that simplifies the frame loop. It also needs
compute support for the compute preview lesson.

## The logical device and queues

The logical device is the interface the application actually uses. Creating
it requests one or more queues from the selected queue family.

The IDE requests:

```text
graphics queue     -> rendering the preview
compute queue      -> dispatching compute shaders
```

When graphics and compute share one family, a single queue serves both.
The renderer can then serialize work on one queue, which avoids cross-queue
synchronization for this application's scale.

The queue priority is a hint to the driver, not a guarantee. The IDE does not
depend on it.

## The swapchain

The swapchain is the presentation mechanism. It owns a collection of images
that the renderer draws into, and it presents them to the surface in order.

```text
swapchain images
    |
    v
acquire image
    |
    v
render
    |
    v
present image
```

The swapchain is created after the device and is tied to the surface. It
depends on:

```text
surface format (color layout)
present mode   (immediate, mailbox, fifo)
image extent   (window size)
```

The IDE picks a surface format and present mode, then creates the swapchain
with the current window size. Recreating it on resize is handled by the
renderer in a later lesson.

## Why the shell and renderer are separate

If the shell created and owned every Vulkan object, later lessons would have
to modify a giant initialization function. Instead the shell creates the
long-lived objects and passes them to the renderer as construction inputs.

```text
shell owns
    window
    instance
    surface
    device
    queues
    swapchain

renderer receives
    device
    queues
    swapchain
    and builds pipeline-level objects
```

This split matches the lifetime categories established in the course
overview: the device lifetime belongs to the shell, and the program lifetime
belongs to the renderer.

## The main loop shape

The main loop has a fixed shape for the whole course:

```cpp
while (running) {
    poll events
    begin frame (ImGui)
    draw panels
    render preview
    present swapchain
}
```

The details of each stage are added in later lessons. The shell establishes
the skeleton now, so each later lesson only fills in one stage.

## Cleanup order

Vulkan requires explicit cleanup, and order matters. Objects must be
destroyed after the objects that depend on them.

```text
destroy swapchain
    ->
destroy device
    ->
destroy surface
    ->
destroy instance
    ->
destroy window
```

The renderer is torn down first because it owns swapchain-dependent objects.
Then the shell destroys the device-level objects. Following this order
prevents the driver from seeing objects used after destruction.

## Errors are not optional

Vulkan calls return result codes. A shell that ignores them becomes
impossible to debug when the driver refuses to create an object.

The IDE checks important results and reports failures with a message that
identifies the failed step:

```cpp
if (vkCreateInstance(&info, nullptr, &instance) != VK_SUCCESS) {
    // report which step failed
    std::cerr << "failed to create instance" << std::endl;
    return false;
}
```

This course keeps the pattern consistent: every creation call is checked, and
failure produces a clear message instead of an undefined crash.

## What the shell does not do

The shell deliberately does not create pipelines, descriptor sets, command
buffers, or textures. Those belong to the renderer. It also does not know
anything about shader source or Slang.

This boundary means the shell code is written once and never changes when
shader features are added. All shader-related state lives in the renderer,
which is the next layer.

## Next step

Now type the code version of this lesson.
