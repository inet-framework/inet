#pragma once

#include "ModuleAccess.h"
#include "OSPFRouting.h"

/**
 * Gives access to OSPF module.
 */
class INET_API OSPFRoutingAccess : public ModuleAccess<OSPFRouting>
{
    public:
        OSPFRoutingAccess() : ModuleAccess<OSPFRouting>("ospf") {}
        //OSPFRoutingAccess(unsigned int index) : ModuleAccess2<cSimpleModule>("ospf", index) {}
};

