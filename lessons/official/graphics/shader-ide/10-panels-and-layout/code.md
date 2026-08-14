# Panels and layout - typing

This lesson types the layout: enable docking, build a dockspace, draw the
panels inside it, fit the preview image while preserving aspect ratio, and
resize the offscreen target when the panel changes size.

## Enable docking

The ImGui config enables docking and layout persistence.

```cpp
#include <imgui.h>

// configure ImGui for docking
void enableDocking() {
    // access the ImGui configuration
    ImGuiIO& io = ImGui::GetIO();

    // enable the docking feature
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // persist the layout across runs
    io.IniFilename = "shader-ide.ini";
}
```

## Create the dockspace

The dockspace fills the window and hosts the panels.

```cpp
// draw the dockspace and the main menu bar
void drawDockspace() {
    // describe the full-window dockspace
    ImGui::Begin("DockSpace");
    ImGui::DockSpace(
        ImGui::GetID("MainDockSpace"),
        ImVec2(0.0f, 0.0f),
        ImGuiDockNodeFlags_None
    );
    ImGui::End();
}
```

## Draw the menu bar

The menu bar hosts global commands.

```cpp
// draw the application menu bar
void drawMenuBar() {
    // open the main menu bar
    if (ImGui::BeginMenuBar()) {
        // open the File menu
        if (ImGui::BeginMenu("File")) {
            // menu items are added here in later lessons
            ImGui::Text("open and save arrive later");
            ImGui::EndMenu();
        }

        // open the View menu
        if (ImGui::BeginMenu("View")) {
            // toggle the diagnostics panel
            ImGui::MenuItem("Diagnostics", nullptr, &showDiagnostics);
            ImGui::EndMenu();
        }

        // close the main menu bar
        ImGui::EndMenuBar();
    }
}
```

## Fit the preview image

The image is fitted while preserving its aspect ratio.

```cpp
#include <algorithm>

// compute the fitted size for an image inside a panel
ImVec2 fitImage(
    const ImVec2& panel,
    const ImVec2& image
) {
    // reject an empty panel
    if (panel.x <= 0.0f || panel.y <= 0.0f) {
        return ImVec2(0.0f, 0.0f);
    }

    // compute the scale that fits both axes
    float scale = std::min(
        panel.x / image.x,
        panel.y / image.y
    );

    // return the scaled size
    return ImVec2(image.x * scale, image.y * scale);
}
```

## Draw the preview panel

The panel reads its size and draws the fitted image.

```cpp
#include <imgui.h>

// draw the preview panel
ImVec2 drawPreviewPanel(
    VkDescriptorSet targetDescriptorSet,
    uint32_t targetWidth,
    uint32_t targetHeight
) {
    // open the preview window
    ImGui::Begin("Preview");

    // read the available panel space
    ImVec2 panelSize = ImGui::GetContentRegionAvail();

    // compute the fitted image size
    ImVec2 fitted = fitImage(
        panelSize,
        ImVec2(
            static_cast<float>(targetWidth),
            static_cast<float>(targetHeight)
        )
    );

    // center the image within the panel
    ImVec2 offset(
        std::max(0.0f, (panelSize.x - fitted.x) * 0.5f),
        std::max(0.0f, (panelSize.y - fitted.y) * 0.5f)
    );

    // move the cursor to the fitted origin
    ImGui::SetCursorPos(offset);

    // draw the rendered target image
    ImGui::Image(
        reinterpret_cast<ImTextureID>(targetDescriptorSet),
        fitted
    );

    // close the preview window
    ImGui::End();

    // return the panel size for the renderer
    return panelSize;
}
```

## Resize the target when the panel changes

The renderer follows the panel size.

```cpp
#include <vulkan/vulkan.h>

// the preview renderer state
struct PreviewRenderer {
    VkDevice device = VK_NULL_HANDLE;
    VkImage targetImage = VK_NULL_HANDLE;
    VkImageView targetView = VK_NULL_HANDLE;
    uint32_t width = 512;
    uint32_t height = 512;
};

// recreate the target when the panel size changed
bool resizePreviewTarget(
    PreviewRenderer& renderer,
    const ImVec2& panelSize
) {
    // clamp the panel size to a minimum
    uint32_t width = std::max(1u, static_cast<uint32_t>(panelSize.x));
    uint32_t height = std::max(1u, static_cast<uint32_t>(panelSize.y));

    // skip when the size did not change
    if (width == renderer.width && height == renderer.height) {
        return true;
    }

    // destroy the old target image
    vkDestroyImageView(renderer.device, renderer.targetView, nullptr);
    vkDestroyImage(renderer.device, renderer.targetImage, nullptr);

    // store the new size
    renderer.width = width;
    renderer.height = height;

    // recreate the target image and view
    return createTargetImage(renderer);
}
```

## Draw the full layout

The layout composes every panel into one frame.

```cpp
// draw the complete application layout
void drawLayout(PreviewRenderer& renderer) {
    // draw the dockspace
    drawDockspace();

    // draw the menu bar
    drawMenuBar();

    // draw the editor panel
    drawEditorPanel(editorBuffer, errorLines);

    // draw the preview panel and capture its size
    ImVec2 previewSize = drawPreviewPanel(
        renderer.targetDescriptorSet,
        renderer.width,
        renderer.height
    );

    // follow the panel with the render target
    resizePreviewTarget(renderer, previewSize);

    // draw the diagnostics panel
    drawDiagnosticsPanel(compileResult, selectedLine);
}
```

## Now type it again

Reconstruct the preview fitting.

```cpp
// compute the scale that fits both axes
float scale = std::min(
    panel.x / image.x,
    panel.y / image.y
);

// return the scaled size
ImVec2 fitted(image.x * scale, image.y * scale);

// draw the rendered target image
ImGui::Image(
    reinterpret_cast<ImTextureID>(targetDescriptorSet),
    fitted
);
```

Then reconstruct the target resize check.

```cpp
// skip when the size did not change
if (width == renderer.width && height == renderer.height) {
    return true;
}

// destroy the old target image
vkDestroyImageView(renderer.device, renderer.targetView, nullptr);
vkDestroyImage(renderer.device, renderer.targetImage, nullptr);

// store the new size
renderer.width = width;
renderer.height = height;

// recreate the target image and view
createTargetImage(renderer);
```

## Wrap up

The flow:

```text
dockspace -> menu bar -> panels
    -> preview panel reports size
    -> target resizes
    -> image rendered and fitted
```

The interface is a resizable workspace around the live preview.
