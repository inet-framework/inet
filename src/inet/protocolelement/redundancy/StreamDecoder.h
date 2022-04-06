//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_STREAMDECODER_H
#define __INET_STREAMDECODER_H

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/PacketFilter.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace inet {

using namespace inet::queueing;

class INET_API StreamDecoder : public PacketFlowBase, public TransparentProtocolRegistrationListener
{
  protected:
    class INET_API Mapping
    {
      public:
        PacketFilter packetFilter;
        MacAddress source;
        MacAddress destination;
        cPatternMatcher *interfaceNameMatcher = nullptr;
        int vlanId = -1;
        int pcp = -1;
        std::string stream;
    };

  protected:
    ModuleRefByPar<IInterfaceTable> interfaceTable;
    std::vector<Mapping> mappings;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void processPacket(Packet *packet) override;

    virtual void configureMappings();
    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;

  public:
    virtual ~StreamDecoder();
};

} // namespace inet

#endif

