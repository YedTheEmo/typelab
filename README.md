# typelab

A terminal-based learning resource for learning by doing: search, pick, and type your way through lesson files right in the terminal. Built on the typing engine from [ctyper](https://github.com/YedTheEmo/ctyper), adapted to live inside a full application loop.

## How it works

1. Boot into a **lesson picker** — browse folders, type to filter, pick a lesson, press Enter.
2. The **typing session** loads the chosen `.md` lesson: type every character exactly as shown.
3. Pressing **Ctrl+C** inside a typing session returns you to the picker — the whole app stays in one terminal. Ctrl+C in the picker quits typelab entirely.

## Features

- **Lesson picker TUI**: browse folders in a tree with breadcrumbs, live search within the current folder, arrow / page-up / page-down navigation, blue selection highlight.
- **Nested lesson tree**: lessons are organized in subfolders so the catalog can grow (`official/graphics/vulkan`, `official/webdev/nextjs`, …).
- **Section navigation**: lesson files use markdown-style `#` headings as section markers. Press **Ctrl+D** to skip the current section and jump to the next heading; press **Ctrl+A** to jump back to the previous heading.
- **Skip a line**: press **Enter** to mark the rest of the current line as skipped and move on.
- **Reset a line** (Windows): press **Shift+Enter** to clear the current line's green/red markings and return to its first character.
- **Auto-skip leading indentation**: leading spaces/tabs on a line are auto-typed for you, and a single `Backspace` clears the whole indent.
- **Visual feedback**: green = correct, red = mistyped.
- **UTF-8 aware** input and rendering.

## Lesson layout

```
lessons/
  official/          bundled lessons (tracked in git)
    !getting-started/01-hello.md
    graphics/vulkan/01-vulkan-overview/concept.md
    graphics/vulkan/01-vulkan-overview/code.md
    graphics/vulkan/02-vulkan-instance-and-device/concept.md
    graphics/vulkan/02-vulkan-instance-and-device/code.md
    graphics/vulkan/...
    webdev/nextjs/...
    webdev/tanstack-start/...
  ext/               your own externally-supplied lessons (git-ignored)
```

Each lesson is a folder named with a numeric prefix so the folders sort into course order. Inside the folder, `concept.md` explains the ideas and `code.md` is the program to type. Drop any `.md` file anywhere under `lessons/` and it appears when you browse into its folder.

### Pinning folders

A folder whose name starts with `!` sorts to the top of its parent (above other folders), and the `!` is hidden in the picker. This is useful for a designated first-stop like `!getting-started` without imposing an order on unrelated tracks.

### Section markers

Any line whose first non-whitespace character is `#` starts a new section:

```
# Welcome

Some text to type.

## Controls

More text to type.
```

## Building

### Windows (MSVC)

```powershell
.\build.bat
```

Or manually:
```cmd
cl /std:c++17 /O2 /EHsc src\main.cpp /Fe:typelab.exe
```

### Linux / macOS

```bash
g++ -std=c++17 -O2 src/main.cpp -o typelab
```

## Usage

```powershell
.\typelab.exe
```

Options:

| Flag | Description |
|------|-------------|
| `-l, --lessons <dir>` | Use a different lessons root (default: `lessons`) |
| `-n, --lines <num>`   | Max lines shown per screen (default: 50) |
| `-a, --audio`         | Beep on mistyped characters |
| `-h, --help`          | Show usage |

## Controls

### Picker

- Browse into a folder with `Enter`; folders are listed first, lessons after
- `..` (or `Backspace` / `←` with an empty search) moves up a folder
- Type to search within the current folder (matches lesson/folder names)
- `↑` / `↓` move · `PgUp` / `PgDn` page · `Home` / `End` jump
- `Esc` clears the current search
- `Enter` opens a lesson
- `Ctrl+C` / `Ctrl+Q` quit typelab

### Typing session

- Type the text exactly as shown
- `Backspace` move back & edit (one press clears an auto-skipped indent)
- `Enter` skip the rest of the current line
- `Shift+Enter` reset the current line: clear its green/red markings and jump back to the first character (Windows)
- `Ctrl+D` skip the current section (jump to next `#` heading)
- `Ctrl+A` jump back to the previous `#` heading
- `Ctrl+C` / `Ctrl+Q` exit the session and return to the picker

## License

MIT
