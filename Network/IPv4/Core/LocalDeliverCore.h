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

// -------------------------------------------------
// file: LocalDeliverCore.h
// header file for the Simple Module LocalDeliverCore
// ------
// Responsibilities: 
// Receive IP datagram for local delivery
// strip off IP header
// buffer fragments for ip_fragmenttime
// wait until all fragments of one fragment number are received
// discard without notification if not all fragments arrive in 
//		ip_fragmenttime
// Defragment once all fragments have arrived
// send Transport packet up to the transport layer
// send ICMP packet to ICMP module
// send IGMP group management packet to Multicast module
// send tunneled IP datagram to PreRouting
// author: Jochen Reber
// -------------------------------------------------

#ifndef __LOCALDELIVERCORE_H__
#define __LOCALDELIVERCORE_H__

#include "IPDatagram.h"
#include "ProcessorAccess.h"

/*  -------------------------------------------------
        Constants
    -------------------------------------------------   */

const int FRAGMENT_BUFFER_MAXIMUM = 1000;

/*  -------------------------------------------------
        structures
    -------------------------------------------------   */
struct FragmentationBufferEntry
{
	bool isFree;

	int fragmentId;
	int fragmentOffset; // unit: 8 bytes
	bool moreFragments;
	int length; // length of fragment excluding header, in byte

	simtime_t timeout;
};

/*  -------------------------------------------------
        Main Module class: LocalDeliverCore
    -------------------------------------------------	*/

class LocalDeliverCore: public ProcessorAccess
{
private:
	simtime_t fragmentTimeoutTime;
	simtime_t delay;
	bool hasHook;

	int fragmentBufSize;
	FragmentationBufferEntry fragmentBuf [FRAGMENT_BUFFER_MAXIMUM];
protected:
	IPInterfacePacket *setInterfacePacket
			(IPDatagram *);

	// functions to handle Fragmentation Buffer

	void eraseTimeoutFragmentsFromBuf();
	void insertInFragmentBuf(IPDatagram *d);
	int getPayloadSizeFromBuf(int fragmentId);
	bool datagramComplete(int fragmentId);
	void removeFragmentFromBuf(int fragmentId);


public:
	Module_Class_Members(LocalDeliverCore, ProcessorAccess, 
				ACTIVITY_STACK_SIZE);

	virtual void initialize();
	virtual void activity();

};

#endif
