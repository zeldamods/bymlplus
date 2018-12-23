// Copyright 2018 leoetlino <leo@leolam.fr>
// Licensed under GPLv2+

#include "common/log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace byml::common::log {

namespace {

std::shared_ptr<spdlog::logger> initLogger() {
  auto logger = spdlog::stderr_color_mt("stderr");
  spdlog::set_level(spdlog::level::trace);
  return logger;
}

}  // end of anonymous namespace

std::shared_ptr<spdlog::logger> gLogger = initLogger();

}  // namespace byml::common::log
