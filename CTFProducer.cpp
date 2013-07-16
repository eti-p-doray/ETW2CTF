/******************************************************************************
Copyright (c) 2013, Florian Wininger, Etienne Bergeron
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#include <cassert>
#include <sstream>
#include <string>

#include "CTFProducer.h"

// Some missing definition and typedef from the windows API.
typedef unsigned int BOOL;
#define FALSE 0
#define TRUE 1
#define WINAPI __stdcall
typedef const wchar_t* LPCWSTR;
typedef struct _SECURITY_ATTRIBUTES *LPSECURITY_ATTRIBUTES;

// Import functions from windows API.
extern "C" BOOL WINAPI CreateDirectoryW(
    LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);

namespace etw2ctf {

bool CTFProducer::OpenFolder(const std::wstring& folder) {
  if (!folder_.empty() || folder.empty())
    return false;
  folder_ = folder;
  if (CreateDirectoryW(folder_.c_str(), NULL) == FALSE)
    return false;
  return TRUE;
}

bool CTFProducer::OpenStream(const std::wstring& filename) {
  assert(!stream_.is_open());

  std::wstringstream ss;
  ss << folder_ << L"\\" << filename;

  stream_.open(ss.str(),
      std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
  return stream_.good();
}

bool CTFProducer::CloseStream() {
  if (!stream_.is_open())
    return false;
  stream_.close();
  return !stream_.fail();
}

bool CTFProducer::Write(const char* raw, size_t length) {
  if (!stream_.is_open())
    return false;
  assert(stream_.good());
  stream_.write(raw, length);

  return true;
}

std::ofstream& CTFProducer::stream() {
  assert(stream_.good());
  return stream_;
}

}  //namespace etw2ctf
