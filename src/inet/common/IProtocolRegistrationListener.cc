//
// Copyright (C) 2012 Opensim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/IProtocolRegistrationListener.h"

namespace inet {

std::set<std::pair<int, int>> protocolRegistrations;

void registerProtocol(const Protocol& protocol, cGate *gate)
{
    protocolRegistrations.insert(std::make_pair<int, int>(protocol.getId(), gate->getOwnerModule()->getId()));
    cGate* pathEndGate = gate->getPathEndGate();
    IProtocolRegistrationListener *protocolRegistration = dynamic_cast<IProtocolRegistrationListener *>(pathEndGate->getOwner());
    if (protocolRegistration != nullptr)
        protocolRegistration->handleRegisterProtocol(protocol, pathEndGate);
}

cModule *lookupProtocol(const Protocol& protocol, cGate *pathStartGate)
{
    auto pathEndGate = pathStartGate->getPathEndGate();
    auto pathEndModule = pathEndGate->getOwnerModule();
//    std::cout << "PATH: " << pathStartGate->getFullPath() << " --> " << pathEndGate->getFullPath() << endl;
//    std::cout << "Lookup: " << protocol.getId() << ", " << pathEndGate->getId() << ", " << pathEndGate->getFullPath() << endl;
    auto it = protocolRegistrations.find(std::make_pair<int, int>(protocol.getId(), pathEndModule->getId()));
    if (it != protocolRegistrations.end())
        return pathEndModule;
    else {
        auto listener = dynamic_cast<IProtocolRegistrationListener *>(pathEndModule);
        if (listener != nullptr)
            return listener->handleLookupProtocol(protocol, pathEndGate);
        else
            return nullptr;
    }
}

} // namespace inet

