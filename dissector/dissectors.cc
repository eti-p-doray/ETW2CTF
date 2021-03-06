// Copyright (c) 2013 The ETW2CTF Authors.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//   * Neither the name of the <organization> nor the
//     names of its contributors may be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dissector/dissectors.h"

#include <cassert>

namespace dissector {

// Head of a linked list of registered dissectors.
static Dissector* dissectors = NULL;

Dissector::Dissector(const char *name, const char *descr)
    : name_(name), descr_(descr), next_(NULL) {
  assert(name != NULL);
  assert(descr != NULL);

  // Register the dissector in the global linked list.
  this->next_ = dissectors;
  dissectors = this;
}

bool DecodeEventWithDissectors(const GUID& guid,
                               uint8_t opcode,
                               char* payload,
                               uint32_t payload_length,
                               converter::Metadata::Packet* packet,
                               converter::Metadata::Event* descr) {
  assert(packet != NULL);
  assert(descr != NULL);

  Dissector* it = dissectors;
  size_t payload_position = packet->size();

  while (it != NULL) {
    // Try to decode using this dissector.
    if (it->DecodeEvent(guid, opcode, payload, payload_length, packet, descr))
      return true;

    // Reset the packet state before decoding failure.
    descr->Reset();
    packet->Reset(payload_position);

    // Move to the next dissector.
    it = it->next();
  }

  return false;
}

}  // namespace dissector
