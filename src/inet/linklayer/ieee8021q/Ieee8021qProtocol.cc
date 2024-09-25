//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee8021q/Ieee8021qProtocol.h"

#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {

using namespace inet::queueing;

Define_Module(Ieee8021qProtocol);

cGate *Ieee8021qProtocol::lookupModuleInterface(cGate *gate, const std::type_info &type, const cObject *arguments, int direction)
{
    Enter_Method("lookupModuleInterface");
    EV_TRACE << "Looking up module interface" << EV_FIELD(gate) << EV_FIELD(type, opp_typename(type)) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
    const char *vlanTagType = par("vlanTagType");
    const Protocol *qtagProtocol;
    if (!strcmp("s", vlanTagType))
        qtagProtocol = &Protocol::ieee8021qSTag;
    else if (!strcmp("c", vlanTagType))
        qtagProtocol = &Protocol::ieee8021qCTag;
    else
        throw cRuntimeError("Unknown tag type");
    if (gate->isName("upperLayerIn")) {
        if (type == typeid(IPassivePacketSink)) {
            auto dispatchProtocolReq = dynamic_cast<const DispatchProtocolReq *>(arguments);
            if (dispatchProtocolReq != nullptr && dispatchProtocolReq->getProtocol() == qtagProtocol && dispatchProtocolReq->getServicePrimitive() == SP_REQUEST)
                return findModuleInterface(gate, type, nullptr, 1);
        }
        else
            return findModuleInterface(gate, type, arguments, 1);
    }
    else if (gate->isName("upperLayerOut"))
        return findModuleInterface(gate, type, arguments); // forward all other interfaces
    else if (gate->isName("lowerLayerIn")) {
        if (type == typeid(IPassivePacketSink)) {
            auto dispatchProtocolReq = dynamic_cast<const DispatchProtocolReq *>(arguments);
            if (dispatchProtocolReq != nullptr && dispatchProtocolReq->getProtocol() == qtagProtocol && dispatchProtocolReq->getServicePrimitive() == SP_INDICATION)
                return findModuleInterface(gate, type, nullptr, 1);
        }
    }
    else if (gate->isName("lowerLayerOut"))
        return findModuleInterface(gate, type, arguments); // forward all other interfaces
    return nullptr;
}

} // namespace inet

