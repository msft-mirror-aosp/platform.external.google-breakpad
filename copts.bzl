# Copyright 2024 - The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the',  help='License');
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an',  help='AS IS' BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Common compiler options and functions to compile BREAKPAD.."""

# Keep in sync with config("unicode") from Chromium's build/config/win/BUILD.gn:
# https://source.chromium.org/chromium/chromium/src/+/main:build/config/win/BUILD.gn;l=570;drc=804d5a91d49d0ad79d3d5529e6ba2610225cfe55
BREAKPAD_APPLE_DEFS = [
    "HAVE_ARC4RANDOM",
    "HAVE_CXX7",
    "HAVE_GETCONTEXT",
    "HAVE_INTTYPES_H",
    "HAVE_PTHREAD",
    "HAVE_STDINT_H",
    "HAVE_STDIO_H",
    "HAVE_STDLIB_H",
    "HAVE_STRING_H",
    "HAVE_STRINGS_H",
    "HAVE_SYS_MMAN_H",
    "HAVE_SYS_RANDOM_H",
    "HAVE_SYS_STAT_H",
    "HAVE_SYS_TYPES_H",
    "HAVE_UNISTD_H",
    "HAVE_MACH_O_NLIST_H",
    "_THREAD_SAFE",
    "STDC_HEADERS",
]

BREAKPAD_APPLE_COPTS = [
    "-Wmissing-braces",
    "-Wnon-virtual-dtor",
    "-Woverloaded-virtual",
    "-Wreorder",
    "-Wsign-compare",
    "-Wunused-local-typedefs",
    "-Wunused-variable",
    "-Wvla",
    "-UNDEBUG",
    "-Wno-error",
    "-std=c++20",
]

BREAKPAD_LINUX_DEFS = [
    "HAVE_A_OUT_H",
    "HAVE_CXX17",
    "HAVE_GETCONTEXT",
    "HAVE_GETRANDOM",
    "HAVE_INTTYPES_H",
    "HAVE_MEMFD_CREATE",
    "HAVE_PTHREAD",
    "HAVE_STDINT_H",
    "HAVE_STDIO_H",
    "HAVE_STDLIB_H",
    "HAVE_STRINGS_H",
    "HAVE_STRING_H",
    "HAVE_SYS_MMAN_H",
    "HAVE_SYS_STAT_H",
    "HAVE_SYS_TY",
    # Older glibc elf.h might not yet define the ELF compression
    # types.
    "SHF_COMPRESSED=2048",
    # We do not have the linux headers that specify EM_RISCV, see:
    # https://elixir.bootlin.com/linux/latest/source/include/uapi/linux/elf-em.h#L51
    # For the proper kernel definitions.
    "EM_RISCV=243",
]

BREAKPAD_LINUX_COPTS = [
    "-Wno-fortify-source",
    "-std=c++20",
]

BREAKPAD_WINDOWS_DEFS = [
    "_CRT_NONSTDC_NO_DEPRECATE",
    "_CRT_NONSTDC_NO_WARNINGS",
    "_CRT_RAND_S",
    "_CRT_SECURE_NO_DEPRECATE",
    "_HAS_EXCEPTIONS=0",
    "_SECURE_ATL",
    "_UNICODE",
    "_WIN32_WINNT=0x0600",
    "_WINDOWS",
    "NOMINMAX",
    "UNICODE",
    "WIN32",
    "WIN32_LEAN_AND_MEAN",
    "WINVER=0x0600",
]

BREAKPAD_WINDOWS_COPTS = ["/std=c++20"]

BREAKPAD_COMMON_COPTS = select({
    "@platforms//os:windows": BREAKPAD_WINDOWS_COPTS,
    "@platforms//os:linux": BREAKPAD_LINUX_COPTS,
    "@platforms//os:macos": BREAKPAD_APPLE_COPTS,
    "//conditions:default": ["-std=c++20"],
})

BREAKPAD_COMMON_DEFS = select({
    "@platforms//os:windows": BREAKPAD_WINDOWS_DEFS,
    "@platforms//os:linux": BREAKPAD_LINUX_DEFS,
    "@platforms//os:macos": BREAKPAD_APPLE_DEFS,
    "//conditions:default": [],
})
