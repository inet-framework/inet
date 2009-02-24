//
// (C) 2005 Vojtech Janota
// (C) 2003 Xuan Thang Nguyen
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

#ifndef __INET_LIB_TABLE_ACCESS_H
#define __INET_LIB_TABLE_ACCESS_H

#include <omnetpp.h>
#include "ModuleAccess.h"
#include "LIBTable.h"


/**
 * Gives access to the LIBTable.
 */
class INET_API LIBTableAccess : public ModuleAccess<LIBTable>
{
    public:
        LIBTableAccess() : ModuleAccess<LIBTable>("libTable") {}
};

#endif

