//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PROTOCOLDISSECTORREGISTRY_H
#define __INET_PROTOCOLDISSECTORREGISTRY_H

#include "inet/common/Protocol.h"
#include "inet/common/packet/dissector/ProtocolDissector.h"

namespace inet {

#define Register_Protocol_Dissector(PROTOCOL, CLASSNAME)    EXECUTE_ON_STARTUP(ProtocolDissectorRegistry::globalRegistry.registerProtocolDissector(PROTOCOL, new CLASSNAME()));

class INET_API ProtocolDissectorRegistry
{
  public:
    static ProtocolDissectorRegistry globalRegistry;

  protected:
    std::map<const Protocol *, const ProtocolDissector *> protocolDissectors;

  public:
    ~ProtocolDissectorRegistry();

    void registerProtocolDissector(const Protocol *protocol, const ProtocolDissector *dissector);

    const ProtocolDissector *findProtocolDissector(const Protocol *protocol) const;
    const ProtocolDissector *getProtocolDissector(const Protocol *protocol) const;
};

} // namespace

#endif

