#include "logging/log_manager.hpp"
#include "logging/log.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <iostream>

namespace raycast {
namespace logging {
/**
 * Initializes our spdlog logger instance. spdlog has the concept of "sinks",
 * which are targets for log output (i.e files, stdout, stderr, etc.). The
 * current default "RaycastLogger" is configured to log to stdout and a log
 * file. Notice that we use stdout_color_sink_st, (st meaning single-threaded)
 * since I do not expect to have multi-threaded code yet.
 */
void LogManager::Initialize() {
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e %^%l%$][%s %!] %v");
    std::vector<spdlog::sink_ptr> sinks{consoleSink};
    auto logger = std::make_shared<spdlog::logger>(RAYCAST_DEFAULT_LOGGER_NAME,
                                                   sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);
    spdlog::register_logger(logger);

    std::cout
        << "\n"
        << R"('########:::::'###::::'##:::'##::'######:::::'###:::::'######::'########:
 ##.... ##:::'## ##:::. ##:'##::'##... ##:::'## ##:::'##... ##:... ##..::
 ##:::: ##::'##:. ##:::. ####::: ##:::..:::'##:. ##:: ##:::..::::: ##::::
 ########::'##:::. ##:::. ##:::: ##:::::::'##:::. ##:. ######::::: ##::::
 ##.. ##::: #########:::: ##:::: ##::::::: #########::..... ##:::: ##::::
 ##::. ##:: ##.... ##:::: ##:::: ##::: ##: ##.... ##:'##::: ##:::: ##::::
 ##:::. ##: ##:::: ##:::: ##::::. ######:: ##:::: ##:. ######::::: ##::::
..:::::..::..:::::..:::::..::::::......:::..:::::..:::......::::::..:::::)"
        << "\n\n"
        << "A puzzle escape game about reflecting and bending light."
        << "\n"
        << "Developed by Lightbox Studios (Team 03)" << "\n"
        << std::endl;
}

/**
 * As the name suggests, cleans up resources for all registered loggers and
 * flushes any logs still in the buffer.
 */
void LogManager::ShutDown() { spdlog::shutdown(); }
} // namespace logging
} // namespace raycast
