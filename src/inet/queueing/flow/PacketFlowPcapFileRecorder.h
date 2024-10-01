//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PACKETFLOWPCAPFILERECORDER_H
#define __INET_PACKETFLOWPCAPFILERECORDER_H

#include "inet/common/packet/recorder/IPcapWriter.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {
namespace queueing {

class INET_API PacketFlowPcapFileRecorder : public PacketFlowBase
{
  protected:
    IPcapWriter *pcapWriter = nullptr;
    Direction direction = DIRECTION_UNDEFINED;
    PcapLinkType networkType = LINKTYPE_INVALID;

  protected:
    virtual void initialize(int stage) override;
    virtual void finish() override;


  public:
    virtual void processPacket(Packet *packet) override;
};

} // namespace queueing
} // namespace inet

#endif

