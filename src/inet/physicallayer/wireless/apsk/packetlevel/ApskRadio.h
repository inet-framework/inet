//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_APSKRADIO_H
#define __INET_APSKRADIO_H

#include "inet/common/Protocol.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/ApskModulationBase.h"
#include "inet/physicallayer/wireless/common/base/packetlevel/FlatRadioBase.h"
#include "inet/physicallayer/wireless/common/radio/bitlevel/ConvolutionalCode.h"

namespace inet {

namespace physicallayer {

class INET_API ApskRadio : public FlatRadioBase
{
  protected:
    const Protocol *protocol = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual b computePaddingLength(b length, const ConvolutionalCode *forwardErrorCorrection, const ApskModulationBase *modulation) const;
    virtual const ApskModulationBase *getModulation() const;

    virtual void handleUpperPacket(Packet *packet) override;
    virtual void sendUp(Packet *packet) override;

    virtual void encapsulate(Packet *packet) const override;
    virtual void decapsulate(Packet *packet) const override;

  public:
    ApskRadio();
};

} // namespace physicallayer

} // namespace inet

#endif

