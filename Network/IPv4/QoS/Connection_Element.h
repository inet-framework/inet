// -*- C++ -*-
// $Header$
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

#ifndef __CONNECTION_ELEMENT_H__
#define __CONNECTION_ELEMENT_H__


#include <omnetpp.h>

class Connection_Element : public cSimpleModule
{

	public:
    Module_Class_Members ( Connection_Element, cSimpleModule, 0 )

    //virtual void activity();
    virtual void handleMessage ( cMessage *msg );
};


#endif

