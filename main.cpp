// Copyright (c) 2013, Florian Wininger, Etienne Bergeron
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
//
// ETW2CTF translates a trace file from ETW (Event Tracing for Windows) trace
// file format to the CTF (Common Trace Format) trace file format.
//
// ETW (Event Tracing for windows) provides the ability to trace Windows kernel
// and user-space application for debugging or profiling. ETL files are binary
// encoded file format, but ETW provides an API to retrieve the events format.
//
// See: http://msdn.microsoft.com/en-us/library/windows/desktop/bb968803(v=vs.85).aspx
//
// The performance SDK (an optional include from the Microsoft SDK) provides
// performance analysis tools to gather ETW traces. The SDK also installs some
// manifest to describe performance events.
//
// See: http://msdn.microsoft.com/en-us/performance/cc825801.aspx
//
// Common Trace Format is a self-describing file format used for tracing tools
// interoperability.
//
// See: http://www.efficios.com/ctf

#include <iostream>
#include <string>

#include "CTFProducer.h"
#include "ETWConsumer.h"
#include "Metadata.h"

namespace {

using etw2ctf::Metadata;

etw2ctf::ETWConsumer consumer;
etw2ctf::CTFProducer producer;

void WINAPI ProcessEvent(PEVENT_RECORD pevent) {
  assert(pevent != NULL);

  Metadata::Packet packet;

  if (!consumer.ProcessEvent(pevent, &packet))
    return;

  // Write the packet into the stream.
  producer.Write(packet.raw_bytes(), packet.size());
}

ULONG WINAPI ProcessBuffer(PEVENT_TRACE_LOGFILEW ptrace) {
  assert(ptrace != NULL);

  // Close the previous stream.
  producer.CloseStream();

  // Open the next buffer.
  std::wstring stream_name;
  if (!consumer.GetBufferName(ptrace, &stream_name))
    return FALSE;
  producer.OpenStream(stream_name);

  // Encode and Write stream header.
  Metadata::Packet packet;
  consumer.ProcessHeader(&packet);
  producer.Write(packet.raw_bytes(), packet.size());

  if (!consumer.ProcessBuffer(ptrace))
    return FALSE;

  return TRUE;
}

}  // namespace

int wmain(int argc, wchar_t** argv) {
  // TODO(bergeret): command line parsing.
  producer.OpenFolder(L"ctf");

  for (int i = 1; i < argc; ++i) {
    std::wstring filename(argv[i]);
    consumer.AddTraceFile(filename);
  }

  // No trace files to consume.
  if (consumer.Empty())
    return 0;

  consumer.SetEventCallback(ProcessEvent);
  consumer.SetBufferCallback(ProcessBuffer);

  // Consume trace files.
  producer.OpenStream(L"stream");

  // Encode and Write stream header.
  // The stream header must always be generated here because it is possible
  // to process an empty trace, without any buffer.
  Metadata::Packet packet;
  consumer.ProcessHeader(&packet);
  producer.Write(packet.raw_bytes(), packet.size());

  // Consume all events. The ETW API will call our registered callbacks on
  // each buffer and each event. Callbacks forward the processing to the
  // consumer via ProcessEvent and ProcessBuffer. After the processing of each
  // event by the consumer, the packet (encoded event) is written to the
  // producer.
  if (!consumer.ConsumeAllEvents()) {
    std::wcerr << L"Could not consume traces files." << std::endl;
    return -1;
  }
  producer.CloseStream();

  // Serialize the metadata build during events processing.
  producer.OpenStream(L"metadata");
  std::string metadata;
  if (!consumer.SerializeMetadata(&metadata))
    return -1;
  if (!producer.Write(metadata.c_str(), metadata.size()))
    return -1;
  producer.CloseStream();

  return 0;
}
