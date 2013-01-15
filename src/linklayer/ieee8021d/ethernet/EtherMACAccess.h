 /**
******************************************************
* @file EtherMACAccess.h
* @brief Gives access to mac module.
*
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
*
*
******************************************************/
#ifndef __ETHERMAC_ACCESS_H
#define __ETHERMAC_ACCESS_H



#include "ModuleAccess.h"
#include "EtherMAC.h"


/**
 * @brief Gives access to the EtherMAC
 */
class EtherMACAccess : public ModuleAccess<EtherMAC>
{
    public:
    	 EtherMACAccess() : ModuleAccess<EtherMAC>("mac") {}
};

#endif
