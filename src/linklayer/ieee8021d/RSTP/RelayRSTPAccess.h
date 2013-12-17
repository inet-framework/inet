#ifndef __RELAYRSTP_ACCESS_H
#define __RELAYRSTP_ACCESS_H


#include "ModuleAccess.h"
#include "RelayRSTP.h"


/**
 * @brief Gives access to Admacrelay module
 */
class RelayRSTPAccess : public ModuleAccess<RelayRSTP>
{
    public:
    	 RelayRSTPAccess() : ModuleAccess<RelayRSTP>("relay") {}
};

#endif
