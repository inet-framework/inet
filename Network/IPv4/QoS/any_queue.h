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
                          any_queue.h  -  description
                             -------------------
    begin                : Thu Aug 24 2000
    author               : Dirk Holzhausen
    email                : dholzh@gmx.de
 ***************************************************************************/



#ifndef ANY_QUEUE_H
#define ANY_QUEUE_H

#include "omnetpp.h"

/**basic class for queue modules
  *@author Dirk Holzhausen
  */

class ANY_Queue : public cSimpleModule  {

 public:
  ANY_Queue( const char *name, cModule *parentmodule, unsigned stacksize = 0 );
		
  virtual void initialize ( int stage );
  virtual int numInitStages () const { return 2; }
		
		
  // abstract methods
		
  virtual void startup() = 0;
		
		virtual bool 				insert(cMessage *) 	= 0;
		virtual	cMessage 		*pop()     					= 0;
		virtual cMessage 		*tail()   					= 0;
		virtual	cMessage 		*head()   					= 0;
		virtual	bool 				isEmpty() 					= 0;
		virtual	long 				length() const	= 0;

		virtual long 				getMaxQueueLength () = 0;
		
  virtual long length ( const char *s ) const { return length (); }
  virtual bool insert ( const char *s, cMessage *msg ) { return insert ( msg ); }
		
};

#endif
