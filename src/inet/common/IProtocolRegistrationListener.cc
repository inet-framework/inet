//
// Copyright (C) 2012 OpenSim Ltd.
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
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/common/IProtocolRegistrationListener.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

void registerService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    auto otherGate = findConnectedGate<IProtocolRegistrationListener>(gate);
    if (otherGate != nullptr && otherGate->getOwnerModule() != gate->getOwnerModule()) {
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

void registerServiceGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive)
{
    auto otherGate = findConnectedGate<IProtocolRegistrationListener>(gate);
    if (otherGate != nullptr && otherGate->getOwnerModule() != gate->getOwnerModule()) {
        IProtocolRegistrationListener *protocolRegistration = check_and_cast<IProtocolRegistrationListener *>(otherGate->getOwner());
        protocolRegistration->handleRegisterServiceGroup(protocolGroup, otherGate, servicePrimitive);
    }
}

void registerServiceGroup(const ProtocolGroup& protocolGroup, cGate *requestIn, cGate *indicationOut)
{
    registerServiceGroup(protocolGroup, requestIn, indicationOut, requestIn, indicationOut);
}

void registerServiceGroup(const ProtocolGroup& protocolGroup, cGate *requestIn, cGate *indicationOut, cGate *responseIn, cGate *confirmOut)
{
    if (requestIn != nullptr)
        registerServiceGroup(protocolGroup, requestIn, SP_REQUEST);
    if (indicationOut != nullptr)
        registerServiceGroup(protocolGroup, indicationOut, SP_INDICATION);
    if (responseIn != nullptr)
        registerServiceGroup(protocolGroup, responseIn, SP_RESPONSE);
    if (confirmOut != nullptr)
        registerServiceGroup(protocolGroup, confirmOut, SP_CONFIRM);
}

void registerAnyService(cGate *gate, ServicePrimitive servicePrimitive)
{
    auto otherGate = findConnectedGate<IProtocolRegistrationListener>(gate);
    if (otherGate != nullptr && otherGate->getOwnerModule() != gate->getOwnerModule()) {
        IProtocolRegistrationListener *protocolRegistration = check_and_cast<IProtocolRegistrationListener *>(otherGate->getOwner());
        protocolRegistration->handleRegisterAnyService(otherGate, servicePrimitive);
    }
}

void registerAnyService(cGate *requestIn, cGate *indicationOut)
{
    registerAnyService(requestIn, indicationOut, requestIn, indicationOut);
}

void registerAnyService(cGate *requestIn, cGate *indicationOut, cGate *responseIn, cGate *confirmOut)
{
    if (requestIn != nullptr)
        registerAnyService(requestIn, SP_REQUEST);
    if (indicationOut != nullptr)
        registerAnyService(indicationOut, SP_INDICATION);
    if (responseIn != nullptr)
        registerAnyService(responseIn, SP_RESPONSE);
    if (confirmOut != nullptr)
        registerAnyService(confirmOut, SP_CONFIRM);
}

void registerProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    auto otherGate = findConnectedGate<IProtocolRegistrationListener>(gate);
    if (otherGate != nullptr && otherGate->getOwnerModule() != gate->getOwnerModule()) {
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

void registerProtocolGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive)
{
    auto otherGate = findConnectedGate<IProtocolRegistrationListener>(gate);
    if (otherGate != nullptr && otherGate->getOwnerModule() != gate->getOwnerModule()) {
        IProtocolRegistrationListener *protocolRegistration = check_and_cast<IProtocolRegistrationListener *>(otherGate->getOwner());
        protocolRegistration->handleRegisterProtocolGroup(protocolGroup, otherGate, servicePrimitive);
    }
}

void registerProtocolGroup(const ProtocolGroup& protocolGroup, cGate *requestOut, cGate *indicationIn)
{
    registerProtocolGroup(protocolGroup, requestOut, indicationIn, requestOut, indicationIn);
}

void registerProtocolGroup(const ProtocolGroup& protocolGroup, cGate *requestOut, cGate *indicationIn, cGate *responseOut, cGate *confirmIn)
{
    if (requestOut != nullptr)
        registerProtocolGroup(protocolGroup, requestOut, SP_REQUEST);
    if (indicationIn != nullptr)
        registerProtocolGroup(protocolGroup, indicationIn, SP_INDICATION);
    if (responseOut != nullptr)
        registerProtocolGroup(protocolGroup, responseOut, SP_RESPONSE);
    if (confirmIn != nullptr)
        registerProtocolGroup(protocolGroup, confirmIn, SP_CONFIRM);
}

void registerAnyProtocol(cGate *gate, ServicePrimitive servicePrimitive)
{
    auto otherGate = findConnectedGate<IProtocolRegistrationListener>(gate);
    if (otherGate != nullptr && otherGate->getOwnerModule() != gate->getOwnerModule()) {
        IProtocolRegistrationListener *protocolRegistration = check_and_cast<IProtocolRegistrationListener *>(otherGate->getOwner());
        protocolRegistration->handleRegisterAnyProtocol(otherGate, servicePrimitive);
    }
}

void registerAnyProtocol(cGate *requestOut, cGate *indicationIn)
{
    registerAnyProtocol(requestOut, indicationIn, requestOut, indicationIn);
}

void registerAnyProtocol(cGate *requestOut, cGate *indicationIn, cGate *responseOut, cGate *confirmIn)
{
    if (requestOut != nullptr)
        registerAnyProtocol(requestOut, SP_REQUEST);
    if (indicationIn != nullptr)
        registerAnyProtocol(indicationIn, SP_INDICATION);
    if (responseOut != nullptr)
        registerAnyProtocol(responseOut, SP_RESPONSE);
    if (confirmIn != nullptr)
        registerAnyProtocol(confirmIn, SP_CONFIRM);
}

} // namespace inet

