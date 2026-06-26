//
// Protocol Test Framework for INET -- evaluate a "protocol.field" path to a value.
//
// Lets captures be written declaratively, e.g. capture("isn", "tcp.sequenceNo"),
// instead of with a C++ lambda. Uses the same dissection + class-descriptor path as
// INET's PacketFilter.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_PACKETFIELD_H
#define __INET_PROTOCOLTEST_PACKETFIELD_H

#include <string>

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"

namespace inet {
namespace protocoltest {

// Evaluate "protocol.field" (e.g. "tcp.sequenceNo", "udp.srcPort") or "ClassName.field"
// against a packet and return the field value. Throws if the protocol/field is absent.
cValue evalPacketField(const Packet *packet, const std::string& path);

} // namespace protocoltest
} // namespace inet

#endif
