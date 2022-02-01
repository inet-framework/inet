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
    IInterfaceRegistrationListener *interfaceRegistration = dynamic_cast<IInterfaceRegistrationListener *>(outPathEnd->getOwner());
    if (interfaceRegistration != nullptr)
        interfaceRegistration->handleRegisterInterface(interface, inPathStart, outPathEnd);
}

} // namespace inet

