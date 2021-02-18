#pragma once

#ifndef RUSH_PLATFORM_HPP
#define RUSH_PLATFORM_HPP

#if defined(__linux__)
#  define RUSH_PLATFORM_LINUX 1
#endif

#if defined(_WIN32) || defined(WIN32)
#  define RUSH_PLATFORM_WINDOWS 1
#endif

#endif // RUSH_PLATFORM_HPP
