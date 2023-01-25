//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIGNALPACKETMODEL_H
#define __INET_SIGNALPACKETMODEL_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalPacketModel.h"

namespace inet {
namespace physicallayer {

class INET_API SignalPacketModel : public virtual ISignalPacketModel
{
  protected:
    const Packet *packet;
    const bps headerBitrate;
    const bps dataBitrate;

  public:
    SignalPacketModel(const Packet *packet, bps headerBitrate, bps dataBitrate);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const Packet *getPacket() const override { return packet; }
    virtual bps getHeaderNetBitrate() const override { return headerBitrate; }
    virtual bps getDataNetBitrate() const override { return dataBitrate; }
};

class INET_API TransmissionPacketModel : public SignalPacketModel, public virtual ITransmissionPacketModel
{
  public:
    TransmissionPacketModel(const Packet *packet, bps headerBitrate, bps dataBitrate);
};

class INET_API ReceptionPacketModel : public SignalPacketModel, public IReceptionPacketModel
{
  protected:

  public:
    ReceptionPacketModel(const Packet *packet, bps headerBitrate, bps dataBitrate);

    virtual double getPacketErrorRate() const override { return NaN; }
};

} // namespace physicallayer
} // namespace inet

#endif

