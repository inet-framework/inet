//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#ifndef __INET_EIGRPDUAL_H
#define __INET_EIGRPDUAL_H

#include <omnetpp.h>

//#include "IPv4Address.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/routing/eigrp/EigrpDualStack.h"
#include "inet/routing/eigrp/pdms/IEigrpPdm.h"
#include "inet/routing/eigrp/tables/EigrpRoute.h"
namespace inet {
namespace eigrp {
/**
 * Class represents DUAL automaton.
 */
template<typename IPAddress>
class INET_API EigrpDual : public cObject /* cSimpleModule */
{
  public:
    enum DualEvent {
        RECV_UPDATE = 0,    /**< Change of route distance in received update message or on interface */
        RECV_QUERY,         /**< Received query message */
        RECV_REPLY,         /**< Received reply message */
        NEIGHBOR_DOWN,      /**< Neighbor went down */
        INTERFACE_DOWN,     /**< EIGRP disabled on interface - only for connected route */
        INTERFACE_UP,       /**< EIGRP enabled on interface */
        LOST_ROUTE,         /**< Route in RT deleted, but not by EIGRP */
    };

  protected:
    IEigrpPdm<IPAddress> *pdm; /**< Protocol dependent module interface */

    /**
     * Invalidates specified route.
     */
    void invalidateRoute(EigrpRouteSource<IPAddress> *routeSrc);

    //-- DUAL states
    void processQo0(DualEvent event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId, bool isSourceNew);
    void processQo1Passive(DualEvent event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId, bool isSourceNew);
    void processQo1Active(DualEvent event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId, bool isSourceNew);
    void processQo2(DualEvent event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId, bool isSourceNew);
    void processQo3(DualEvent event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId, bool isSourceNew);

    //-- DUAL transitions
    void processTransition1(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, uint64_t dmin, int neighborId);
    void processTransition2(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, uint64_t dmin, int neighborId);
    void processTransition3(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, uint64_t dmin, int neighborId);
    void processTransition4(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, uint64_t dmin, int neighborId);
    void processTransition5(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId);
    void processTransition6(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId);
    void processTransition7(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId);
    void processTransition8(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId, bool isSourceNew);
    void processTransition9(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId);
    void processTransition10(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId);
    void processTransition11(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, uint64_t dmin, int neighborId);
    void processTransition12(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, uint64_t dmin, int neighborId);
    void processTransition13(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId);
    void processTransition14(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, uint64_t dmin, int neighborId);
    void processTransition15(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId);
    void processTransition16(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, uint64_t dmin, int neighborId);
    void processTransition17(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId);
    void processTransition18(int event, EigrpRouteSource<IPAddress> *source, EigrpRoute<IPAddress> *route, int neighborId, bool isSourceNew);

  public:
    EigrpDual(IEigrpPdm<IPAddress> *pdm) { this->pdm = pdm; }

    /**
     * The entry point for processing events.
     * @param event type of event
     * @param source route
     * @param neighborId source of the event
     * @param isSourceNew if route was created now
     */
    void processEvent(DualEvent event, EigrpRouteSource<IPAddress> *source, int neighborId, bool isSourceNew);
};
} // eigrp
} // inet
#endif

