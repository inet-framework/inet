#pragma once

#include "ModuleAccess.h"

/**
 * Gives access to OSPF module.
 */

class INET_API BGPRoutingAccess : public ModuleAccess<cSimpleModule>
{
    public:
		BGPRoutingAccess() : ModuleAccess<cSimpleModule>("bgp") {}
		//BGPRoutingAccess(unsigned int index) : ModuleAccess2<cSimpleModule>("bgp", index) {}
};