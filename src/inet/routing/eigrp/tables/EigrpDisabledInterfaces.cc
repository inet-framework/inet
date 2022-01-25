//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */
#include "inet/routing/eigrp/tables/EigrpDisabledInterfaces.h"

#include <algorithm>

#include "inet/common/stlutils.h"
#include "inet/routing/eigrp/tables/EigrpInterfaceTable.h"

namespace inet {
namespace eigrp {

EigrpDisabledInterfaces::EigrpDisabledInterfaces()
{
}

EigrpDisabledInterfaces::~EigrpDisabledInterfaces()
{
    int cnt = ifVector.size();
    EigrpInterface *iface;

    for (int i = 0; i < cnt; i++) {
        iface = ifVector[i];
        ifVector[i] = nullptr;
        delete iface;
    }
    ifVector.clear();
}

void EigrpDisabledInterfaces::addInterface(EigrpInterface *interface)
{
    // TODO check duplicity
    this->ifVector.push_back(interface);
}

EigrpInterface *EigrpDisabledInterfaces::removeInterface(EigrpInterface *iface)
{
    auto it = find(ifVector, iface);

    if (it != ifVector.end()) {
        ifVector.erase(it);
        return iface;
    }

    return nullptr;
}

EigrpInterface *EigrpDisabledInterfaces::findInterface(int ifaceId)
{
    EigrpInterface *iface;

    for (auto it = ifVector.begin(); it != ifVector.end(); it++) {
        iface = *it;
        if (iface->getInterfaceId() == ifaceId) {
            return iface;
        }
    }

    return nullptr;
}

} // namespace eigrp
} // namespace inet

