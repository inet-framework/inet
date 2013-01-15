 /**
******************************************************
* @file MVRPAccess.h
* @brief MVRP access
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#ifndef __MVRP_ACCESS_H
#define __MVRP_ACCESS_H

#include "ModuleAccess.h"
#include "MVRP.h"


/**
 * @brief Gives access to MVRP module
 */
class MVRPAccess : public ModuleAccess<MVRP>
{
    public:
    	 MVRPAccess() : ModuleAccess<MVRP>("mvrp") {}
};

#endif
