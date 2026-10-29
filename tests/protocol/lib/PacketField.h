//
// Protocol Test Framework for INET -- read and write a "protocol.field" path, and evaluate
// an expression over a packet or an object.
//
// Lets captures be written declaratively, e.g. capture("isn", "tcp.sequenceNo"),
// instead of with a C++ lambda. Uses the same dissection + class-descriptor path as
// INET's PacketFilter.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_PACKETFIELD_H
#define __INET_PROTOCOLTEST_PACKETFIELD_H

#include <map>
#include <string>

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"

namespace inet {
namespace protocoltest {

// The values that the captures of a program bound, by name. An expression names one as
// {name}.
typedef std::map<std::string, cValue> CaptureStore;

// Evaluate "protocol.field" (e.g. "tcp.sequenceNo", "udp.srcPort") or "ClassName.field"
// against a packet and return the field value. Throws if the protocol/field is absent.
cValue evalPacketField(const Packet *packet, const std::string& path);

// Replaces each {name} of the text with the value of that capture. Converts only the
// captures that the text names. Returns false when the text names a capture that is not
// set yet; the text then still holds that {name}.
bool substituteCaptures(std::string& text, const CaptureStore& captures);

// Evaluates an expression and returns its value. The expression can name what a filter
// names ("tcp.sequenceNo + 100000", "Ipv4Header.timeToLive"), the captures ("{isn} + 1"),
// `length`, the length of the packet in bytes, and `value`, the value of a scalar signal.
// The packet may be null for an expression that names no field. Throws when a name has no
// value, and when a capture is not set yet.
cValue evalPacketExpression(const std::string& expression, const CaptureStore& captures, const Packet *packet,
                            const cValue& value = cValue());

// Evaluates an expression whose names are the fields of an object: of a module, and of the
// objects that its fields point to ("state.snd_cwnd >= 14600", "fsmState"). The expression
// can name the captures too.
cValue evalObjectExpression(const std::string& expression, const CaptureStore& captures, cObject *object);

// Writes a field of the chunk that evalPacketField() reads for the same path, and repairs
// what the change makes wrong: the checksum of each IPv4 header and of each TCP header in
// the packet, as the mode of each checksum asks. Throws if the protocol or the field is
// absent, or if the packet holds that chunk in another form than the dissector shows (for
// example in bytes).
void setPacketField(Packet *packet, const std::string& path, const cValue& value);

} // namespace protocoltest
} // namespace inet

#endif
