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


#include "LIBTableAccess.h"

Define_Module( LIBTableAccess );

void LIBTableAccess::initialize()
{
	cObject *foundmod;
	cModule *curmod = this;


	// find LIB Table
	lt = NULL;
	for (curmod = parentModule(); curmod != NULL;
			curmod = curmod->parentModule())
	{
		if ((foundmod = curmod->findObject("libTable", false)) != NULL)
		{
			lt = (LIBTable *)foundmod;
			ev << "LIBTABLE DEBUG: Lib table found successfully\n";
			break;
		}
	}
	
	if(lt==NULL)
	ev << "LIBTABLE DEBUG: Error, cannot find lib table\n";

}

