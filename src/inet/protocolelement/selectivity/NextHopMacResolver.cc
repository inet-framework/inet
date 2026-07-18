//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/protocolelement/selectivity/NextHopMacResolver.h"

#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/NextHopAddressTag_m.h"

namespace inet {

Define_Module(NextHopMacResolver);

void NextHopMacResolver::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        parseNeighbors(par("neighbors"));
}

// Parses a "l3address macaddress; l3address macaddress; ..." table.
void NextHopMacResolver::parseNeighbors(const char *neighborsString)
{
    cStringTokenizer entryTokenizer(neighborsString, ";");
    while (entryTokenizer.hasMoreTokens()) {
        std::vector<std::string> fields = cStringTokenizer(entryTokenizer.nextToken()).asVector();
        if (fields.empty())
            continue;
        if (fields.size() != 2)
            throw cRuntimeError("%s: invalid neighbor entry, expected 'l3address macaddress'", getFullPath().c_str());
        neighbors[L3AddressResolver().resolve(fields[0].c_str())] = MacAddress(fields[1].c_str());
    }
}

void NextHopMacResolver::processPacket(Packet *packet)
{
    const auto& nextHopReq = packet->findTag<NextHopAddressReq>();
    if (nextHopReq != nullptr) {
        auto it = neighbors.find(nextHopReq->getNextHopAddress());
        if (it == neighbors.end())
            throw cRuntimeError("%s: no MAC address configured for next hop %s",
                    getFullPath().c_str(), nextHopReq->getNextHopAddress().str().c_str());
        packet->addTagIfAbsent<MacAddressReq>()->setDestAddress(it->second);
    }
}

} // namespace inet
