//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/IModuleInterfaceLookup.h"
#include "inet/queueing/contract/IActivePacketSink.h"

namespace inet {

cGate *findModuleInterface(cGate *originatorGate, const std::type_info& typeInfo, const cObject *arguments, int direction)
{
    auto type = opp_typename(typeInfo);
    auto originator = originatorGate->getOwnerModule();
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
                    auto expectedArguments = property->getValue("arguments");
                    if (expectedArguments != nullptr && !strcmp(expectedArguments, "null") && arguments != nullptr) {
                        EV_TRACE << "Module interface not found using @interface gate property, no arguments were expected" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                        continue;
                    }
                    auto protocol = property->getValue("protocol");
                    if (protocol != nullptr) {
                        auto dispatchProtocolReq = dynamic_cast<const DispatchProtocolReq *>(arguments);
                        if (dispatchProtocolReq == nullptr || strcmp(protocol, dispatchProtocolReq->getProtocol()->getName())) {
                            EV_TRACE << "Module interface not found using @interface gate property, protocol doesn't match" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                            continue;
                        }
                    }
                    auto service = property->getValue("service");
                    if (service != nullptr) {
                        auto dispatchProtocolReq = dynamic_cast<const DispatchProtocolReq *>(arguments);
                        if (!strcmp(service, "request")) {
                            if (dispatchProtocolReq == nullptr || dispatchProtocolReq->getServicePrimitive() != SP_REQUEST) {
                                EV_TRACE << "Module interface not found using @interface gate property, service doesn't match" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                                continue;
                            }
                        }
                        else if (!strcmp(service, "indication")) {
                            if (dispatchProtocolReq == nullptr || dispatchProtocolReq->getServicePrimitive() != SP_INDICATION) {
                                EV_TRACE << "Module interface not found using @interface gate property, service doesn't match" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                                continue;
                            }
                        }
                        else
                            throw cRuntimeError("Unknown service parameter value in @interface gate property, module = %s, gate = %s, property = %s", module->getFullPath().c_str(), gate->getFullName(), property->str().c_str());
                    }
                    // KLUDGE: TODO: this is needed for the tunnel example, it's used by the PacketQueueBase
                    if (property->getValue("forward") && !findModuleInterface(gate->getOwnerModule()->gate("out"), typeid(queueing::IActivePacketSink), arguments, direction)) {
                        EV_TRACE << "Module interface not found using @interface gate property, cannot forward" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                        continue;
                    }
                    auto module = gate->getOwnerModule();
                    EV_TRACE << "Module interface found using @interface gate property" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_FIELD(property) << EV_ENDL;
                    return gate;
                }
            }
            EV_TRACE << "Module interface not found using @interface gate properties" << EV_FIELD(originator) << EV_FIELD(originatorGate) << EV_FIELD(module) << EV_FIELD(gate) << EV_FIELD(type) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
        }
    }
}

} // namespace inet

