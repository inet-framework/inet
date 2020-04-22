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
#include "inet/common/ModuleAccess.h"

namespace inet {

void registerService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    auto otherGate = findConnectedGate<IProtocolRegistrationListener>(gate);
    if (otherGate != nullptr) {
        IProtocolRegistrationListener *protocolRegistration = check_and_cast<IProtocolRegistrationListener *>(otherGate->getOwner());
        protocolRegistration->handleRegisterService(protocol, otherGate, servicePrimitive);
    }
}

void registerService(const Protocol& protocol, cGate *requestIn, cGate *indicationOut)
{
    registerService(protocol, requestIn, indicationOut, requestIn, indicationOut);
}

void registerService(const Protocol& protocol, cGate *requestIn, cGate *indicationOut, cGate *responseIn, cGate *confirmOut)
{
    if (requestIn != nullptr)
        registerService(protocol, requestIn, SP_REQUEST);
    if (indicationOut != nullptr)
        registerService(protocol, indicationOut, SP_INDICATION);
    if (responseIn != nullptr)
        registerService(protocol, responseIn, SP_RESPONSE);
    if (confirmOut != nullptr)
        registerService(protocol, confirmOut, SP_CONFIRM);
}

void registerProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    auto otherGate = findConnectedGate<IProtocolRegistrationListener>(gate);
    if (otherGate != nullptr) {
        IProtocolRegistrationListener *protocolRegistration = check_and_cast<IProtocolRegistrationListener *>(otherGate->getOwner());
        protocolRegistration->handleRegisterProtocol(protocol, otherGate, servicePrimitive);
    }
}

void registerProtocol(const Protocol& protocol, cGate *requestOut, cGate *indicationIn)
{
    registerProtocol(protocol, requestOut, indicationIn, requestOut, indicationIn);
}

void registerProtocol(const Protocol& protocol, cGate *requestOut, cGate *indicationIn, cGate *responseOut, cGate *confirmIn)
{
    if (requestOut != nullptr)
        registerProtocol(protocol, requestOut, SP_REQUEST);
    if (indicationIn != nullptr)
        registerProtocol(protocol, indicationIn, SP_INDICATION);
    if (responseOut != nullptr)
        registerProtocol(protocol, responseOut, SP_RESPONSE);
    if (confirmIn != nullptr)
        registerProtocol(protocol, confirmIn, SP_CONFIRM);
}

} // namespace inet

