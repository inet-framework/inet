//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ERRORMODELBASE_H
#define __INET_ERRORMODELBASE_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IErrorModel.h"

namespace inet {

namespace physicallayer {

class INET_API ErrorModelBase : public cModule, public IErrorModel
{
  public:
    enum class CorruptionMode {
        CM_UNDEFINED = -1,
        CM_PACKET,
        CM_CHUNK,
        CM_BYTE,
        CM_BIT
    };

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
    virtual bool hasProbabilisticError(b length, double ber) const;

    virtual Packet *corruptBits(const Packet *packet, double ber, bool& isCorrupted) const;
    virtual Packet *corruptBytes(const Packet *packet, double ber, bool& isCorrupted) const;
    virtual Packet *corruptChunks(const Packet *packet, double ber, bool& isCorrupted) const;
    virtual Packet *corruptPacket(const Packet *packet, bool& isCorrupted) const;

    virtual Packet *computeCorruptedPacket(const Packet *packet, double ber) const;
    virtual Packet *computeCorruptedPacket(const ISnir *snir) const override;
};

} // namespace physicallayer

} // namespace inet

#endif

