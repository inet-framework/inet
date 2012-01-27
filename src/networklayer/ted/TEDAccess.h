//
// (C) 2005 Vojtech Janota
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#ifndef __INET_TEDACCESS_H
#define __INET_TEDACCESS_H

#include "INETDefs.h"

#include "ModuleAccess.h"
#include "TED.h"

/**
 * Provides access to the TED module.
 */
class TEDAccess : public ModuleAccess<TED>
{
    public:
        TEDAccess() : ModuleAccess<TED>("ted") {}
};

#endif

