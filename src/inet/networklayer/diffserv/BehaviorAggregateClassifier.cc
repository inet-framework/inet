//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/networklayer/diffserv/BehaviorAggregateClassifier.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/diffserv/DiffservUtil.h"

#ifdef INET_WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#endif // ifdef INET_WITH_IPv4

#ifdef INET_WITH_IPv6
#include "inet/networklayer/ipv6/Ipv6Header.h"
#endif // ifdef INET_WITH_IPv6

#ifdef INET_WITH_TCP_COMMON
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#endif // ifdef INET_WITH_TCP_COMMON

#ifdef INET_WITH_UDP
#include "inet/transportlayer/udp/UdpHeader_m.h"
#endif // ifdef INET_WITH_UDP

namespace inet {

using namespace DiffservUtil;

Define_Module(BehaviorAggregateClassifier);

simsignal_t BehaviorAggregateClassifier::pkClassSignal = registerSignal("pkClass");

bool BehaviorAggregateClassifier::PacketDissectorCallback::matches(Packet *packet)
{
    dissect = true;
    matches_ = false;
    PacketDissector packetDissector(ProtocolDissectorRegistry::getInstance(), *this);
    packetDissector.dissectPacket(packet);
    return matches_;
}

void BehaviorAggregateClassifier::PacketDissectorCallback::visitChunk(const Ptr<const Chunk>& chunk, const Protocol *protocol)
{
    if (protocol == nullptr)
        return;
    if (*protocol == Protocol::ipv4) {
        dissect = false;
#ifdef INET_WITH_IPv4
        const auto& ipv4Header = dynamicPtrCast<const Ipv4Header>(chunk);
        if (!ipv4Header)
            return;
        dscp = ipv4Header->getDscp();
        matches_ = true;
#endif // ifdef INET_WITH_IPv4
    }
    else if (*protocol == Protocol::ipv6) {
        dissect = false;
#ifdef INET_WITH_IPv6
        const auto& ipv6Header = dynamicPtrCast<const Ipv6Header>(chunk);
        if (!ipv6Header)
            return;
        dscp = ipv6Header->getDscp();
        matches_ = true;
#endif // ifdef INET_WITH_IPv6
    }
}

void BehaviorAggregateClassifier::initialize(int stage)
{
    PacketClassifierBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        numOutGates = gateSize("out");
        std::vector<int> dscps;
        parseDSCPs(par("dscps"), "dscps", dscps);
        int numDscps = (int)dscps.size();
        if (numDscps > numOutGates)
            throw cRuntimeError("%d dscp values are given, but the module has only %d out gates",
                    numDscps, numOutGates);
        for (int i = 0; i < numDscps; ++i)
            dscpToGateIndexMap[dscps[i]] = i;

        numRcvd = 0;
        WATCH(numRcvd);
    }
}

void BehaviorAggregateClassifier::pushPacket(Packet *packet, cGate *inputGate)
{
    EV_INFO << "Classifying packet " << packet->getName() << ".\n";
    numRcvd++;
    int index = classifyPacket(packet);
    emit(pkClassSignal, index);
    if (index >= 0)
        pushOrSendPacket(packet, outputGates[index], consumers[index]);
    else {
        auto defaultOutputGate = gate("defaultOut");
        auto defaultConsumer = findConnectedModule<IPassivePacketSink>(defaultOutputGate);
        pushOrSendPacket(packet, defaultOutputGate, defaultConsumer);
    }
}

void BehaviorAggregateClassifier::refreshDisplay() const
{
    char buf[20] = "";
    if (numRcvd > 0)
        sprintf(buf + strlen(buf), "rcvd:%d ", numRcvd);
    getDisplayString().setTagArg("t", 0, buf);
}

int BehaviorAggregateClassifier::classifyPacket(Packet *packet)
{
    PacketDissectorCallback callback;

    if (callback.matches(packet)) {
        int dscp = callback.dscp;
        auto it = dscpToGateIndexMap.find(dscp);
        if (it != dscpToGateIndexMap.end())
            return it->second;
    }
    return -1;
}

} // namespace inet

