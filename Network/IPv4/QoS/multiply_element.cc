/***************************************************************************
                          multiply_element.cc  -  description
                             -------------------
    begin                : Mon Aug 7 2000
    copyright            : (C) 2000 by Dirk A. Holzhausen
    email                : dirk.holzhausen@gmx.de
 ***************************************************************************/



#include "multiply_element.h"
#include "omnetpp.h"

Define_Module ( Multiply_Element );

void Multiply_Element::initialize () {
	cntGates = gates () - 1;
}

void Multiply_Element::handleMessage ( cMessage *msg ) {
		
		for ( int i = 0; i != cntGates; i++ )
			send ( new cMessage ( *msg ) , "out", i );

		delete msg;
}
