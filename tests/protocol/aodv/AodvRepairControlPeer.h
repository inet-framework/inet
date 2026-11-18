// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __AODV_REPAIR_CONTROL_PEER_H
#define __AODV_REPAIR_CONTROL_PEER_H
#include "AodvReplyFailurePeer.h"
class AodvRepairControlPeer : public AodvReplyFailurePeer
{
  protected:
    void finish() override {
        ASSERT(forwardedRequests == 0);
        for (int i = 0; i < routes->getNumRoutes(); i++) {
            auto route = routes->getRoute(i);
            if (route->getSourceType() == IRoute::AODV) {
                auto data = check_and_cast<AodvRouteData *>(route->getProtocolData());
                ASSERT(!data->isBeingRepaired() && !data->isRepariable());
            }
        }
    }
};
Define_Module(AodvRepairControlPeer);
#endif
