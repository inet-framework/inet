//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ERRORMODELBASE_H
#define __INET_ERRORMODELBASE_H

#include "inet/common/Module.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IErrorModel.h"

namespace inet {

namespace physicallayer {

class INET_API ErrorModelBase : public Module, public IErrorModel
{
  public:
    enum class SnirMode {
        SM_UNDEFINED = -1,
        SM_MIN,
        SM_MEAN
    };

  protected:
    CorruptionMode corruptionMode = CorruptionMode::CM_UNDEFINED;
    SnirMode snirMode = SnirMode::SM_UNDEFINED;
    double snirOffset = NaN;

  protected:
    virtual void initialize(int stage) override;

    virtual double getScalarSnir(const ISnir *snir) const;
    virtual double getScalarSnir(const ISnir *snir, simtime_t startTime, simtime_t endTime) const;
    virtual void checkRegionPart(IRadioSignal::SignalPart part) const;
    virtual bool hasProbabilisticError(b length, double ber) const;

    virtual Packet *corruptBits(const Packet *packet, double ber, bool& isCorrupted) const;
    virtual Packet *corruptBytes(const Packet *packet, double ber, bool& isCorrupted) const;
    virtual Packet *corruptChunks(const Packet *packet, double ber, bool& isCorrupted) const;
    virtual Packet *corruptPacket(const Packet *packet, bool& isCorrupted) const;

    virtual Packet *computeCorruptedPacket(const Packet *packet, double ber) const;
    virtual Packet *computeCorruptedPacket(const ISnir *snir) const override;

    /**
     * Returns the received form of a failed region of the packet in the
     * representation of the corruption mode.
     */
    virtual Ptr<const Chunk> computeCorruptedChunk(const Packet *packet, const ErrorRegion& region) const;

    /**
     * Corrupts the units of a failed region of numUnits units: corruptUnit()
     * receives the index of each unit with an error. Each unit has an error
     * with the provided unit error rate, and at least one unit has an error,
     * because the region failed.
     */
    virtual void corruptUnits(int numUnits, double unitErrorRate, const std::function<void(int)>& corruptUnit) const;

  public:
    using IErrorModel::computePacketErrorRate;
    virtual double computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part, b length, simtime_t startTime, simtime_t endTime) const override;

    virtual CorruptionMode getCorruptionMode() const override { return corruptionMode; }
    virtual Packet *computeCorruptedPacket(const Packet *packet, const std::vector<ErrorRegion>& regions) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

