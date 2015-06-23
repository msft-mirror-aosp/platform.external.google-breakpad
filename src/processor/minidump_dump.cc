// Copyright (c) 2006, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// minidump_dump.cc: Print the contents of a minidump file in somewhat
// readable text.
//
// Author: Mark Mentovai

#include <stdio.h>
#include <string.h>

#include "google_breakpad/processor/minidump.h"
#include "processor/logging.h"
#include "processor/scoped_ptr.h"

namespace {

using google_breakpad::Minidump;
using google_breakpad::MinidumpThreadList;
using google_breakpad::MinidumpModuleList;
using google_breakpad::MinidumpMemoryList;
using google_breakpad::MinidumpException;
using google_breakpad::MinidumpAssertion;
using google_breakpad::MinidumpSystemInfo;
using google_breakpad::MinidumpMiscInfo;
using google_breakpad::MinidumpBreakpadInfo;

// These are additional minidump stream values which are specific to the linux
// breakpad implementation.
enum {
  MD_LINUX_CPU_INFO              = 0x47670003,    /* /proc/cpuinfo    */
  MD_LINUX_PROC_STATUS           = 0x47670004,    /* /proc/$x/status  */
  MD_LINUX_LSB_RELEASE           = 0x47670005,    /* /etc/lsb-release */
  MD_LINUX_CMD_LINE              = 0x47670006,    /* /proc/$x/cmdline */
  MD_LINUX_ENVIRON               = 0x47670007,    /* /proc/$x/environ */
  MD_LINUX_AUXV                  = 0x47670008     /* /proc/$x/auxv    */
};

bool LoadStreamContents(Minidump *minidump,
                        u_int32_t stream_type,
                        u_int32_t *length,
                        char **contents,
                        int *errors) {
  if (!minidump->SeekToStreamType(stream_type, length) || *length == 0) {
    return false;
  }
  google_breakpad::scoped_array<char> buffer(new char[*length + 1]);
  if (!minidump->ReadBytes(&buffer[0], *length)) {
    ++errors;
    BPLOG(ERROR) << "minidump.ReadBytes failed";
    return false;
  }
  buffer[*length] = '\0';

  *contents = buffer.release();
  return true;
}

static void DumpStringArray(Minidump *minidump,
                            u_int32_t stream_type,
                            const char *stream_name,
                            int *errors) {
  u_int32_t length = 0;
  char *contents;
  if (!LoadStreamContents(minidump, stream_type, &length, &contents, errors))
    return;
  google_breakpad::scoped_array<char> buffer(contents);
  printf("String array stream %s:\n", stream_name);
  const char *current_string = &buffer[0];
  int string_number = 0;
  // Loop through strings.  Note that the buffer is always NULL
  // terminated, so we will eventually land on the NULL character,
  // exiting the loop.
  while (current_string - &buffer[0] < length) {
    printf("%2d: %s\n", string_number, current_string);
    ++string_number;
    current_string += strlen(current_string) + 1;
  }
  printf("\n");
}

static void DumpRawStream(Minidump *minidump,
                          u_int32_t stream_type,
                          const char *stream_name,
                          int *errors) {
  u_int32_t length = 0;
  char *contents;
  if (!LoadStreamContents(minidump, stream_type, &length, &contents, errors))
    return;

  google_breakpad::scoped_array<char> buffer(contents);
  printf("Raw stream %s:\n%s\n", stream_name, &buffer[0]);
}

static bool PrintMinidumpDump(const char *minidump_file) {
  Minidump minidump(minidump_file);
  if (!minidump.Read()) {
    BPLOG(ERROR) << "minidump.Read() failed";
    return false;
  }
  minidump.Print();

  int errors = 0;

  MinidumpThreadList *thread_list = minidump.GetThreadList();
  if (!thread_list) {
    ++errors;
    BPLOG(ERROR) << "minidump.GetThreadList() failed";
  } else {
    thread_list->Print();
  }

  MinidumpModuleList *module_list = minidump.GetModuleList();
  if (!module_list) {
    ++errors;
    BPLOG(ERROR) << "minidump.GetModuleList() failed";
  } else {
    module_list->Print();
  }

  MinidumpMemoryList *memory_list = minidump.GetMemoryList();
  if (!memory_list) {
    ++errors;
    BPLOG(ERROR) << "minidump.GetMemoryList() failed";
  } else {
    memory_list->Print();
  }

  MinidumpException *exception = minidump.GetException();
  if (!exception) {
    BPLOG(INFO) << "minidump.GetException() failed";
  } else {
    exception->Print();
  }

  MinidumpAssertion *assertion = minidump.GetAssertion();
  if (!assertion) {
    BPLOG(INFO) << "minidump.GetAssertion() failed";
  } else {
    assertion->Print();
  }

  MinidumpSystemInfo *system_info = minidump.GetSystemInfo();
  if (!system_info) {
    ++errors;
    BPLOG(ERROR) << "minidump.GetSystemInfo() failed";
  } else {
    system_info->Print();
  }

  MinidumpMiscInfo *misc_info = minidump.GetMiscInfo();
  if (!misc_info) {
    ++errors;
    BPLOG(ERROR) << "minidump.GetMiscInfo() failed";
  } else {
    misc_info->Print();
  }

  MinidumpBreakpadInfo *breakpad_info = minidump.GetBreakpadInfo();
  if (!breakpad_info) {
    // Breakpad info is optional, so don't treat this as an error.
    BPLOG(INFO) << "minidump.GetBreakpadInfo() failed";
  } else {
    breakpad_info->Print();
  }

  DumpStringArray(&minidump,
                  MD_LINUX_CMD_LINE,
                  "MD_LINUX_CMD_LINE",
                  &errors);
  DumpStringArray(&minidump,
                  MD_LINUX_ENVIRON,
                  "MD_LINUX_ENVIRON",
                  &errors);
  DumpRawStream(&minidump,
                MD_LINUX_LSB_RELEASE,
                "MD_LINUX_LSB_RELEASE",
                &errors);
  DumpRawStream(&minidump,
                MD_LINUX_PROC_STATUS,
                "MD_LINUX_PROC_STATUS",
                &errors);
  DumpRawStream(&minidump,
                MD_LINUX_CPU_INFO,
                "MD_LINUX_CPU_INFO",
                &errors);

  return errors == 0;
}

}  // namespace

int main(int argc, char **argv) {
  BPLOG_INIT(&argc, &argv);

  if (argc != 2) {
    fprintf(stderr, "usage: %s <file>\n", argv[0]);
    return 1;
  }

  return PrintMinidumpDump(argv[1]) ? 0 : 1;
}
