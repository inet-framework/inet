//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STREAMENCODER_H
#define __INET_STREAMENCODER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API StreamEncoder : public PacketFlowBase, public TransparentProtocolRegistrationListener
{
  protected:
    class INET_API Mapping
    {
      public:
        int vlanId = -1;
        int pcp = -1;
        std::string stream;
    };

    std::vector<Mapping> mappings;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void processPacket(Packet *packet) override;

    virtual void configureMappings();
    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;
};

} // namespace inet

#endif

