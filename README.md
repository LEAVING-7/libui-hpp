# libui-hpp

A header-only C++ wrapper around [libui-ng](https://github.com/libui-ng/libui-ng) with a fluent widget API.

libui-hpp wraps the C API of libui-ng in `namespace ui`, using RAII-friendly value types, method chaining, and variadic helpers for layout. The library ships as a single header (`include/ui/ui.hpp`) and a CMake `INTERFACE` target named `ui`.

## Features

- Header-only: `#include <ui/ui.hpp>` and link against libui-ng
- Chainable widget API (`Window`, `Button`, `Tab`, `VerticalBox`, and more)
- Variadic box constructors for concise layout code
- Cross-platform via libui-ng (Windows, macOS, Linux)

## Requirements

- CMake 3.20 or newer
- A C++17 compiler
- [Meson](https://mesonbuild.com/) and Ninja (used to build libui-ng on first configure)
- Platform dependencies for libui-ng:
  - **Windows**: MSVC
  - **macOS**: Xcode command-line tools
  - **Linux**: GTK 3 development packages (`pkg-config`, `gtk+-3.0`)

On the first CMake configure, libui-ng is fetched automatically via `FetchContent` and built as a static library.

## Quick Start

```bash
cmake -B build
cmake --build build --config Release
```

Run the widget gallery example:

```bash
# Windows
build\examples\Release\widget-gallery.exe

# Linux / macOS
./build/examples/widget-gallery
```

To build only the library target without examples:

```bash
cmake -B build -DLIBUI_HPP_BUILD_EXAMPLES=OFF
```

## Using in Your Project

Add this repository to your build and link the `ui` target:

```cmake
add_subdirectory(path/to/libui-hpp)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE ui)
```

```cpp
#include <ui/ui.hpp>
```

The `ui` target exports the include path and links libui-ng and its platform libraries.

## Example

A minimal tabbed window in the style of the widget gallery:

```cpp
#include <cstdio>

#include <ui/ui.hpp>

int main() {
  std::string err;
  if (!ui::Application::init(&err)) {
    fprintf(stderr, "error initializing libui: %s\n", err.c_str());
    return 1;
  }

  ui::Window window = ui::Window::make("Widget Gallery", 480, 320, false);
  window.on_closing(
      [](uiWindow *, void *) {
        ui::Application::quit();
        return 1;
      },
      nullptr);

  ui::Tab tab = ui::Tab::make();
  window.set_child(tab).margined(true);

  tab.append("Basic Controls",
             ui::VerticalBox::make(ui::Button::make("Button"),
                                   ui::Label::make("Hello from libui-hpp."),
                                   ui::Spinbox::make(0, 100))
                 .padded(true));
  tab.set_margined(0, true);

  window.show();
  ui::Application::run();
  ui::Application::uninit();
  return 0;
}
```

## Examples

This repository includes two demo applications:

| Target | Source | Description |
|--------|--------|-------------|
| `widget-gallery` | `examples/widget_gallery.cpp` | Control gallery with tabs, dialogs, and a borderless popup |
| `table-demo` | `examples/table_demo.cpp` | Table widget with custom model and cell types |

Build them with the default CMake options (`LIBUI_HPP_BUILD_EXAMPLES=ON`).

## Project Layout

```
include/ui/ui.hpp   Public header
examples/           Demo applications
cmake/              libui-ng fetch and build helpers
windows/            Windows manifest resources for examples
```

## License

See the libui-ng project for the underlying UI library license. Add a license file here if you plan to distribute this wrapper separately.
