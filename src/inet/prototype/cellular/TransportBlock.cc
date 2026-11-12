//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/prototype/cellular/TransportBlock.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/prototype/cellular/AllocationReq_m.h"
#include "inet/prototype/cellular/CarrierGrid.h"
#include "inet/prototype/cellular/McsTable.h"
#include "inet/prototype/cellular/TransportBlockHeader_m.h"

namespace inet {

namespace cellular {

const Protocol TransportBlock::protocol("cellulartransportblock", "Cellular transport block", Protocol::LinkLayer);

B TransportBlock::computeSize(int numPrbs, int numSymbols, int mcs)
{
    double bits = numPrbs * CarrierGrid::SUBCARRIERS_PER_PRB * numSymbols * McsTable::getSpectralEfficiency(mcs);
    B headerLength = TransportBlockHeader().getChunkLength();
    return std::max(B((int64_t)(bits / 8)), headerLength + B(1));
}

Packet *TransportBlock::create(const char *name, int ueIndex, int carrierIndex, int mcs, const std::vector<int>& prbs, int numSymbols, W powerPerPrb, int beam, int intendedReceiverId)
{
    auto packet = new Packet(name);
    auto header = makeShared<TransportBlockHeader>();
    header->setUeIndex(ueIndex);
    header->setCarrierIndex(carrierIndex);
    header->setMcs(mcs);
    B size = computeSize(prbs.size(), numSymbols, mcs);
    packet->insertAtBack(header);
    packet->insertAtBack(makeShared<ByteCountChunk>(size - header->getChunkLength()));
    packet->addTag<PacketProtocolTag>()->setProtocol(&protocol);
    auto allocationReq = packet->addTag<AllocationReq>();
    allocationReq->setPrbsArraySize(prbs.size());
    for (size_t i = 0; i < prbs.size(); i++)
        allocationReq->setPrbs(i, prbs[i]);
    allocationReq->setNumSymbols(numSymbols);
    allocationReq->setPowerPerPrb(powerPerPrb);
    allocationReq->setBeam(beam);
    allocationReq->setIntendedReceiverId(intendedReceiverId);
    allocationReq->setMcs(mcs);
    return packet;
}

} // namespace cellular

} // namespace inet

