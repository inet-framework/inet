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

#ifndef __INET_PROTOCOLDISSECTORREGISTRY_H_
#define __INET_PROTOCOLDISSECTORREGISTRY_H_

#include "inet/common/Protocol.h"
#include "inet/common/packet/dissector/ProtocolDissector.h"

namespace inet {

#define Register_Protocol_Dissector(PROTOCOL, CLASSNAME) EXECUTE_ON_STARTUP(ProtocolDissectorRegistry::globalRegistry.registerProtocolDissector(PROTOCOL, new CLASSNAME()));

class INET_API ProtocolDissectorRegistry
{
  public:
    static ProtocolDissectorRegistry globalRegistry;

  protected:
    std::map<const Protocol *, const ProtocolDissector *> protocolDissectors;

  public:
    ~ProtocolDissectorRegistry();

    void registerProtocolDissector(const Protocol* protocol, const ProtocolDissector *dissector);

    const ProtocolDissector *findProtocolDissector(const Protocol* protocol) const;
    const ProtocolDissector *getProtocolDissector(const Protocol* protocol) const;
};

} // namespace

#endif // #ifndef __INET_PROTOCOLDISSECTORREGISTRY_H_

