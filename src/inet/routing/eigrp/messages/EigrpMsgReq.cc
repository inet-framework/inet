//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/*
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#include "inet/routing/eigrp/messages/EigrpMsgReq.h"

namespace inet {

int EigrpMsgReq::findMsgRoute(int routeId) const
{
    EigrpMsgRoute rt;

    for (unsigned i = 0; i < getRoutesArraySize(); i++) {
        rt = getRoutes(i);
        if (rt.routeId == routeId) {
            return i;
        }
    }
    return -1;
}

} // namespace inet

