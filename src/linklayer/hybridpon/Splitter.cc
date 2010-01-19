// $Id$
//-------------------------------------------------------------------------------
//	Splitter.cc --
//
//	This file implements 'Splitter' class.
//
//	Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//-------------------------------------------------------------------------------


#include "Splitter.h"


// Register module.
Define_Module(Splitter)


void Splitter::initialize()
{

}


void Splitter::handleMessage(cMessage *msg)
{
	send(msg, "toMac");
}


void Splitter::finish()
{

}
