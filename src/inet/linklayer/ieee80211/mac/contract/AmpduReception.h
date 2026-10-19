// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __INET_AMPDURECEPTION_H
#define __INET_AMPDURECEPTION_H

#include <memory>
#include <optional>

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mac/aggregation/MpduAggregationUtils.h"

namespace inet {
namespace ieee80211 {

// These facts describe one received PSDU. BlockAckRecord remains the receipt owner.
struct INET_API AmpduTopology
{
    struct Member {
        B offset;
        std::optional<SequenceNumberCyclic> sequence;
        bool first = false;
    };
    std::vector<MpduRange> ranges;
    std::vector<Member> members;
    B length = B(0);
    MacAddress transmitter;
    MacAddress receiver;
    Tid tid = -1;
};

struct INET_API AmpduReception
{
    struct Member {
        std::unique_ptr<Packet> packet;
        B offset;
    };
    std::shared_ptr<const AmpduTopology> topology;
    std::vector<Member> members;
    bool anyCorrect = false;
};

// HCF supplies an immutable context for one member or one standalone MPDU.
struct INET_API BlockAckReceiveContext
{
    std::shared_ptr<const AmpduTopology> aggregate;
    std::shared_ptr<const AmpduTopology> previousAggregate;
    B memberOffset = B(0);
    bool firstMember = false;
    bool followsAggregate = false;
    bool followsIndividual = false;
};

} // namespace ieee80211
} // namespace inet

#endif
