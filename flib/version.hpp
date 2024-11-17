// Copyright © 2020-2024 Luka Arnecic.
// See the LICENSE file at the top-level directory of this distribution.

#pragma once

// Library major version
#define FLIB_VERSION_MAJOR 0

// Library minor version - max 999
#define FLIB_VERSION_MINOR 7

// Library patch version - max 99
#define FLIB_VERSION_PATCH 2

// Library version represented with a single number
//
//   Major version: FLIB_VERSION / 100000
//   Minor version: FLIB_VERSION / 100 % 1000
//     Patch level: FLIB_VERSION % 100
#define FLIB_VERSION FLIB_VERSION_MAJOR * 100000 + FLIB_VERSION_MINOR * 100 + FLIB_VERSION_PATCH