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

/***************************************************************************
                          FIFO_Queue.h  -  description
                             -------------------
    begin                : Wed Jul 26 2000
    author               : Dirk A. Holzhausen
    email                : dirk.holzhausen@gmx.de


		based on FIFO_Queue2 (-> Klaus Wehrle)

 ***************************************************************************/


#ifndef FIFO_QUEUE_H
#define FIFO_QUEUE_H

#include "omnetpp.h"
#include "any_queue.h"

class FIFO_Queue : public ANY_Queue {

private:
	int pckEnqCnt;
	int pckDeqCnt;

	long		max_length;
	cQueue	*fifo_queue;
	long  	queue_length;
	

public:

	FIFO_Queue ( const char *name, cModule *parentmodule, unsigned stacksize = 0 );

	
	// own stuff
		
	virtual void startup ();							// own initialization routine
	
	virtual bool 				insert(cMessage *);
	virtual	cMessage 		*pop();
	virtual cMessage 		*tail();
	virtual	cMessage 		*head();
	virtual	bool 				isEmpty();
  virtual	long				length() const;

	virtual long 				length ( const char *s ) const { return length (); }
  virtual bool				insert ( const char *s, cMessage *msg ) { return insert ( msg ); }

	virtual long				getMaxQueueLength () { return max_length; }

};

#endif

