#pragma once
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/spdlog.h"

#define RAYCAST_DEFAULT_LOGGER_NAME "RaycastLogger"
#ifndef RAYCAST_CONFIG_RELEASE
#define LOG_TRACE(...)                                                         \
                                                                               \
    if (spdlog::get(RAYCAST_DEFAULT_LOGGER_NAME) != nullptr) {                 \
        SPDLOG_LOGGER_TRACE(spdlog::get(RAYCAST_DEFAULT_LOGGER_NAME),          \
                            __VA_ARGS__);                                      \
    }
#define LOG_DEBUG(...)                                                         \
                                                                               \
    if (spdlog::get(RAYCAST_DEFAULT_LOGGER_NAME) != nullptr) {                 \
        SPDLOG_LOGGER_DEBUG(spdlog::get(RAYCAST_DEFAULT_LOGGER_NAME),          \
                            __VA_ARGS__);                                      \
    }
#define LOG_INFO(...)                                                          \
                                                                               \
    if (spdlog::get(RAYCAST_DEFAULT_LOGGER_NAME) != nullptr) {                 \
        SPDLOG_LOGGER_INFO(spdlog::get(RAYCAST_DEFAULT_LOGGER_NAME),           \
                           __VA_ARGS__);                                       \
    }
#define LOG_WARN(...)                                                          \
                                                                               \
    if (spdlog::get(RAYCAST_DEFAULT_LOGGER_NAME) != nullptr) {                 \
        SPDLOG_LOGGER_WARN(spdlog::get(RAYCAST_DEFAULT_LOGGER_NAME),           \
                           __VA_ARGS__);                                       \
    }
#define LOG_ERROR(...)                                                         \
                                                                               \
    if (spdlog::get(RAYCAST_DEFAULT_LOGGER_NAME) != nullptr) {                 \
        SPDLOG_LOGGER_ERROR(spdlog::get(RAYCAST_DEFAULT_LOGGER_NAME),          \
                            __VA_ARGS__);                                      \
    }
#define LOG_CRITICAL(...)                                                      \
                                                                               \
    if (spdlog::get(RAYCAST_DEFAULT_LOGGER_NAME) != nullptr) {                 \
        SPDLOG_LOGGER_CRITICAL(spdlog::get(RAYCAST_DEFAULT_LOGGER_NAME),       \
                               __VA_ARGS__);                                   \
    }
#else
#define LOG_TRACE(...) (void)0
#define LOG_DEBUG(...) (void)0
#define LOG_INFO(...) (void)0
#define LOG_WARN(...) (void)0
#define LOG_ERROR(...) (void)0
#define LOG_FATAL(...) (void)0
#endif
