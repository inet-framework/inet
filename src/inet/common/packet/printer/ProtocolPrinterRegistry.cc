//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/packet/printer/ProtocolPrinterRegistry.h"

namespace inet {

ProtocolPrinterRegistry ProtocolPrinterRegistry::globalRegistry;

ProtocolPrinterRegistry::~ProtocolPrinterRegistry()
{
    for (auto it : protocolPrinters)
        delete it.second;
}

void ProtocolPrinterRegistry::registerProtocolPrinter(const Protocol* protocol, const ProtocolPrinter *printer)
{
    protocolPrinters[protocol] = printer;
}

const ProtocolPrinter *ProtocolPrinterRegistry::findProtocolPrinter(const Protocol* protocol) const
{
    auto it = protocolPrinters.find(protocol);
    if (it != protocolPrinters.end())
        return it->second;
    else
        return nullptr;
}

const ProtocolPrinter *ProtocolPrinterRegistry::getProtocolPrinter(const Protocol* protocol) const
{
    auto protocolPrinter = findProtocolPrinter(protocol);
    if (protocolPrinter != nullptr)
        return protocolPrinter;
    else
        throw cRuntimeError("Cannot find protocol printer for %s", protocol->getName());
}

} // namespace
