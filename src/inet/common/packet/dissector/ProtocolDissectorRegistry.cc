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

#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"

namespace inet {

ProtocolDissectorRegistry ProtocolDissectorRegistry::globalRegistry;

ProtocolDissectorRegistry::~ProtocolDissectorRegistry()
{
    for (auto it : protocolDissectors)
        delete it.second;
}

void ProtocolDissectorRegistry::registerProtocolDissector(const Protocol* protocol, const ProtocolDissector *dissector)
{
    protocolDissectors[protocol] = dissector;
}

const ProtocolDissector *ProtocolDissectorRegistry::findProtocolDissector(const Protocol* protocol) const
{
    auto it = protocolDissectors.find(protocol);
    if (it != protocolDissectors.end())
        return it->second;
    else
        return nullptr;
}

const ProtocolDissector *ProtocolDissectorRegistry::getProtocolDissector(const Protocol* protocol) const
{
    auto protocolDissector = findProtocolDissector(protocol);
    if (protocolDissector != nullptr)
        return protocolDissector;
    else
        throw cRuntimeError("Cannot find protocol dissector for %s", protocol->getName());
}

} // namespace
