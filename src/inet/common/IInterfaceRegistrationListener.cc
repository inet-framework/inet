//
// Copyright (C) 2012 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/IInterfaceRegistrationListener.h"

namespace inet {

void registerInterface(const NetworkInterface& interface, cGate *in, cGate *out)
{
    EV_INFO << "Registering network interface" << EV_FIELD(interface) << EV_FIELD(in) << EV_FIELD(out) << EV_ENDL;
    auto outPathEnd = out->getPathEndGate();
    auto inPathStart = in->getPathStartGate();
    IInterfaceRegistrationListener *outInterfaceRegistration = dynamic_cast<IInterfaceRegistrationListener *>(outPathEnd->getOwner());
    IInterfaceRegistrationListener *inInterfaceRegistration = dynamic_cast<IInterfaceRegistrationListener *>(inPathStart->getOwner());
    if (outInterfaceRegistration != nullptr && inInterfaceRegistration != nullptr && outInterfaceRegistration == inInterfaceRegistration)
        outInterfaceRegistration->handleRegisterInterface(interface, inPathStart, outPathEnd);
}

} // namespace inet

