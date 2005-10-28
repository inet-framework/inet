/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/

#ifndef __OSPFROUTING__H__
#define __OSPFROUTING__H__

#include <vector>
#include <omnetpp.h>
#include "RoutingTable.h"
#include "RoutingTableAccess.h"
#include "OSPFPacket_m.h"
#include "OSPFRouter.h"

/**
 * OMNeT++ module class acting as a facade for the OSPF datastructure.
 * Handles the configuration loading and forwards the OMNeT++ messages (OSPF packets).
 */
class OSPFRouting :  public cSimpleModule
{
  private:
    RoutingTableAccess  routingTableAccess; ///< Provides access to the IP routing table.
    OSPF::Router*       ospfRouter;         ///< Root object of the OSPF datastructure.

    void    GetAreaListFromXML (const cXMLElement& routerNode, std::map<std::string, int>& areaList) const;
    void    LoadAreaFromXML (const cXMLElement& asConfig, const std::string& areaID);
    void    LoadInterfaceParameters (const cXMLElement& ifConfig);
    void    LoadExternalRoute (const cXMLElement& externalRouteConfig);
    void    LoadHostRoute (const cXMLElement& hostRouteConfig);
    void    LoadVirtualLink (const cXMLElement& virtualLinkConfig);

    bool    LoadConfigFromXML (const char * filename);

  public:
    Module_Class_Members(OSPFRouting, cSimpleModule, 0);

    virtual ~OSPFRouting (void);

    /** Returns 3 - this module needs 3 stages for complete initialization. */
    virtual int numInitStages() const  {return 3;}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);
};

#endif  // __OSPFROUTING__H__


