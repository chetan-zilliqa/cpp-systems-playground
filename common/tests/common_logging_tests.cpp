#include <cassert>
#include "common/logging.hpp"

using namespace common;

int main() {
    // Default level should be INFO
    assert(global_log_level == LogLevel::Info);

    // Change level to Debug and verify
    set_log_level(LogLevel::Debug);
    assert(global_log_level == LogLevel::Debug);

    // Change level to Error and verify
    set_log_level(LogLevel::Error);
    assert(global_log_level == LogLevel::Error);

    // Basic smoke test of logging calls (no output verification)
    LOG_DEBUG("This is a debug message (may be suppressed)");
    LOG_INFO("This is an info message");
    LOG_WARN("This is a warning");
    LOG_ERROR("This is an error");

    return 0;
}
