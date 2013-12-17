#ifndef __PORTFILTRSTPACCESS_H_
#define __PORTFILTRSTPACCESS_H_


#include "ModuleAccess.h"
#include "PortFiltRSTP.h"


/**
 * @brief Gives access to the PortFiltRSTP
 */
class PortFiltRSTPAccess : public ModuleAccess<PortFiltRSTP>
{
    public:
    	 PortFiltRSTPAccess() : ModuleAccess<PortFiltRSTP>("PortFilt") {}
};

#endif