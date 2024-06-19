//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/IModuleInterfaceLookup.h"
#include "inet/queueing/contract/IActivePacketSink.h"

namespace inet {

cGate *findModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction)
{
    EV_TRACE << "Searching for module interface" << EV_FIELD(gate) << EV_FIELD(type, opp_typename(type)) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
    bool forward = direction > 0 || (direction == 0 && gate->getType() == cGate::OUTPUT);
    while (true) {
        gate = forward ? gate->getNextGate() : gate->getPreviousGate();
        if (gate == nullptr) {
            EV_TRACE << "Module interface not found" << EV_FIELD(type, opp_typename(type)) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
            return nullptr;
        }
        else if (auto lookup = dynamic_cast<IModuleInterfaceLookup *>(gate->getOwner()))
            return lookup->lookupModuleInterface(gate, type, arguments, direction);
        else {
            // NOTE: default behavior is to look for @interface properties on the gate
            EV_TRACE << "Checking interface by properties" << EV_FIELD(gate) << EV_FIELD(type, opp_typename(type)) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
            auto properties = gate->getProperties();
            for (auto index : properties->getIndicesFor("interface")) {
                auto property = properties->get("interface", index);
                std::string fullyQualifiedType = index;
                if (!strcmp(opp_typename(type), fullyQualifiedType.c_str())) {
                    auto protocol = property->getValue("protocol");
                    auto service = property->getValue("service");
                    if (protocol != nullptr) {
                        auto dispatchProtocolReq = dynamic_cast<const DispatchProtocolReq *>(arguments);
                        if (dispatchProtocolReq == nullptr || strcmp(protocol, dispatchProtocolReq->getProtocol()->getName()))
                            continue;
                    }
                    if (service != nullptr) {
                        auto dispatchProtocolReq = dynamic_cast<const DispatchProtocolReq *>(arguments);
                        if (!strcmp(service, "request")) {
                            if (dispatchProtocolReq == nullptr || dispatchProtocolReq->getServicePrimitive() != SP_REQUEST)
                                continue;
                        }
                        else if (!strcmp(service, "indication")) {
                            if (dispatchProtocolReq == nullptr || dispatchProtocolReq->getServicePrimitive() != SP_INDICATION)
                                continue;
                        }
                        else
                            throw cRuntimeError("Unknown service");
                    }
                    // KLUDGE: TODO: this is needed for the tunnel example, it's used by the PacketQueueBase
                    if (property->getValue("forward") && !findModuleInterface(gate->getOwnerModule()->gate("out"), typeid(queueing::IActivePacketSink), arguments, direction))
                        continue;
                    return gate;
                }
            }
        }
    }
}

} // namespace inet

