//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/IProtocolRegistrationListener.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

OPP_THREAD_LOCAL std::deque<std::string> registrationPath;

std::string printRegistrationPath()
{
    std::string result;
    bool first = true;
    for (auto level : registrationPath) {
        result += (first ? "" : "->") + level;
        first = false;
    }
    return result;
}

void registerService(const Protocol& protocol, cGate *gate, ServicePrimitive servicePrimitive)
{
    registrationPath.push_back(gate->getOwnerModule()->getFullName());
    auto otherGate = findConnectedGate<IProtocolRegistrationListener>(gate);
    if (otherGate != nullptr && otherGate->getOwnerModule() != gate->getOwnerModule()) {
        EV_DEBUG << "Forwarding service registration from " << printRegistrationPath() << EV_FIELD(protocol) << EV_FIELD(gate) << EV_FIELD(otherGate) << EV_FIELD(servicePrimitive) << EV_ENDL;
        IProtocolRegistrationListener *protocolRegistration = check_and_cast<IProtocolRegistrationListener *>(otherGate->getOwner());
        protocolRegistration->handleRegisterService(protocol, otherGate, servicePrimitive);
    }
    registrationPath.pop_back();
}

void registerService(const Protocol& protocol, cGate *requestIn, cGate *indicationOut, cGate *responseIn, cGate *confirmOut)
{
    EV_INFO << "Registering service" << EV_FIELD(protocol) << EV_FIELD(requestIn) << EV_FIELD(indicationOut) << EV_FIELD(responseIn) << EV_FIELD(confirmOut) << EV_ENDL;
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
    registrationPath.push_back(gate->getOwnerModule()->getFullName());
    auto otherGate = findConnectedGate<IProtocolRegistrationListener>(gate);
    if (otherGate != nullptr && otherGate->getOwnerModule() != gate->getOwnerModule()) {
        EV_DEBUG << "Forwarding service group registration from " << printRegistrationPath() << EV_FIELD(protocolGroup) << EV_FIELD(gate) << EV_FIELD(otherGate) << EV_FIELD(servicePrimitive) << EV_ENDL;
        IProtocolRegistrationListener *protocolRegistration = check_and_cast<IProtocolRegistrationListener *>(otherGate->getOwner());
        protocolRegistration->handleRegisterServiceGroup(protocolGroup, otherGate, servicePrimitive);
    }
    registrationPath.pop_back();
}

void registerServiceGroup(const ProtocolGroup& protocolGroup, cGate *requestIn, cGate *indicationOut, cGate *responseIn, cGate *confirmOut)
{
    EV_INFO << "Registering service group" << EV_FIELD(protocolGroup) << EV_FIELD(requestIn) << EV_FIELD(indicationOut) << EV_FIELD(responseIn) << EV_FIELD(confirmOut) << EV_ENDL;
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
    registrationPath.push_back(gate->getOwnerModule()->getFullName());
    auto otherGate = findConnectedGate<IProtocolRegistrationListener>(gate);
    if (otherGate != nullptr && otherGate->getOwnerModule() != gate->getOwnerModule()) {
        EV_DEBUG << "Forwarding any service registration from " << printRegistrationPath() << EV_FIELD(gate) << EV_FIELD(otherGate) << EV_FIELD(servicePrimitive) << EV_ENDL;
        IProtocolRegistrationListener *protocolRegistration = check_and_cast<IProtocolRegistrationListener *>(otherGate->getOwner());
        protocolRegistration->handleRegisterAnyService(otherGate, servicePrimitive);
    }
    registrationPath.pop_back();
}

void registerAnyService(cGate *requestIn, cGate *indicationOut, cGate *responseIn, cGate *confirmOut)
{
    EV_INFO << "Registering any service" << EV_FIELD(requestIn) << EV_FIELD(indicationOut) << EV_FIELD(responseIn) << EV_FIELD(confirmOut) << EV_ENDL;
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
    registrationPath.push_back(gate->getOwnerModule()->getFullName());
    auto otherGate = findConnectedGate<IProtocolRegistrationListener>(gate);
    if (otherGate != nullptr && otherGate->getOwnerModule() != gate->getOwnerModule()) {
        EV_DEBUG << "Forwarding protocol registration from " << printRegistrationPath() << EV_FIELD(protocol) << EV_FIELD(gate) << EV_FIELD(otherGate) << EV_FIELD(servicePrimitive) << EV_ENDL;
        IProtocolRegistrationListener *protocolRegistration = check_and_cast<IProtocolRegistrationListener *>(otherGate->getOwner());
        protocolRegistration->handleRegisterProtocol(protocol, otherGate, servicePrimitive);
    }
    registrationPath.pop_back();
}

void registerProtocol(const Protocol& protocol, cGate *requestOut, cGate *indicationIn, cGate *responseOut, cGate *confirmIn)
{
    EV_INFO << "Registering protocol" << EV_FIELD(protocol) << EV_FIELD(requestOut) << EV_FIELD(indicationIn) << EV_FIELD(responseOut) << EV_FIELD(confirmIn) << EV_ENDL;
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
    registrationPath.push_back(gate->getOwnerModule()->getFullName());
    auto otherGate = findConnectedGate<IProtocolRegistrationListener>(gate);
    if (otherGate != nullptr && otherGate->getOwnerModule() != gate->getOwnerModule()) {
        EV_DEBUG << "Forwarding protocol group registration from " << printRegistrationPath() << EV_FIELD(protocolGroup) << EV_FIELD(gate) << EV_FIELD(otherGate) << EV_FIELD(servicePrimitive) << EV_ENDL;
        IProtocolRegistrationListener *protocolRegistration = check_and_cast<IProtocolRegistrationListener *>(otherGate->getOwner());
        protocolRegistration->handleRegisterProtocolGroup(protocolGroup, otherGate, servicePrimitive);
    }
    registrationPath.pop_back();
}

void registerProtocolGroup(const ProtocolGroup& protocolGroup, cGate *requestOut, cGate *indicationIn, cGate *responseOut, cGate *confirmIn)
{
    EV_INFO << "Registering protocol group" << EV_FIELD(protocolGroup) << EV_FIELD(requestOut) << EV_FIELD(indicationIn) << EV_FIELD(responseOut) << EV_FIELD(confirmIn) << EV_ENDL;
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
    registrationPath.push_back(gate->getOwnerModule()->getFullName());
    auto otherGate = findConnectedGate<IProtocolRegistrationListener>(gate);
    if (otherGate != nullptr && otherGate->getOwnerModule() != gate->getOwnerModule()) {
        EV_DEBUG << "Forwarding any protocol registration from " << printRegistrationPath() << EV_FIELD(gate) << EV_FIELD(otherGate) << EV_FIELD(servicePrimitive) << EV_ENDL;
        IProtocolRegistrationListener *protocolRegistration = check_and_cast<IProtocolRegistrationListener *>(otherGate->getOwner());
        protocolRegistration->handleRegisterAnyProtocol(otherGate, servicePrimitive);
    }
    registrationPath.pop_back();
}

void registerAnyProtocol(cGate *requestOut, cGate *indicationIn, cGate *responseOut, cGate *confirmIn)
{
    EV_INFO << "Registering any protocol" << EV_FIELD(requestOut) << EV_FIELD(indicationIn) << EV_FIELD(responseOut) << EV_FIELD(confirmIn) << EV_ENDL;
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

