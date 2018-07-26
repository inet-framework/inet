//
// Copyright (C) OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/ModuleAccess.h"
#include "inet/emulation/common/ExtInterface.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

Define_Module(ExtInterface);

void ExtInterface::initialize(int stage)
{
    if (stage == INITSTAGE_LINK_LAYER)
        registerInterface();
}

void ExtInterface::registerInterface()
{
    setMtu(par("mtu"));
    setBroadcast(true);      //TODO
    setMulticast(true);      //TODO
    setPointToPoint(true);   //TODO

    IInterfaceTable *interfaceTable = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    if (interfaceTable)
        interfaceTable->addInterface(this);
    inet::registerInterface(*this, gate("upperLayerIn"), gate("upperLayerOut"));
}

} // namespace inet

