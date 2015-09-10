//
// Copyright (C) 2013 Opensim Ltd.
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
// Author: Levente Meszaros <levy@omnetpp.org>, Andras Varga (andras@omnetpp.org)
//

#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

Register_Class(NodeStartOperation);
Register_Class(NodeShutdownOperation);
Register_Class(NodeCrashOperation);
//Register_Class(NodeSuspendOperation);

void NodeOperation::initialize(cModule *module, StringMap& params)
{
    if (!isNetworkNode(module))
        throw cRuntimeError("Node operations may only be applied to network nodes (host, router, etc.)");

    LifecycleOperation::initialize(module, params);
}

} // namespace inet

