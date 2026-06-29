//
// Protocol Test Framework for INET -- Phase 0: normalised packet event.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_PROTOCOLTEST_PACKETEVENT_H
#define __INET_PROTOCOLTEST_PACKETEVENT_H

#include <string>

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"

namespace inet {
namespace protocoltest {

// What kind of observation this is, derived from the emitting signal.
enum class EventKind {
    SentToLower,
    ReceivedFromLower,
    SentToUpper,
    ReceivedFromUpper,
    SentToPeer,
    ReceivedFromPeer,
    Dropped,
    Other
};

// Coarse protocol-stack layer, derived (best-effort in Phase 0) from the
// emitting module's position. Refined later when selectors need it.
enum class Layer { Unknown, Physical, Link, Network, Transport, Application };

// A single normalised observation produced from a packet signal. This is the
// atom the matching engine (later phases) consumes; in Phase 0 it is only logged.
struct PacketEvent {
    const Packet *packet = nullptr;   // the observed packet (not owned)
    EventKind kind = EventKind::Other;
    int direction = -1;               // DirectionTag value if present, else -1
    cModule *node = nullptr;          // containing network node, if resolvable
    cComponent *module = nullptr;     // emitting component
    int interfaceId = -1;             // from Interface{Ind,Req} tag, else -1
    std::string interfaceName;        // resolved from interfaceId, else empty
    std::string protocolName;         // packet's PacketProtocolTag protocol name, else empty
    Layer layer = Layer::Unknown;
    simtime_t time = 0;
    long treeId = -1;                 // packet identity, for cross-node correlation
};

const char *getEventKindName(EventKind kind);
const char *getLayerName(Layer layer);

} // namespace protocoltest
} // namespace inet

#endif
