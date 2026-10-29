//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/linklayer/common/PcpTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
#include "inet/linklayer/ieee8021q/Ieee8021qTagHeader_m.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

class PcpTestRequester : public queueing::PacketFlowBase
{
  protected:
    int numPackets = 0;

  protected:
    int getValue(const char *name)
    {
        auto values = check_and_cast<cValueArray *>(par(name).objectValue());
        return numPackets < values->size() ? values->get(numPackets).intValue() : -1;
    }

    virtual void processPacket(Packet *packet) override
    {
        bool sTag = par("sTag");
        packet->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::ipv4);
        packet->addTag<CVlanReq>()->setVlanId(10);
        if (sTag)
            packet->addTag<SVlanReq>()->setVlanId(100);
        if (int userPriority = getValue("userPriorities"); userPriority != -1)
            packet->addTag<UserPriorityReq>()->setUserPriority(userPriority);
        if (int pcp = getValue("cPcps"); pcp != -1)
            packet->addTag<CPcpReq>()->setPcp(pcp);
        if (int pcp = getValue("sPcps"); pcp != -1)
            packet->addTag<SPcpReq>()->setPcp(pcp);
        if (par("epd")) {
            auto encapsulationProtocolReq = packet->addTag<EncapsulationProtocolReq>();
            encapsulationProtocolReq->appendProtocol(&Protocol::ieee8021qCTag);
            if (sTag)
                encapsulationProtocolReq->appendProtocol(&Protocol::ieee8021qSTag);
        }
        numPackets++;
    }
};

Define_Module(PcpTestRequester);

class PcpTestProbe : public queueing::PacketFlowBase
{
  protected:
    int numPackets = 0;

  protected:
    void printTag(const char *name, const TagBase *tag, int value)
    {
        std::cout << " " << name << "=";
        if (tag == nullptr)
            std::cout << "none";
        else
            std::cout << value;
    }

    virtual void processPacket(Packet *packet) override
    {
        std::cout << getParentModule()->getFullName() << " #" << numPackets++ << ":";
        if (par("tpid")) {
            const auto& header = packet->peekAtFront<Ieee8021qTagTpidHeader>();
            std::cout << " pcp=" << (int)header->getPcp();
        }
        else {
            b offset = b(0);
            for (int i = 0; i < (int)par("numHeaders"); i++) {
                const auto& header = packet->peekDataAt<Ieee8021qTagEpdHeader>(offset);
                std::cout << " pcp" << i << "=" << (int)header->getPcp();
                offset += header->getChunkLength();
            }
        }
        auto userPriorityReq = packet->findTag<UserPriorityReq>();
        auto userPriorityInd = packet->findTag<UserPriorityInd>();
        auto cPcpReq = packet->findTag<CPcpReq>();
        auto sPcpReq = packet->findTag<SPcpReq>();
        auto cPcpInd = packet->findTag<CPcpInd>();
        auto sPcpInd = packet->findTag<SPcpInd>();
        printTag("UserPriorityReq", userPriorityReq.get(), userPriorityReq ? userPriorityReq->getUserPriority() : 0);
        printTag("UserPriorityInd", userPriorityInd.get(), userPriorityInd ? userPriorityInd->getUserPriority() : 0);
        printTag("CPcpReq", cPcpReq.get(), cPcpReq ? cPcpReq->getPcp() : 0);
        printTag("SPcpReq", sPcpReq.get(), sPcpReq ? sPcpReq->getPcp() : 0);
        printTag("CPcpInd", cPcpInd.get(), cPcpInd ? cPcpInd->getPcp() : 0);
        printTag("SPcpInd", sPcpInd.get(), sPcpInd ? sPcpInd->getPcp() : 0);
        std::cout << "\n";
    }
};

Define_Module(PcpTestProbe);

} // namespace inet
