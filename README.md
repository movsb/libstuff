# esp32 & mcs51

## ebp

My own ESP components for C++.

The name is from x86:

* `esp`: The Stack Pointer.
* `ebp`: The Base  Pointer.

### How to Use

In your *CMakeLists.txt*, write down this line before `project()`:

```cmake
set(EXTRA_COMPONENT_DIRS PATH_TO_EBP)
```

Then, in your code, `include` it by typing `#include <ebp/ebp.hpp>`.

## libstc

A single source/header STC MCU library.
