 /**
******************************************************
* @file AdmacrelayAccess.h
* @brief Admacrelay access
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#ifndef __ADMACRELAY_ACCESS_H
#define __ADMACRELAY_ACCESS_H


#include "ModuleAccess.h"
#include "Admacrelay.h"


/**
 * @brief Gives access to Admacrelay module
 */
class AdmacrelayAccess : public ModuleAccess<Admacrelay>
{
    public:
    	 AdmacrelayAccess() : ModuleAccess<Admacrelay>("admacrelay") {}
};

#endif
