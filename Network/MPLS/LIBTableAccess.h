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
#ifndef __LIB_TABLE_ACCESS_H__
#define __LIB_TABLE_ACCESS_H__

#include <omnetpp.h>

#include "LIBtable.h"

class LIBTableAccess: public cSimpleModule
{
private:

protected:

    LIBTable *lt;

public:
    Module_Class_Members(LIBTableAccess, cSimpleModule, 16384);

    virtual void initialize();

};

#endif

