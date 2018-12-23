// Copyright 2018 leoetlino <leo@leolam.fr>
// Licensed under GPLv2+
#pragma once

#if BYML_ENABLE_DEBUG_LOGGING
#include <memory>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#endif

namespace byml::common::log {

#if BYML_ENABLE_DEBUG_LOGGING
extern std::shared_ptr<spdlog::logger> gLogger;
#endif

#if BYML_ENABLE_DEBUG_LOGGING
#define GENERIC_LOG(level, ...)                                                                    \
  do {                                                                                             \
    if (gLogger->should_log(level))                                                                \
      gLogger->log(level, "{}::{}():{}: {}", __FILE__, __FUNCTION__, __LINE__,                     \
                   fmt::format(__VA_ARGS__));                                                      \
  } while (false)
#else
#define GENERIC_LOG(level, ...)                                                                    \
  do {                                                                                             \
  } while (false)
#endif

#define DEBUG_LOG(...) GENERIC_LOG(spdlog::level::debug, __VA_ARGS__)
#define INFO_LOG(...) GENERIC_LOG(spdlog::level::info, __VA_ARGS__)
#define WARN_LOG(...) GENERIC_LOG(spdlog::level::warn, __VA_ARGS__)
#define ERR_LOG(...) GENERIC_LOG(spdlog::level::err, __VA_ARGS__)
#define CRIT_LOG(...) GENERIC_LOG(spdlog::level::critical, __VA_ARGS__)

}  // namespace byml::common::log
