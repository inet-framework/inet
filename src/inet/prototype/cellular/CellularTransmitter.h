//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CELLULARTRANSMITTER_H
#define __INET_CELLULARTRANSMITTER_H

#include "inet/common/math/Functions.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/TransmitterBase.h"
#include "inet/prototype/cellular/AllocationReq_m.h"
#include "inet/prototype/cellular/CarrierGrid.h"

namespace inet {

namespace cellular {

using namespace inet::physicallayer;

/**
 * Sends one allocation per packet on one component carrier. The packet's AllocationReq names
 * the PRBs, the symbols, the power per PRB, the beam and the intended receiver.
 */
class INET_API CellularTransmitter : public TransmitterBase
{
  protected:
    CarrierGrid grid;
    W power = W(NaN);

  protected:
    virtual void initialize(int stage) override;

    // the power spectral density of the allocation, one lattice cell per (symbol, PRB)
    virtual Ptr<const math::IFunction<WpHz, math::Domain<simsec, Hz>>> createPowerFunction(simtime_t startTime, simtime_t endTime, const AllocationReq *allocationReq) const;

  public:
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    const CarrierGrid& getGrid() const { return grid; }
    virtual W getMaxPower() const override { return power; }

    virtual const ITransmission *createTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime) const override;
};

} // namespace cellular

} // namespace inet

#endif

