//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STREAMMERGER_H
#define __INET_STREAMMERGER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketFilterBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API StreamMerger : public PacketFilterBase, public TransparentProtocolRegistrationListener
{
  protected:
    cValueMap *mapping = nullptr;
    int bufferSize = -1;

    std::map<std::string, std::vector<int>> sequenceNumbers;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void processPacket(Packet *packet) override;
    virtual bool matchesPacket(const Packet *packet) const override;

    virtual bool matchesInputStream(const char *streamName) const;
    virtual bool matchesSequenceNumber(const char *streamName, int sequenceNumber) const;

    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;
};

} // namespace inet

#endif

