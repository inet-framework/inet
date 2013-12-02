//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
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
// Authors: Tomas Prochazka (mailto:xproch21@stud.fit.vutbr.cz), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)

#ifndef PIMROUTINGTABLEACCESS_H_
#define PIMROUTINGTABLEACCESS_H_

#include <omnetpp.h>
#include "ModuleAccess.h"
#include "PIMRoutingTable.h"

/**
 * @brief Class gives access to the MulticastRoutingTable.
 */
class INET_API PIMRoutingTableAccess : public ModuleAccess<PIMRoutingTable>
{
    public:
        PIMRoutingTableAccess() : ModuleAccess<PIMRoutingTable>("routingTable") {}
};

#endif /* ANSAROUTINGTABLEACCESS_H_ */
