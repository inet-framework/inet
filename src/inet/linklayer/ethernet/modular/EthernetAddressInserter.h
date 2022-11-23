//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ETHERNETADDRESSINSERTER_H
#define __INET_ETHERNETADDRESSINSERTER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/protocolelement/base/ProtocolHeaderInserterBase.h"

namespace inet {

class INET_API EthernetAddressInserter : public ProtocolHeaderInserterBase
{
  protected:
    ModuleRefByPar<IInterfaceTable> interfaceTable;

  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
};

} // namespace inet

#endif

