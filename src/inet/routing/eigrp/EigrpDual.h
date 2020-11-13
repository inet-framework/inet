//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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

#ifndef EIGRPDUAL_H_
#define EIGRPDUAL_H_

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
template <typename IPAddress>
class EigrpDual : public cObject /* cSimpleModule */
{
  public:
    enum DualEvent
    {
        RECV_UPDATE = 0,    /**< Change of route distance in received update message or on interface */
        RECV_QUERY,         /**< Received query message */
        RECV_REPLY,         /**< Received reply message */
        NEIGHBOR_DOWN,      /**< Neighbor went down */
        INTERFACE_DOWN,     /**< EIGRP disabled on interface - only for connected route */
        INTERFACE_UP,       /**< EIGRP enabled on interface */
        LOST_ROUTE,         /**< Route in RT deleted, but not by EIGRP */
    };

  protected:
    IEigrpPdm<IPAddress> *pdm;     /**< Protocol dependent module interface */

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
} //eigrp
} //inet
#endif /* EIGRPDUAL_H_ */
