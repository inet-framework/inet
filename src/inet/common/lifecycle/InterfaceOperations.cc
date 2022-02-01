//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/lifecycle/InterfaceOperations.h"

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Register_Class(InterfaceDownOperation);
Register_Class(InterfaceUpOperation);

void InterfaceOperationBase::initialize(cModule *module, StringMap& params)
{
    if (!isNetworkNode(module))
        throw cRuntimeError("Interface operations may only be applied to network nodes (host, router, etc.)");

    LifecycleOperation::initialize(module, params);

    // note: these operations cannot be generic, because their ctor requires NetworkInterface as parameter
    IInterfaceTable *ift = L3AddressResolver().findInterfaceTableOf(module);
    if (!ift)
        throw cRuntimeError("Interface table of network node '%s' not found, required for operation %s", module->getFullPath().c_str(), getClassName());
    std::string interfaceName = params["interfacename"];
    params.erase("interfacename"); // TODO implement "towards=..."
    if (interfaceName.empty())
        throw cRuntimeError("interfacename attribute missing, required for operation %s", getClassName());
    NetworkInterface *ie = ift->findInterfaceByName(interfaceName.c_str());
    if (!ie)
        throw cRuntimeError("No interface named '%s', required for operation %s", interfaceName.c_str(), getClassName());
}

} // namespace inet

