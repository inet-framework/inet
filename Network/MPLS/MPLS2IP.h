/*******************************************************************
*
*	This library is free software, you can redistribute it 
*	and/or modify 
*	it under  the terms of the GNU Lesser General Public License 
*	as published by the Free Software Foundation; 
*	either version 2 of the License, or any later version.
*	The library is distributed in the hope that it will be useful, 
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
*	See the GNU Lesser General Public License for more details.
*
*
*********************************************************************/
/*
*	File Name MPLS2IP.h
*	LDP library
*	This file defines MPLS2IP class
**/
#ifndef __MPLS2IP_MODULE_H__
#define __MPLS2IP_MODULE_H__

#include <omnetpp.h>
#include <RoutingTableAccess.h>

class MPLS2IP: public RoutingTableAccess
{
private:
	simtime_t delay;

public:
	Module_Class_Members(MPLS2IP, RoutingTableAccess, ACTIVITY_STACK_SIZE);

	virtual void initialize();
	virtual void activity();
};

#endif