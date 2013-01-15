 /**
******************************************************
* @file Relay1QAccess.h
* @brief Relay access
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#ifndef __RELAY1Q_ACCESS_H
#define __RELAY1Q_ACCESS_H


#include "ModuleAccess.h"
#include "Relay1Q.h"


/**
 * @brief Gives access to the Relay1Q
 */
class Relay1QAccess : public ModuleAccess<Relay1Q>
{
    public:
    	 Relay1QAccess() : ModuleAccess<Relay1Q>("relay") {}
};

#endif
