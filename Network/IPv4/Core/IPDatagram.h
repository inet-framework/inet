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

/*
	file: IPDatagram.h
	Purpose: Description of the IPDatagram fields, 
		the message type used in IPProcessing
	author: Jochen Reber
	date: 14.5.00
*/


#ifndef __IPDATAGRAM_H
#define __IPDATAGRAM_H

#include <omnetpp.h>

#include "IPInterfacePacket.h"

/* 	-------------------------------------------------
 		Constants
 	------------------------------------------------- 	*/

// default IP header length: 20 bytes
const int DEFAULT_HEADER_LENGTH = 20;
// maximum IP header length: 64 bytes
// should be correct, but I am not completely sure
// needs to be checked
const int MAX_HEADER_LENGTH = 64;
// option entry number
const int MAX_IPADDR_OPTION_ENTRIES = 9;
const int MAX_TIMESTAMP_OPTION_ENTRIES = 4;


/* 	-------------------------------------------------
 		Enumerations
 	------------------------------------------------- 	*/

/* 	IP Option code does not use the same numeric 
	values as the IP Protocol							*/
enum IPOption 
{
	IPOPTION_NO_OPTION,
	IPOPTION_RECORD_ROUTE, 
	IPOPTION_TIMESTAMP, 
	IPOPTION_LOOSE_SOURCE_ROUTING,
	IPOPTION_STRICT_SOURCE_ROUTING
};

/* 	the timestamp flag uses the same numeric values 
	as the IP Protocol									*/
enum TimestampFlag
{
	IP_TIMESTAMP_TIMESTAMP_ONLY = 0,
	IP_TIMESTAMP_WITH_ADDRESS = 1,
	IP_TIMESTAMP_SENDER_INIT_ADDRESS = 3
};

/* 	-------------------------------------------------
 		Type Definitions
 	------------------------------------------------- 	*/

typedef char *IPAddressParameter;


/* 	-------------------------------------------------
 		structures 
 	------------------------------------------------- 	*/

/* 	currently, access to the option fields is "raw" 
	and the calling function needs to do all the work;
	once support for options is implemented, 
	wrapper functions may be added						*/

// Option structure: Record Route
struct RecordRouteOptionField
{
	IPAddrChar recordAddress [MAX_IPADDR_OPTION_ENTRIES];
	short nextAddressPtr;
};

// Option structure: Timestamp
struct TimestampOptionField
{
	TimestampFlag flag;
	short overflow;
	short nextAddressPtr;

	/* 	use either up to 4 addresses with timestamps or
		only up to 9 timestamps, according to the flag */	
	IPAddrChar recordAddress[MAX_TIMESTAMP_OPTION_ENTRIES];
	simtime_t recordTimestamp[MAX_IPADDR_OPTION_ENTRIES];
};

// Option Structure: Source Routing
struct SourceRoutingOptionField
{

	IPAddrChar recordAddress[MAX_IPADDR_OPTION_ENTRIES];
	short nextAddressPtr;
	short lastAddressPtr;
};
	
// Differentiated Services Field (old ToS)
struct DSField
{
	unsigned char codepoint;
	unsigned char unused;
};

/* 	-------------------------------------------------
 		Main class: IPDatagram
 	-------------------------------------------------

	used existing fields:
	length () / setLength(err)  in bits
	hasBitError() / setBitError()
	protocol() / setProtocol(short p) is always PR_IP
	kind() always set to MK_PACKET
	timestamp() / setTimestamp (simtime) used in timestamp option

	void *context / contextPointer() / setContextPointer (*void)
		currently optional

	transportProtocol: use of enumeration in protocol.h?
		ICMP?

	additional length fields defined in this class
	use the unit bytes (totalLength()=length()/8 and header_length) or
	8 bytes (fragment_offset)


*/

class IPDatagram: public cPacket
{
private:

	// Required header fields
	short version_field;
	short header_length;
	DSField ds;
	int identification;
	bool more_fragments;
	bool dont_fragment;
	int fragment_offset;
	short time_to_live;
	IPProtocolFieldId transport_protocol;
	IPAddrChar src_address;
	IPAddrChar dest_address;
	IPOption option_code;

	// internal fields
	int outputPortNo;
	int inputPortNo;
	
	// option fields in IP header
	union {
		RecordRouteOptionField record_route;
		TimestampOptionField option_timestamp;
		SourceRoutingOptionField source_routing;	
	};
	
	void setOptionHeaderLength();
	void initValues();

public:

	// constructors
	IPDatagram();
	IPDatagram(const IPDatagram &d);

	// assignment operator
	virtual IPDatagram& operator=(const IPDatagram& d);
        virtual cObject *dup() const { return new IPDatagram(*this); }
	
	// info functions
	virtual void info(char *buf);
	virtual void writeContents(ostream& os);

	// overriding encapsulation-function for cPackets
	virtual void encapsulate(cPacket *);
	virtual cPacket *decapsulate();

	// IP header fields:
	
	// first word header fields
	short version() { return version_field; }
	
	// header length is always set automatically
	short headerLength() { return header_length; }

	// length of Datagram in bytes
	int totalLength() { return length()/8; }
	void setTotalLength(int l); 

	unsigned char codepoint() { return ds.codepoint; }
	void setCodepoint(unsigned char cp) { ds.codepoint = cp; }
	
	// fragmentation header fields
	int fragmentId() { return identification; }
	void setFragmentId(int id) { identification = id; }
	
	bool moreFragments() { return more_fragments; }
	void setMoreFragments(bool moreFragments) 
			{ more_fragments = moreFragments; }

	bool dontFragment() { return dont_fragment; }
	void setDontFragment(bool dontFragment)
			{ dont_fragment = dontFragment; }

	int fragmentOffset() { return fragment_offset; }
	void setFragmentOffset (int offset) 
			{ fragment_offset = offset; }

	// third word header fields
	short timeToLive() { return time_to_live; }
	void setTimeToLive(short ttl) { time_to_live= ttl; }
	
	IPProtocolFieldId transportProtocol() 
			{ return transport_protocol; }
	void setTransportProtocol(IPProtocolFieldId prot)
			{ transport_protocol = prot; }

	// header checksum not required
	
	// source and destination address
	IPAddressParameter srcAddress() { return src_address; }
        //void setSrcAddress(const IPAddressParameter src);
        void setSrcAddress(const char* src);

	IPAddressParameter destAddress() { return dest_address; }
	// void setDestAddress(IPAddressParameter dest);
        void setDestAddress(const char* dest);

	// output port no: control info; not in IP header
	int outputPort() { return outputPortNo; }
	void setOutputPort(int p) { outputPortNo = p; }

	// input port no: control info; not in IP header; -1 for IPSend
	int inputPort() { return inputPortNo; }
	void setInputPort(int p) { inputPortNo = p; }


	// fields for options
	bool hasOption();
	IPOption optionType() { return option_code; }
	
	/* 	if no option exists, return NULL, 
		otherwise, return pointer to appropriate structure;
		conversion needs to be done by calling function	*/
	void *optionField();

	// existing option is overwritten when setXXXOption is called
	void setRecordRoute (const RecordRouteOptionField&);
	void setTimestampOption (const TimestampOptionField&);
	void setSourceRoutingOption 
		(bool looseSourceRouting, const SourceRoutingOptionField&);

};

#endif

