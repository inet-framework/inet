/*******************************************************************
*
*    This library is free software, you can redistribute it
*    and/or modify
*    it under  the terms of the GNU Lesser General Public License
*    as published by the Free Software Foundation;
*    either version 2 of the License, or any later version.
*    The library is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*    See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/

#ifndef __MPLSACCESS_H__
#define __MPLSACCESS_H__

#include <omnetpp.h>
#include "ModuleAccess.h"
#include "MPLSModule.h"


/**
 * Gives access to the MPLS module.
 */
class MPLSAccess : public ModuleAccess<MPLSModule>
{
    public:
        MPLSAccess() : ModuleAccess<MPLSModule>("mplsModule") {}
};

#endif

