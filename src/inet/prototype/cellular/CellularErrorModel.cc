//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/prototype/cellular/CellularErrorModel.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/physicallayer/wireless/common/analogmodel/dimensional/DimensionalSnir.h"
#include "inet/prototype/cellular/CellularTransmission.h"
#include "inet/prototype/cellular/McsTable.h"

namespace inet {

namespace cellular {

using namespace inet::math;

Define_Module(CellularErrorModel);

void CellularErrorModel::initialize(int stage)
{
    ErrorModelBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
        slope = par("slope");
}

std::ostream& CellularErrorModel::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "CellularErrorModel";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(slope);
    return stream;
}

double CellularErrorModel::computeEffectiveSnir(const ISnir *snir) const
{
    // the region of the decision has a frequency dimension, so ISnir's interval queries, which
    // cover the whole band, do not fit; the function of the dimensional SNIR does (change C14)
    auto snirFunction = check_and_cast<const DimensionalSnir *>(snir)->getSnir();
    auto reception = snir->getReception();
    auto transmission = check_and_cast<const CellularTransmission *>(reception->getTransmission());
    const auto& grid = transmission->getGrid();
    const auto& prbs = transmission->getPrbs();
    Hz prbWidth = grid.getPrbWidth();
    Hz lowerEdge = grid.getLowerEdge();
    double sum = 0;
    // one mean per contiguous run of PRBs, weighted by the length of the run
    for (size_t first = 0; first < prbs.size();) {
        size_t last = first;
        while (last + 1 < prbs.size() && prbs[last + 1] == prbs[last] + 1)
            last++;
        Point<simsec, Hz> startPoint(simsec(reception->getStartTime()), lowerEdge + prbWidth * prbs[first]);
        Point<simsec, Hz> endPoint(simsec(reception->getEndTime()), lowerEdge + prbWidth * (prbs[last] + 1));
        Interval<simsec, Hz> interval(startPoint, endPoint, 0b11, 0b00, 0b00);
        sum += snirFunction->getMean(interval) * (last - first + 1);
        first = last + 1;
    }
    return sum / prbs.size();
}

double CellularErrorModel::computeBlockErrorRate(double effectiveSnir, int mcs) const
{
    double margin = math::fraction2dB(effectiveSnir) - McsTable::getRequiredSnir(mcs);
    return 1 / (1 + std::exp(slope * margin));
}

double CellularErrorModel::computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const
{
    ASSERT(part == IRadioSignal::SIGNAL_PART_WHOLE || part == IRadioSignal::SIGNAL_PART_DATA);
    auto transmission = check_and_cast<const CellularTransmission *>(snir->getReception()->getTransmission());
    return computeBlockErrorRate(computeEffectiveSnir(snir), transmission->getMcs());
}

Packet *CellularErrorModel::computeCorruptedPacket(const ISnir *snir) const
{
    auto transmittedPacket = snir->getReception()->getTransmission()->getPacket();
    auto receivedPacket = transmittedPacket->dup();
    receivedPacket->clearTags();
    receivedPacket->addTag<PacketProtocolTag>()->setProtocol(transmittedPacket->getTag<PacketProtocolTag>()->getProtocol());
    receivedPacket->setBitError(true);
    return receivedPacket;
}

} // namespace cellular

} // namespace inet

