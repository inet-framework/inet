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

/*
	file: PPPModule.h
	Purpose: Header file for PPPModule
	Responsibilities:
	comment: test stub version only, no L2 header or fragment
		 no PPP done, just send raw IPDatagrams over
		 point-to-point link

	author: Jochen Reber
*/

#ifndef __PPP_MODULE_H__
#define __PPP_MODULE_H__

#include <omnetpp.h>

#include "basic_consts.h"
#include "RoutingTableAccess.h"


// note: RoutingTableAccess needed later, because of Routing Table
// (or not really, since PPP doesn't require L2-addies)
class PPPModule: public RoutingTableAccess
{
private:
	simtime_t delay;

public:
	Module_Class_Members(PPPModule, RoutingTableAccess, ACTIVITY_STACK_SIZE);

	virtual void initialize();
	virtual void activity();
};

#endif

