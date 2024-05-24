//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/IModuleInterfaceLookup.h"

namespace inet {

cGate *findModuleInterface(cGate *gate, const std::type_info& type, const cObject *arguments, int direction)
{
    EV_INFO << "Searching for module interface" << EV_FIELD(gate) << EV_FIELD(type, opp_typename(type)) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
    bool forward = direction > 0 || (direction == 0 && gate->getType() == cGate::OUTPUT);
    while (true) {
        gate = forward ? gate->getNextGate() : gate->getPreviousGate();
        if (gate == nullptr) {
            EV_INFO << "Module interface not found" << EV_FIELD(gate) << EV_FIELD(type, opp_typename(type)) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
            return nullptr;
        }
        else if (auto lookup = dynamic_cast<IModuleInterfaceLookup *>(gate->getOwner()))
            return lookup->lookupModuleInterface(gate, type, arguments, direction);
        else {
            // NOTE: default behavior is to look for @interface properties on the gate
            EV_INFO << "Checking interface by properties" << EV_FIELD(gate) << EV_FIELD(type, opp_typename(type)) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
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
                    return gate;
                }
            }
        }
    }
}

} // namespace inet

