//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/IModuleInterfaceLookup.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/queueing/contract/IActivePacketSink.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {

//
// Examples for finding an IPassivePacketSink module interface:
//  - TcpSocket -> Tcp module: TcpSocket class searches using a DispatchProtocolReq argument for TCP SDUs, Tcp module accepts because it matches exactly
//  - Tcp module -> application module: Tcp module searches using a SocketInd argument for TCP SDUs, application module accepts this lookup because it has the open socket
//  - Tcp -> Ipv4: Tcp module searches using DispatchProtocolReq argument for TCP PDUs, Ipv4 module accepts because the TCP protocol is in the IP protocol group
//  - Ipv4 -> Tcp: Ipv4 module searches using DispatchProtocolReq argument for IPv4 SDUs, Tcp module accepts because TODO???
//  - Any module -> MessageDispatcher: MessageDispatcher module accepts any lookup if it can unambiguously find another module on one of its outputs
//  - Any module -> PacketDelayer: PacketDelayer module accepts any lookup if the output accepts the same lookup
//
// TCP 4 gates:
//  - upperLayerIn receives outgoing TCP SDUs using DispatchProtocolReq(tcp, SP_REQUEST)
//  - upperLayerOut sends incoming TCP SDUs using PacketServiceTag(tcp)
//  - lowerLayerIn receives incoming TCP PDUs using DispatchProtocolReq(tcp, SP_INDICATION)
//  - lowerLayerOut sends outgoing TCP PDUs using PacketProtocolTag(tcp)
//
cGate *findModuleInterface(cGate *originatorGate, const std::type_info& typeInfo, const cObject *arguments, int direction)
{
    auto type = opp_typename(typeInfo);
    auto originator = originatorGate->getOwnerModule();
    cMethodCallContextSwitcher switcher(originator);
    switcher.methodCall("findModuleInterface");
    EV_TRACE << "Finding module interface" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
    bool forward = direction > 0 || (direction == 0 && originatorGate->getType() == cGate::OUTPUT);
    auto gate = originatorGate;
    while (true) {
        gate = forward ? gate->getNextGate() : gate->getPreviousGate();
        if (gate == nullptr) {
            EV_TRACE << "Module interface not found, there are no more gates to check" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
            return nullptr;
        }
        auto module = gate->getOwnerModule();
        if (auto lookup = dynamic_cast<IModuleInterfaceLookup *>(module)) {
            EV_TRACE << "Finding module interface using IModuleInterfaceLookup" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
            gate = lookup->lookupModuleInterface(gate, typeInfo, arguments, direction);
            if (gate != nullptr) {
                auto module = gate->getOwnerModule();
                EV_TRACE << "Module interface found using IModuleInterfaceLookup" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
            }
            else
                EV_TRACE << "Module interface not found using IModuleInterfaceLookup" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
            return gate;
        }
        else {
            EV_TRACE << "Finding module interface using @interface gate properties" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
            auto properties = gate->getProperties();
            for (auto index : properties->getIndicesFor("interface")) {
                auto property = properties->get("interface", index);
                EV_TRACE << "Finding module interface using @interface gate property" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                std::string fullyQualifiedType = index;
                if (!strcmp(opp_typename(typeInfo), fullyQualifiedType.c_str())) {
                    if (auto forwardGateName = property->getValue("forward")) {
                        auto typeName = property->getValue("forward", 1);
                        const std::type_info& forwardTypeInfo =
                                typeName == nullptr ? typeInfo :
                                !strcmp(typeName, "inet::queueing::IPassivePacketSink") ? typeid(inet::queueing::IPassivePacketSink) :
                                !strcmp(typeName, "inet::queueing::IActivePacketSink") ? typeid(inet::queueing::IActivePacketSink) :
                                throw cRuntimeError("Unknown type name: %s", typeName);
                        if (module->isGateVector(forwardGateName)) {
                            for (int i = 0; i < module->gateSize(forwardGateName); i++) {
                                if (!findModuleInterface(module->gate(forwardGateName, i), forwardTypeInfo, arguments, direction)) {
                                    EV_TRACE << "Module interface not found using @interface gate property, cannot forward" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                                    goto NOT_FOUND;
                                }
                            }
                        }
                        else {
                            if (!findModuleInterface(module->gate(forwardGateName), forwardTypeInfo, arguments, direction)) {
                                EV_TRACE << "Module interface not found using @interface gate property, cannot forward" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                                continue;
                            }
                        }
                    }
                    else if (arguments == nullptr) {
                        auto expectedArguments = property->getValue("arguments");
                        if (expectedArguments != nullptr && strcmp(expectedArguments, "null")) {
                            EV_TRACE << "Module interface not found using @interface gate property, no arguments is not allowed" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                            continue;
                        }
                    }
                    else if (auto dispatchProtocolReq = dynamic_cast<const DispatchProtocolReq *>(arguments)) {
                        if (property->getNumKeys() != 0) {
                            auto protocol = property->getValue("protocol");
                            if (protocol == nullptr || strcmp(protocol, dispatchProtocolReq->getProtocol()->getName())) {
                                EV_TRACE << "Module interface not found using @interface gate property, protocol doesn't match" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                                continue;
                            }
                            auto service = property->getValue("service");
                            if (service == nullptr) {
                                EV_TRACE << "Module interface not found using @interface gate property, service doesn't match" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                                continue;
                            }
                            else if (!strcmp(service, "request")) {
                                if (dispatchProtocolReq->getServicePrimitive() != SP_REQUEST) {
                                    EV_TRACE << "Module interface not found using @interface gate property, service doesn't match" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                                    continue;
                                }
                            }
                            else if (!strcmp(service, "indication")) {
                                if (dispatchProtocolReq->getServicePrimitive() != SP_INDICATION) {
                                    EV_TRACE << "Module interface not found using @interface gate property, service doesn't match" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                                    continue;
                                }
                            }
                            else
                                throw cRuntimeError("Unknown service parameter value in @interface gate property, module = %s, gate = %s, property = %s", module->getFullPath().c_str(), gate->getFullName(), property->str().c_str());
                        }
                    }
                    else if (auto packetProtocolTag = dynamic_cast<const PacketProtocolTag *>(arguments)) {
                        if (property->getNumKeys() != 0) {
                            int numValues = property->getNumValues("pdu");
                            auto protocolGroupName = property->getValue("protocolGroup");
                            auto protocolGroup = protocolGroupName != nullptr ? ProtocolGroup::findProtocolGroup(protocolGroupName) : nullptr;
                            bool found = protocolGroup != nullptr && protocolGroup->findProtocolNumber(packetProtocolTag->getProtocol()) != -1;
                            for (int i = 0; i < numValues; i++) {
                                auto pdu = property->getValue("pdu", i);
                                if (!strcmp(pdu, packetProtocolTag->getProtocol()->getName()))
                                    found = true;
                            }
                            if (!found) {
                                EV_TRACE << "Module interface not found using @interface gate property, protocol doesn't match" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                                continue;
                            }
                        }
                    }
                    else if (auto packetServiceTag = dynamic_cast<const PacketServiceTag *>(arguments)) {
                        if (property->getNumKeys() != 0) {
                            int numValues = property->getNumValues("sdu");
                            auto protocolGroupName = property->getValue("protocolGroup");
                            auto protocolGroup = protocolGroupName != nullptr ? ProtocolGroup::findProtocolGroup(protocolGroupName) : nullptr;
                            bool found = protocolGroup != nullptr && protocolGroup->findProtocolNumber(packetServiceTag->getProtocol()) != -1;
                            for (int i = 0; i < numValues; i++) {
                                auto sdu = property->getValue("sdu", i);
                                if (!strcmp(sdu, packetServiceTag->getProtocol()->getName()))
                                    found = true;
                            }
                            if (!found) {
                                EV_TRACE << "Module interface not found using @interface gate property, protocol doesn't match" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                                continue;
                            }
                        }
                    }
                    else if (dynamic_cast<const SocketInd *>(arguments) != nullptr) {
                        EV_TRACE << "Module interface not found using @interface gate property, socket cannot be matched" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                        continue;
                    }
                    else {
                        EV_TRACE << "Module interface not found using @interface gate property, arguments doesn't match" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                        continue;
                    }
                    auto module = gate->getOwnerModule();
                    EV_TRACE << "Module interface found using @interface gate property" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                    return gate;
                }
                NOT_FOUND:;
            }
            EV_TRACE << "Module interface not found using @interface gate properties" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
        }
    }
}

} // namespace inet

