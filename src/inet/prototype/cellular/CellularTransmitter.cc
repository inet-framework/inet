//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/prototype/cellular/CellularTransmitter.h"

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/wireless/common/analogmodel/dimensional/DimensionalTransmissionAnalogModel.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"
#include "inet/prototype/cellular/CellularTransmission.h"
#include "inet/prototype/cellular/PanelAntenna.h"
#include "inet/prototype/cellular/SharedAntennaRef.h"

namespace inet {

namespace cellular {

using namespace inet::math;

Define_Module(CellularTransmitter);

void CellularTransmitter::initialize(int stage)
{
    TransmitterBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        grid = CarrierGrid(Hz(par("centerFrequency")), Hz(par("subcarrierSpacing")), par("numPrbs"));
        power = W(par("power"));
    }
}

std::ostream& CellularTransmitter::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "CellularTransmitter";
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FIELD(centerFrequency, grid.getCenterFrequency())
               << EV_FIELD(numPrbs, grid.getNumPrbs())
               << EV_FIELD(power);
    return stream;
}

Ptr<const IFunction<WpHz, Domain<simsec, Hz>>> CellularTransmitter::createPowerFunction(simtime_t startTime, simtime_t endTime, const AllocationReq *allocationReq) const
{
    // the lattice holds sample points, and a left interpolator gives each cell the value of its
    // lower corner, so N cells need N + 1 samples along each axis, the last ones unused
    int numSymbols = allocationReq->getNumSymbols();
    int numPrbs = grid.getNumPrbs();
    int sizeX = numSymbols + 1;
    int sizeY = numPrbs + 1;
    std::vector<WpHz> values(sizeX * sizeY, WpHz(0));
    WpHz powerSpectralDensity = allocationReq->getPowerPerPrb() / grid.getPrbWidth();
    for (size_t i = 0; i < allocationReq->getPrbsArraySize(); i++) {
        int prb = allocationReq->getPrbs(i);
        ASSERT(0 <= prb && prb < numPrbs);
        for (int symbol = 0; symbol < numSymbols; symbol++)
            values[sizeX * prb + symbol] = powerSpectralDensity;
    }
    Hz lowerEdge = grid.getLowerEdge();
    return makeShared<PeriodicallyInterpolated2DFunction<WpHz, simsec, Hz>>(simsec(startTime), simsec(endTime), sizeX,
            lowerEdge, lowerEdge + grid.getBandwidth(), sizeY,
            LeftInterpolator<simsec, WpHz>::singleton, LeftInterpolator<Hz, WpHz>::singleton, values);
}

const ITransmission *CellularTransmitter::createTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime) const
{
    auto allocationReq = packet->getTag<AllocationReq>();
    if (allocationReq->getNumSymbols() <= 0 || allocationReq->getPrbsArraySize() == 0)
        throw cRuntimeError("The allocation of packet %s is empty", packet->getName());
    W totalPower = allocationReq->getPowerPerPrb() * allocationReq->getPrbsArraySize();
    if (totalPower > power * (1 + 1E-9))
        throw cRuntimeError("The allocation of packet %s needs %g W, more than the maximum of %g W", packet->getName(), totalPower.get(), power.get());
    simtime_t endTime = startTime + grid.getSymbolDuration() * allocationReq->getNumSymbols();
    // workaround for C15 (the transmitter analog model gets no packet): the transmitter builds
    // the analog model itself, because the power spectral density depends on the allocation
    auto analogModel = new DimensionalTransmissionAnalogModel(0, 0, endTime - startTime, grid.getCenterFrequency(), grid.getBandwidth(), createPowerFunction(startTime, endTime, allocationReq.get()));
    auto antennaRef = check_and_cast<const SharedAntennaRef *>(transmitter->getAntenna());
    Ptr<const IAntennaGain> gain;
    if (allocationReq->getBeam() == -1)
        gain = antennaRef->getGain();
    else
        gain = check_and_cast<const PanelAntenna *>(antennaRef->getAntenna())->getBeamGain(allocationReq->getBeam());
    IMobility *mobility = antennaRef->getMobility();
    std::vector<int> prbs(allocationReq->getPrbsArraySize());
    for (size_t i = 0; i < prbs.size(); i++)
        prbs[i] = allocationReq->getPrbs(i);
    return new CellularTransmission(transmitter, packet, startTime, endTime, mobility->getCurrentPosition(), mobility->getCurrentAngularPosition(), analogModel, gain, grid, prbs, allocationReq->getNumSymbols(), allocationReq->getMcs(), allocationReq->getIntendedReceiverId());
}

} // namespace cellular

} // namespace inet

