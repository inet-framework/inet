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
    file: Netfilters.cc
    Purpose: quick implementation of IP Processing netfilters for 
             test purposes all they do is receive a message and then send it
	     back again
    author: Jochen Reber
*/

#include "omnetpp.h"

#include "Netfilters.h"

// * Pre Routing Netfilter *
Define_Module( NF_IP_PRE_ROUTING );

void NF_IP_PRE_ROUTING::activity()
{
	cMessage *msg;

	while(true)
	{
		msg = receive();
		send(msg, "out");
	}
}

// * Routing Netfilter *
Define_Module( NF_IP_FORWARD );

void NF_IP_FORWARD::activity()
{
	cMessage *msg;

	while(true)
	{
		msg = receive();
		send(msg, "out");
	}
}

// * Local Deliver Netfilter *
Define_Module( NF_IP_LOCAL_IN );

void NF_IP_LOCAL_IN::activity()
{
	cMessage *msg;

	while(true)
	{
		msg = receive();
		send(msg, "out");
	}
}

// * IPSend Netfilter *
Define_Module( NF_IP_LOCAL_OUT );

void NF_IP_LOCAL_OUT::activity()
{
	cMessage *msg;

	while(true)
	{
		msg = receive();
		send(msg, "out");
	}
}

// * IPOutput Netfilter *
Define_Module( NF_IP_POST_ROUTING );

void NF_IP_POST_ROUTING::activity()
{
	cMessage *msg;

	while(true)
	{
		msg = receive();
		send(msg, "out");
	}
}

