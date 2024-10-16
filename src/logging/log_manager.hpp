#pragma once

namespace raycast {
namespace logging {
class LogManager {
  public:
    LogManager() = default;
    ~LogManager() = default;

    void Initialize();
    void ShutDown();
};
} // namespace logging
} // namespace raycast
