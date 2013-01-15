 /**
******************************************************
* @file Cache1QAccess.h
* @brief Admacrelay access
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#ifndef __CACHE1Q_ACCESS_H
#define __CACHE1Q_ACCESS_H



#include "ModuleAccess.h"
#include "Cache1Q.h"


/**
 * @brief Gives access to the Cache1Q
 */
class Cache1QAccess : public ModuleAccess<Cache1Q>
{
    public:
    	 Cache1QAccess() : ModuleAccess<Cache1Q>("cache") {}
};

#endif
