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

#ifndef __INET_PROTOCOLPRINTERREGISTRY_H_
#define __INET_PROTOCOLPRINTERREGISTRY_H_

#include "inet/common/Protocol.h"
#include "inet/common/packet/printer/ProtocolPrinter.h"

namespace inet {

#define Register_Protocol_Printer(PROTOCOL, CLASSNAME) EXECUTE_ON_STARTUP(ProtocolPrinterRegistry::globalRegistry.registerProtocolPrinter(PROTOCOL, new CLASSNAME()));

class INET_API ProtocolPrinterRegistry
{
  public:
    static ProtocolPrinterRegistry globalRegistry;

  protected:
    std::map<const Protocol *, const ProtocolPrinter *> protocolPrinters;

  public:
    ~ProtocolPrinterRegistry();

    void registerProtocolPrinter(const Protocol* protocol, const ProtocolPrinter *printer);

    const ProtocolPrinter *findProtocolPrinter(const Protocol* protocol) const;
    const ProtocolPrinter *getProtocolPrinter(const Protocol* protocol) const;
};

} // namespace

#endif // #ifndef __INET_PROTOCOLPRINTERREGISTRY_H_

