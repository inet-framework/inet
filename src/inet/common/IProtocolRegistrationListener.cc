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
    EV_INFO << "Registering service" << EV_FIELD(protocol) << EV_FIELD(gate) << EV_FIELD(servicePrimitive) << std::endl;
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
    EV_INFO << "Registering service group" << EV_FIELD(protocolGroup) << EV_FIELD(gate) << EV_FIELD(servicePrimitive) << std::endl;
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
    EV_INFO << "Registering any service" << EV_FIELD(gate) << EV_FIELD(servicePrimitive) << std::endl;
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
    EV_INFO << "Registering protocol" << EV_FIELD(protocol) << EV_FIELD(gate) << EV_FIELD(servicePrimitive) << std::endl;
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
    EV_INFO << "Registering protocol group" << EV_FIELD(protocolGroup) << EV_FIELD(gate) << EV_FIELD(servicePrimitive) << std::endl;
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
    EV_INFO << "Registering any protocol" << EV_FIELD(gate) << EV_FIELD(servicePrimitive) << std::endl;
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

void TransparentProtocolRegistrationListener::handleRegisterService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method2("handleRegisterService");
    if (isForwardingService(protocol, gate, servicePrimitive)) {
        EV_INFO << "Forwarding service" << EV_FIELD(protocol) << EV_FIELD(servicePrimitive) << EV_FIELD(gate) << EV_ENDL;
        for (auto fwGate : getRegistrationForwardingGates(gate))
            registerService(protocol, fwGate, servicePrimitive);
    }
}

void TransparentProtocolRegistrationListener::handleRegisterProtocol(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method2("handleRegisterProtocol");
    if (isForwardingProtocol(protocol, gate, servicePrimitive)) {
        EV_INFO << "Forwarding protocol" << EV_FIELD(protocol) << EV_FIELD(servicePrimitive) << EV_FIELD(gate) << EV_ENDL;
        for (auto fwGate : getRegistrationForwardingGates(gate))
            registerProtocol(protocol, fwGate, servicePrimitive);
    }
}

void TransparentProtocolRegistrationListener::handleRegisterServiceGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method2("handleRegisterServiceGroup");
    if (isForwardingServiceGroup(protocolGroup, gate, servicePrimitive)) {
        EV_INFO << "Forwarding service group" << EV_FIELD(protocolGroup) << EV_FIELD(servicePrimitive) << EV_FIELD(gate) << EV_ENDL;
        for (auto fwGate : getRegistrationForwardingGates(gate))
            registerServiceGroup(protocolGroup, fwGate, servicePrimitive);
    }
}

void TransparentProtocolRegistrationListener::handleRegisterProtocolGroup(const ProtocolGroup& protocolGroup, cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method2("handleRegisterProtocolGroup");
    if (isForwardingProtocolGroup(protocolGroup, gate, servicePrimitive)) {
        EV_INFO << "Forwarding protocol group" << EV_FIELD(protocolGroup) << EV_FIELD(servicePrimitive) << EV_FIELD(gate) << EV_ENDL;
        for (auto fwGate : getRegistrationForwardingGates(gate))
            registerProtocolGroup(protocolGroup, fwGate, servicePrimitive);
    }
}

void TransparentProtocolRegistrationListener::handleRegisterAnyService(cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method2("handleRegisterAnyService");
    if (isForwardingAnyService(gate, servicePrimitive)) {
        EV_INFO << "Forwarding any service" << EV_FIELD(servicePrimitive) << EV_FIELD(gate) << EV_ENDL;
        for (auto fwGate : getRegistrationForwardingGates(gate))
            registerAnyService(fwGate, servicePrimitive);
    }
}

void TransparentProtocolRegistrationListener::handleRegisterAnyProtocol(cGate *gate, ServicePrimitive servicePrimitive)
{
    Enter_Method2("handleRegisterAnyProtocol");
    if (isForwardingAnyProtocol(gate, servicePrimitive)) {
        EV_INFO << "Forwarding any protocol" << EV_FIELD(servicePrimitive) << EV_FIELD(gate) << EV_ENDL;
        for (auto fwGate : getRegistrationForwardingGates(gate))
            registerAnyProtocol(fwGate, servicePrimitive);
    }
}

} // namespace inet

