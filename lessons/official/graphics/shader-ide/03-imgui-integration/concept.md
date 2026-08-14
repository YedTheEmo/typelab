# Dear ImGui integration - concepts

Dear ImGui is an immediate-mode user interface library. Instead of building a
tree of widgets that the application updates imperatively, the application
calls widget functions every frame and ImGui draws whatever the calls
describe. This fits the shader IDE perfectly: the editor, the diagnostics,
and the preview panel are all redrawn every frame from application state.

This lesson integrates ImGui with the SDL and Vulkan shell from the previous
lesson. The integration has three parts: the ImGui context, the platform
backend that reads SDL events, and the Vulkan backend that draws ImGui's
geometry with a pipeline built into the shell.

## Immediate mode fits a live tool

In immediate mode, state is not stored in widgets. It is stored in the
application, and the application tells ImGui what to show each frame.

```text
application state
    |
    v
widget calls (per frame)
    |
    v
ImGui draws widgets
```

A checkbox is a function call whose return value reflects the current state.
A text box is a function call that reads and writes an application buffer.
Because everything is redrawn constantly, adding a live-updating panel is
just another function call in the frame.

This matches the IDE's needs. The editor buffer, the compiled status, and the
diagnostics change every frame. Immediate mode makes each of them one call.

## The three objects of an ImGui application

An ImGui application owns three related objects.

The context stores all ImGui state: fonts, styles, windows, and internal
buffers. One context is created at startup and destroyed at shutdown.

The platform backend connects ImGui to the window system. For SDL3 this is
the imgui_impl_sdl3 backend. It translates SDL mouse, keyboard, and timer
events into the state ImGui needs.

The renderer backend connects ImGui to the graphics API. For Vulkan this is
the imgui_impl_vulkan backend. It builds the ImGui draw data into Vulkan
command buffer calls.

```text
ImGui::CreateContext()
    |
    v
ImGui_ImplSDL3_InitForVulkan(window)
    |
    v
ImGui_ImplVulkan_Init(info, renderPass)
```

## The Vulkan backend needs the shell

The Vulkan backend does not create its own instance or device. It reuses the
objects the shell already created. Initialization receives the instance, the
device, the queue, the queue family, a descriptor pool, and a render pass.

The descriptor pool is important. ImGui allocates descriptor sets from it
every frame. It must be large enough to cover the widgets and textures ImGui
creates.

```text
ImGui_ImplVulkan_InitInfo
    Instance
    PhysicalDevice
    Device
    QueueFamily
    Queue
    DescriptorPool
    MinImageCount
    ImageCount
```

The backend records its draw commands into a command buffer that the
application provides. The render pass tells the backend how ImGui's output is
written.

## Fonts are a texture

ImGui renders text from a font atlas texture. On Vulkan, the atlas must be
uploaded to the GPU before ImGui can draw text.

The upload happens once, after the Vulkan backend is initialized. It uses a
one-time command buffer submission:

```text
begin command buffer
    |
    v
ImGui_ImplVulkan_CreateFontsTexture()
    |
    v
end command buffer
    |
    v
submit and wait
```

After the upload completes, the backend frees the CPU-side staging data.
From then on the font atlas lives only on the GPU.

## The ImGui frame

Every frame follows the same sequence.

```cpp
ImGui_ImplVulkan_NewFrame();
ImGui_ImplSDL3_NewFrame();
ImGui::NewFrame();
```

The three NewFrame calls refresh input and internal state. After NewFrame the
application calls widgets, then produces draw data:

```cpp
ImGui::Render();
```

Render does not draw yet. It produces an internal list of draw commands.
Drawing happens later when the Vulkan backend converts those commands into
recorded Vulkan calls:

```cpp
ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
```

This split means ImGui output is part of the renderer's command recording,
not a separate pass.

## Windows and docking

ImGui windows are not OS windows. They are rectangles managed inside the
application window. The IDE creates several named windows:

```text
Editor        -> the shader source
Preview       -> the rendered output
Diagnostics   -> compile messages
```

Docking lets the user drag and rearrange these windows. It is enabled with a
config flag and provides the panel layout covered in a later lesson. For this
lesson the integration simply proves that ImGui can draw on the Vulkan
swapchain.

## ImGui owns input capture

While ImGui has keyboard and mouse focus, the application must not treat that
input as its own. Text typed into the editor box is ImGui input. Dragging a
window is ImGui input. The IDE's shader input comes only from ImGui widgets.

This is a useful boundary: SDL provides raw input, ImGui interprets it, and
the application reads the result through ImGui's state.

## Integration with the frame loop

The shell's main loop gains three stages around the existing render:

```text
poll events
    |
begin ImGui frame
    |
draw panels (widget calls)
    |
render preview
    |
present swapchain
```

The preview rendering happens in the same frame as ImGui. In a later lesson
the preview image is presented inside an ImGui window as a texture, which
means the Vulkan renderer and the ImGui backend share one command buffer
recording.

## Shutdown order

ImGui must be torn down before the Vulkan objects it uses.

```text
ImGui_ImplVulkan_Shutdown()
ImGui_ImplSDL3_Shutdown()
ImGui::DestroyContext()
```

The shutdown functions run while the device and swapchain still exist,
because the Vulkan backend holds device objects until it is shut down.

## What this lesson establishes

After integration the shell can draw an ImGui interface on a Vulkan
swapchain. This is the surface on which every other panel will be built. The
editor text box, the diagnostics list, and the preview window are all ImGui
widgets added to the frame in later lessons.

## Next step

Now type the code version of this lesson.
