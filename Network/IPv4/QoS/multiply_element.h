/***************************************************************************
                          multiply_element.h  -  description
                             -------------------
    begin                : Mon Aug 7 2000
    copyright            : (C) 2000 by Dirk A. Holzhausen
    email                : dirk.holzhausen@gmx.de
 ***************************************************************************/



#ifndef MULTIPLY_ELEMENT_H
#define MULTIPLY_ELEMENT_H

#include "omnetpp.h"

/**
  *@author Dirk A. Holzhausen
  */

class Multiply_Element : public cSimpleModule {

	private:
		int cntGates;

	public:
		
		Module_Class_Members ( Multiply_Element, cSimpleModule, 0 )
	
		virtual void initialize ();
		virtual void handleMessage ( cMessage *msg );
};

#endif
