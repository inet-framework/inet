 /**
******************************************************
* @file Cache1QAccess.h
* @brief B-Component cache access
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2010
******************************************************/
#ifndef __CACHE1AH_ACCESS_H
#define __CACHE1AH_ACCESS_H



#include "ModuleAccess.h"
#include "Cache1AH.h"


/**
 * @brief Gives access to the Cache1AH
 */
class Cache1AHAccess : public ModuleAccess<Cache1AH>
{
    public:
    	 Cache1AHAccess() : ModuleAccess<Cache1AH>("cache") {}
};

#endif
