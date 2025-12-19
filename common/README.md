# Common Module

The `common` project provides lightweight shared utilities used across the
monorepo. At the moment, it contains a simple but flexible logging facility
with log-level support.

## Features

- Header-only `common` library
- Minimal dependency footprint
- Logging API with levels:
  - DEBUG
  - INFO
  - WARN
  - ERROR
- Simple macros for convenience

## Example Usage

```cpp
#include "common/logging.hpp"

int main() {
    common::set_log_level(common::LogLevel::Debug);

    LOG_INFO("Application started");
    LOG_DEBUG("Debug message");
    LOG_ERROR("Something went wrong!");

    return 0;
}
