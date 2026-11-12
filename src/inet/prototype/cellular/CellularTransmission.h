//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CELLULARTRANSMISSION_H
#define __INET_CELLULARTRANSMISSION_H

#include "inet/physicallayer/wireless/common/base/packetlevel/TransmissionBase.h"
#include "inet/prototype/cellular/CarrierGrid.h"

namespace inet {

namespace cellular {

using namespace inet::physicallayer;

/**
 * One allocation: the PRBs and OFDM symbols of one carrier that one transmitter sends to one
 * intended receiver, with one beam. Its analog model is a power spectral density lattice with
 * one cell per (symbol, PRB) of the carrier.
 */
class INET_API CellularTransmission : public TransmissionBase
{
  protected:
    const CarrierGrid grid;
    const std::vector<int> prbs;
    const int numSymbols;
    const int mcs;
    const int intendedReceiverId;

  public:
    CellularTransmission(const IRadio *transmitterRadio, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const Coord& position, const Quaternion& orientation, const ITransmissionAnalogModel *analogModel, const Ptr<const IAntennaGain>& beamGain, const CarrierGrid& grid, const std::vector<int>& prbs, int numSymbols, int mcs, int intendedReceiverId);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    const CarrierGrid& getGrid() const { return grid; }
    const std::vector<int>& getPrbs() const { return prbs; }
    int getNumSymbols() const { return numSymbols; }
    int getMcs() const { return mcs; }
    int getIntendedReceiverId() const { return intendedReceiverId; }
};

} // namespace cellular

} // namespace inet

#endif

