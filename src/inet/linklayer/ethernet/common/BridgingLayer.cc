//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/common/BridgingLayer.h"

#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/queueing/contract/IPassivePacketSink.h"

namespace inet {

using namespace inet::queueing;

Define_Module(BridgingLayer);

cGate *BridgingLayer::lookupModuleInterface(cGate *gate, const std::type_info &type, const cObject *arguments, int direction)
{
    Enter_Method("lookupModuleInterface");
    EV_TRACE << "Looking up module interface" << EV_FIELD(gate) << EV_FIELD(type, opp_typename(type)) << EV_FIELD(arguments) << EV_FIELD(direction) << EV_ENDL;
    if (gate->isName("upperLayerIn")) {
        if (type == typeid(IPassivePacketSink)) {
            auto dispatchProtocolReq = dynamic_cast<const DispatchProtocolReq *>(arguments);
            if (dispatchProtocolReq != nullptr && dispatchProtocolReq->getProtocol() == &Protocol::ethernetMac && dispatchProtocolReq->getServicePrimitive() == SP_REQUEST)
                return findModuleInterface(gate, type, nullptr, 1);
            auto interfaceReq = dynamic_cast<const InterfaceReq *>(arguments);
            if (interfaceReq != nullptr)
                return findModuleInterface(gate, type, arguments, 1);
        }
//        else if (type == typeid(IEthernet))
//            return findModuleInterface(this->gate("lowerLayerOut"), type, arguments);
    }
    else if (gate->isName("upperLayerOut"))
        return findModuleInterface(gate, type, arguments); // forward all other interfaces
    else if (gate->isName("lowerLayerIn")) {
        if (type == typeid(IPassivePacketSink)) {
            auto dispatchProtocolReq = dynamic_cast<const DispatchProtocolReq *>(arguments);
            if (dispatchProtocolReq != nullptr && dispatchProtocolReq->getServicePrimitive() == SP_INDICATION && findModuleInterface(this->gate("lowerLayerOut"), type, arguments) == nullptr)
                return findModuleInterface(gate, type, nullptr, 1);
        }
    }
    else if (gate->isName("lowerLayerOut"))
        return findModuleInterface(gate, type, arguments); // forward all other interfaces
    return nullptr;
}

} // namespace inet

