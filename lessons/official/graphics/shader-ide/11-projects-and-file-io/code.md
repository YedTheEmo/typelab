# Projects and file I/O - typing

This lesson types the file layer: read and write shader files, manage a list
of shader slots, switch between them with tabs, select the entry point, and
autosave after a successful compile.

## Define a shader slot

A slot holds everything the editor needs for one shader.

```cpp
#include <string>
#include <vector>

// one open shader
struct ShaderSlot {
    // the file path, empty for a new shader
    std::string path;

    // the source text being edited
    std::string source;

    // the selected entry point
    std::string entryPoint = "main";

    // the saved copy, for change detection
    std::string savedSource;

    // true when the source differs from the file
    bool dirty = false;
};
```

## Read a file

Loading a shader is reading the file into a string.

```cpp
#include <fstream>
#include <sstream>

// read a text file into a string
bool readFile(
    const std::string& path,
    std::string& contents
) {
    // open the file for reading
    std::ifstream file(path);

    // reject an unreadable file
    if (!file.is_open()) {
        return false;
    }

    // copy the file contents into a string
    std::ostringstream buffer;
    buffer << file.rdbuf();
    contents = buffer.str();

    return true;
}
```

## Write a file

Saving is writing the buffer text to disk.

```cpp
// write a string to a text file
bool writeFile(
    const std::string& path,
    const std::string& contents
) {
    // open the file for writing
    std::ofstream file(path);

    // reject an unwritable file
    if (!file.is_open()) {
        return false;
    }

    // write the contents
    file << contents;

    return true;
}
```

## Load a shader into a slot

The buffer is filled and marked for recompile.

```cpp
// load a shader file into a slot
bool loadShaderSlot(ShaderSlot& slot, const std::string& path) {
    // read the file contents
    std::string contents;
    if (!readFile(path, contents)) {
        return false;
    }

    // store the path and the source
    slot.path = path;
    slot.source = contents;
    slot.savedSource = contents;

    // the loaded shader is not dirty
    slot.dirty = false;

    return true;
}
```

## Save a shader from a slot

The source is written and the saved copy is refreshed.

```cpp
// save a shader slot to its file
bool saveShaderSlot(ShaderSlot& slot) {
    // reject a shader without a file path
    if (slot.path.empty()) {
        return false;
    }

    // write the source to the file
    if (!writeFile(slot.path, slot.source)) {
        return false;
    }

    // refresh the saved copy
    slot.savedSource = slot.source;

    // the shader is now clean
    slot.dirty = false;

    return true;
}
```

## Mark a slot dirty

Edits update the dirty flag from the saved copy.

```cpp
// update the dirty flag from the saved source
void refreshDirty(ShaderSlot& slot) {
    // compare the source with the saved copy
    slot.dirty = slot.source != slot.savedSource;
}
```

## The shader manager

The manager owns the slot list and the active index.

```cpp
// the collection of open shaders
struct ShaderManager {
    // the open shader slots
    std::vector<ShaderSlot> slots;

    // the index of the active shader
    size_t active = 0;

    // return the active slot
    ShaderSlot& activeSlot() {
        return slots[active];
    }
};
```

## Draw the tab bar

Tabs switch between open shaders.

```cpp
#include <imgui.h>

// draw the shader tab bar
void drawShaderTabs(ShaderManager& manager) {
    // draw a tab for each slot
    for (size_t i = 0; i < manager.slots.size(); i++) {
        // build the tab label
        std::string label = manager.slots[i].path;

        // mark dirty shaders with a star
        if (manager.slots[i].dirty) {
            label += " *";
        }

        // draw the tab and detect a selection
        if (ImGui::TabItem(label.c_str())) {
            // switch the active shader
            manager.active = i;
        }
    }

    // draw the new shader button
    if (ImGui::Button("+ New")) {
        // create a fresh empty slot
        ShaderSlot fresh;
        fresh.entryPoint = "main";
        fresh.source = "";
        manager.slots.push_back(fresh);
        manager.active = manager.slots.size() - 1;
    }
}
```

## Draw the entry point selector

The combo box changes the active shader's entry point.

```cpp
// draw the entry point selector
void drawEntryPointSelector(ShaderManager& manager) {
    // read the active slot
    ShaderSlot& slot = manager.activeSlot();

    // draw the combo box label
    ImGui::Text("entry point");

    // draw the combo box
    if (ImGui::BeginCombo("##entry", slot.entryPoint.c_str())) {
        // offer the common entry points
        const char* choices[] = { "main", "fragment", "compute" };

        // draw each choice
        for (const char* choice : choices) {
            // draw the selectable item
            if (ImGui::Selectable(choice)) {
                // store the selection
                slot.entryPoint = choice;
            }
        }

        // close the combo box
        ImGui::EndCombo();
    }
}
```

## Autosave after a successful compile

A successful compile saves the active shader.

```cpp
// autosave the active shader after a successful compile
void autosaveActive(ShaderManager& manager) {
    // read the active slot
    ShaderSlot& slot = manager.activeSlot();

    // skip shaders without a file
    if (slot.path.empty()) {
        return;
    }

    // write the source to the file
    saveShaderSlot(slot);
}
```

## Wire the File menu

The menu commands drive the manager.

```cpp
// draw the File menu
void drawFileMenu(ShaderManager& manager) {
    // open the File menu
    if (ImGui::BeginMenu("File")) {
        // save the active shader
        if (ImGui::MenuItem("Save")) {
            saveShaderSlot(manager.activeSlot());
        }

        // mark the dirty state
        refreshDirty(manager.activeSlot());

        // close the File menu
        ImGui::EndMenu();
    }
}
```

## Now type it again

Reconstruct the load and save pair.

```cpp
// load a shader file into a slot
bool loadShaderSlot(ShaderSlot& slot, const std::string& path) {
    std::string contents;
    if (!readFile(path, contents)) {
        return false;
    }
    slot.path = path;
    slot.source = contents;
    slot.savedSource = contents;
    slot.dirty = false;
    return true;
}

// save a shader slot to its file
bool saveShaderSlot(ShaderSlot& slot) {
    if (slot.path.empty()) {
        return false;
    }
    if (!writeFile(slot.path, slot.source)) {
        return false;
    }
    slot.savedSource = slot.source;
    slot.dirty = false;
    return true;
}
```

Then reconstruct the change detection.

```cpp
// update the dirty flag from the saved source
void refreshDirty(ShaderSlot& slot) {
    slot.dirty = slot.source != slot.savedSource;
}
```

## Wrap up

The flow:

```text
file -> read -> slot -> buffer -> edit -> write -> file
    and compile success -> autosave
```

Shaders now persist as plain files and switch like documents.
