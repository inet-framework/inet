//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PROTOCOLPRINTERREGISTRY_H
#define __INET_PROTOCOLPRINTERREGISTRY_H

#include "inet/common/Protocol.h"
#include "inet/common/packet/printer/ProtocolPrinter.h"

namespace inet {

#define Register_Protocol_Printer(PROTOCOL, CLASSNAME)    EXECUTE_ON_STARTUP(ProtocolPrinterRegistry::globalRegistry.registerProtocolPrinter(PROTOCOL, new CLASSNAME()));

class INET_API ProtocolPrinterRegistry
{
  public:
    static ProtocolPrinterRegistry globalRegistry;

  protected:
    std::map<const Protocol *, const ProtocolPrinter *> protocolPrinters;

  public:
    ~ProtocolPrinterRegistry();

    void registerProtocolPrinter(const Protocol *protocol, const ProtocolPrinter *printer);

    const ProtocolPrinter *findProtocolPrinter(const Protocol *protocol) const;
    const ProtocolPrinter *getProtocolPrinter(const Protocol *protocol) const;
};

} // namespace

#endif

