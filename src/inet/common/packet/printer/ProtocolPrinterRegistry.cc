//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"

namespace inet {

ProtocolPrinterRegistry::~ProtocolPrinterRegistry()
{
    for (auto it : protocolPrinters)
        delete it.second;
}

void ProtocolPrinterRegistry::registerProtocolPrinter(const Protocol *protocol, const ProtocolPrinter *printer)
{
    protocolPrinters[protocol] = printer;
}

const ProtocolPrinter *ProtocolPrinterRegistry::findProtocolPrinter(const Protocol *protocol) const
{
    auto it = protocolPrinters.find(protocol);
    if (it != protocolPrinters.end())
        return it->second;
    else
        return nullptr;
}

const ProtocolPrinter *ProtocolPrinterRegistry::getProtocolPrinter(const Protocol *protocol) const
{
    auto it = protocolPrinters.find(protocol);
    if (it != protocolPrinters.end())
        return it->second;
    else
        throw cRuntimeError("Cannot find protocol printer for %s", protocol->getName());
}

ProtocolPrinterRegistry& ProtocolPrinterRegistry::getInstance()
{
    static int handle = cSimulationOrSharedDataManager::registerSharedVariableName("inet::ProtocolPrinterRegistry::instance");
    return getSimulationOrSharedDataManager()->getSharedVariable<ProtocolPrinterRegistry>(handle);
}

} // namespace

