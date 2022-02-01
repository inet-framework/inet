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
    const bps bitrate;

  public:
    SignalPacketModel(const Packet *packet, bps bitrate);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;
    virtual const Packet *getPacket() const override { return packet; }
    virtual bps getBitrate() const override { return bitrate; }
};

class INET_API TransmissionPacketModel : public SignalPacketModel, public virtual ITransmissionPacketModel
{
  public:
    TransmissionPacketModel(const Packet *packet, bps bitrate);
};

class INET_API ReceptionPacketModel : public SignalPacketModel, public IReceptionPacketModel
{
  protected:
    double packetErrorRate;

  public:
    ReceptionPacketModel(const Packet *packet, bps bitrate, double packetErrorRate);

    virtual double getPacketErrorRate() const override { return packetErrorRate; }
};

} // namespace physicallayer
} // namespace inet

#endif

