//
// Copyright (C) Opensim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/InterfaceOperations.h"
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

    // note: these operations cannot be generic, because their ctor requires InterfaceEntry as parameter
    IInterfaceTable *ift = L3AddressResolver().findInterfaceTableOf(module);
    if (!ift)
        throw cRuntimeError("Interface table of network node '%s' not found, required for operation %s", module->getFullPath().c_str(), getClassName());
    std::string interfaceName = params["interfacename"];
    params.erase("interfacename");  //TODO implement "towards=..."
    if (interfaceName.empty())
        throw cRuntimeError("interfacename attribute missing, required for operation %s", getClassName());
    InterfaceEntry *ie = ift->findInterfaceByName(interfaceName.c_str());
    if (!ie)
        throw cRuntimeError("No interface named '%s', required for operation %s", interfaceName.c_str(), getClassName());
}

} // namespace inet
