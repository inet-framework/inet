// -*- C++ -*-
//
//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

/*
    file: AppIn.h
    Purpose: receives packets of Traffic generators
            and prints out their information
    author: Jochen Reber
*/

#ifndef __APPIN_H__
#define __APPIN_H__

#include <omnetpp.h>

#include "basic_consts.h"
#include "IPInterfacePacket.h"

class AppIn: public cSimpleModule
{
    void processMessage(cMessage *);
public:
    Module_Class_Members(AppIn, cSimpleModule, ACTIVITY_STACK_SIZE);

    void initialize();
    void activity();

};

#endif
