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

/* 	--------------------------------------------------------
	file: IPDatagram.cc
	Purpose: Implementation of the access routines of the 
		 IPDatagram class
	author: Jochen Reber
	date: 14.5.00
 	--------------------------------------------------------	*/


#include <string.h>
#include <stdio.h>

#include "omnetpp.h"
#include "IPDatagram.h"


/* 	Constructor
	sets explicit values protocol, version, headerLength
	all other fields are set to 0/false	*/

IPDatagram::IPDatagram() : cPacket()
{
	initValues();
}

// Copy Constructor
IPDatagram::IPDatagram(const IPDatagram& d ) : cPacket()
{
	setName( d.name() );
	operator=( d );
}

/* encapsulate a packet of type cPacket of the Transport Layer,
	transportProtocol is set
	assumes that transportPacket->length() is
	length of transport packet in bits */
void IPDatagram::encapsulate(cPacket *transportPacket)
{
	cPacket::encapsulate(transportPacket);
	transport_protocol = (IPProtocolFieldId)transportPacket->protocol();
}

cPacket *IPDatagram::decapsulate()
{
	return (cPacket *)(cPacket::decapsulate());
}

// private function for initialisation
void IPDatagram::initValues()
{
	header_length = DEFAULT_HEADER_LENGTH; 
	setLength(header_length*8);
	setProtocol( PR_IP );
	version_field = 4;
	ds.codepoint = 0;
	ds.unused = 0;
	// Fragmentation information has to be set seperately
	identification = 0; 
	more_fragments = false;
	dont_fragment = false;
	fragment_offset = 0;
	time_to_live = 0;
	transport_protocol = IP_PROT_NONE;
	outputPortNo = 0;
	strcpy( src_address, "0.0.0.0" );
	strcpy( dest_address, "0.0.0.0");
	option_code = IPOPTION_NO_OPTION;
    inputPortNo = -1;
}

// assignment operator
IPDatagram& IPDatagram::operator= (const IPDatagram& d)
{
	if (this==&d) return *this;

	cPacket::operator=(d);
	
    version_field = d.version_field;
    header_length = d.header_length;
    ds = d.ds;
    identification = d.identification;
    more_fragments = d.more_fragments;
    dont_fragment = d.dont_fragment;
    fragment_offset = d.fragment_offset;
    time_to_live = d.time_to_live;
    transport_protocol = d.transport_protocol;
    strcpy (src_address, d.src_address);
    strcpy (dest_address, d.dest_address);
	outputPortNo = d.outputPortNo;
	option_code = d.option_code;
    inputPortNo = d.inputPortNo;    

	switch(d.option_code)
	{
		case IPOPTION_RECORD_ROUTE:
			setRecordRoute (d.record_route);
			break;
		case IPOPTION_TIMESTAMP:
			setTimestampOption (d.option_timestamp);
			break;
		case IPOPTION_LOOSE_SOURCE_ROUTING:
			setSourceRoutingOption (true, d.source_routing);
			break;
		case IPOPTION_STRICT_SOURCE_ROUTING:
			setSourceRoutingOption (false, d.source_routing);
			break;
		default:
			break;
	}

	return *this;
}

// output function for datagram
void IPDatagram::writeContents( ostream& os )
{
	os	<< "IP Datagram:"
		<< "\nSender: "	<< senderModuleId()
		<< "\nBit length: " << length() 
		<< "Byte len: " << totalLength()
		<< "Header len:" << header_length
		<< "\nTransport Prot: " << (int)transport_protocol
		<< "TTL: " << time_to_live
		<< "\nid: "		<< identification
		<< "offset: "	<< fragment_offset
		<< "\nSource addr: " << src_address
		<< "\nDestination addr: "<< dest_address
		<< "\n";

}

void IPDatagram::info(char *buf)
{
	cPacket::info( buf );
	sprintf( buf +strlen(buf), "Source=%s Dest=%s",
		src_address, dest_address );
}

/* 	--------------------------------------------------------
		Setting and getting header fields
 	--------------------------------------------------------	*/

void IPDatagram::setTotalLength(int l)
{ 
	setLength( l * 8 );
}

void IPDatagram::setSrcAddress(const char* src)
{
	strcpy( src_address, src );
}

void IPDatagram::setDestAddress(const char* dest)
{
	strcpy( dest_address, dest ); 
}


// Option Fields
bool IPDatagram::hasOption()
{ return (!(option_code == IPOPTION_NO_OPTION)); }

void *IPDatagram::optionField()
{ 
	void *optionField=NULL;

	switch(option_code) {
	case IPOPTION_NO_OPTION:
		optionField = NULL;
		break;
	case IPOPTION_RECORD_ROUTE:
		optionField = (void *) (&record_route);
		break;
	case IPOPTION_TIMESTAMP:
		optionField = (void *) (&option_timestamp);
		break;
	case IPOPTION_LOOSE_SOURCE_ROUTING:
		optionField = (void *) (&source_routing);
		break;
	case IPOPTION_STRICT_SOURCE_ROUTING:
		optionField = (void *) (&source_routing);
		break;
	}

	return optionField;
}

// private function
void IPDatagram::setOptionHeaderLength()
{

	// HeaderLength is set automatically to maximum with option
	setLength(length() - headerLength()*8);
	header_length = MAX_HEADER_LENGTH;
	setLength(length() + headerLength()*8);
}

void IPDatagram::setRecordRoute (const RecordRouteOptionField& option)
{
	int i;

	setOptionHeaderLength();
	option_code = IPOPTION_RECORD_ROUTE;

	for (i = 0; i < MAX_IPADDR_OPTION_ENTRIES; i++)
		strcpy(record_route.recordAddress[i], option.recordAddress[i]);
	record_route.nextAddressPtr = option.nextAddressPtr;
	
}

void IPDatagram::setTimestampOption (const TimestampOptionField& option)
{
	int i;

	setOptionHeaderLength();
	option_code = IPOPTION_RECORD_ROUTE;

	for (i = 0; i < MAX_IPADDR_OPTION_ENTRIES; i++)
		strcpy(option_timestamp.recordAddress[i], 
				option.recordAddress[i]);

	for (i = 0; i < MAX_TIMESTAMP_OPTION_ENTRIES; i++)
		option_timestamp.recordTimestamp[i] =
			option.recordTimestamp[i];

	option_timestamp.flag = option.flag;
	option_timestamp.overflow = option.overflow;
	option_timestamp.nextAddressPtr = option.nextAddressPtr;
}

void IPDatagram::setSourceRoutingOption 
		(bool looseSourceRouting, const SourceRoutingOptionField& option)
{
	int i;

	setOptionHeaderLength();
	option_code = looseSourceRouting ? IPOPTION_LOOSE_SOURCE_ROUTING :
			IPOPTION_STRICT_SOURCE_ROUTING;

	for (i = 0; i < MAX_IPADDR_OPTION_ENTRIES; i++)
		strcpy(source_routing.recordAddress[i], option.recordAddress[i]);

	source_routing.nextAddressPtr = option.nextAddressPtr;
	source_routing.lastAddressPtr = option.lastAddressPtr;
}

