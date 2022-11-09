//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETSERVER_H
#define __INET_PACKETSERVER_H

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketServerBase.h"

namespace inet {

extern template class ClockUserModuleMixin<queueing::PacketServerBase>;

namespace queueing {

class INET_API PacketServer : public ClockUserModuleMixin<PacketServerBase>
{
  protected:
    cMessage *serveTimer = nullptr;
    ClockEvent *processingTimer = nullptr;
    Packet *packet = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;
    virtual void scheduleProcessingTimer();
    virtual bool canStartProcessingPacket();
    virtual void startProcessingPacket();
    virtual void endProcessingPacket();

  public:
    virtual ~PacketServer();

    virtual void handleCanPushPacketChanged(cGate *gate) override;
    virtual void handleCanPullPacketChanged(cGate *gate) override;

    virtual std::string resolveDirective(char directive) const override;
};

} // namespace queueing
} // namespace inet

#endif

