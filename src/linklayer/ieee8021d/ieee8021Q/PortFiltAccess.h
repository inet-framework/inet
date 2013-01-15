 /**
******************************************************
* @file PortFiltAccess.h
* @brief PortFilt access
* @author Juan Luis Garrote Molinero
* @version 1.0
* @date Feb 2011
******************************************************/
#ifndef __PortFilt_ACCESS_H
#define __PortFilt_ACCESS_H



#include "ModuleAccess.h"
#include "PortFilt.h"


/**
 * @brief Gives access to PortFilt module
 */
class PortFiltAccess : public ModuleAccess<PortFilt>
{
    public:
    	 PortFiltAccess() : ModuleAccess<PortFilt>("PortFilt") {}
};

#endif
