/***************************************************************************
                          setinadapter.h  -  description
                             -------------------
    begin                : Fri Sep 8 2000
    copyright            : (C) 2000 by Dirk Holzhausen
    email                : dholzh@gmx.de
 ***************************************************************************/


#ifndef SETINADAPTER_H
#define SETINADAPTER_H

#include "omnetpp.h"

/**
  *@author Dirk Holzhausen
  */

class SetInAdapter : public cSimpleModule  {

	private:
		int inAdapter;
	
	public:
	
		Module_Class_Members ( SetInAdapter, cSimpleModule, 0 )
		
		virtual void initialize();
		virtual void handleMessage ( cMessage *msg );
	
};

#endif
