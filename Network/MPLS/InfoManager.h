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
#ifndef __INFOMANAGER_H
#define __INFOMANAGER_H

#include <omnetpp.h>

class InfoManager: public cSimpleModule
{

public:
	Module_Class_Members(InfoManager, cSimpleModule, 0);
	void initialize();
    void handleMessage(cMessage *msg);





};


#endif
