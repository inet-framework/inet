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
	file: IPInterfacePacket.h
	Purpose: Message type to pass from Transport Layer
		to the IP layer; adds IP control information to
		transport packet
	Usage:
	Any transport protocol packet of type cPacket passed to the 
	IP layer should be encapsulated into an IPInterfacePacket with
	encapsulate(cPacket *transpPacket); 
	the destination address needs to be set with setDestAddr(char *dest)
	the decapsulate()-function decapsulates the transport
	protocol packet again after reception.

	the length() of the interface packet is 1 + transpPacket->length()
	the length of the transport packet has to be set in bits

	author: Jochen Reber
	date: 31.5.00
*/

#ifndef __IPINTERFACEPACKET_H__
#define __IPINTERFACEPACKET_H__

#include <string.h>
#include <iostream>

#include "basic_consts.h"

using std::ostream;

/** Protocol field: taken from RFC 1700
 *  Replacing old RFC1700 with newer (28/06/01) Protocol Numbers
 */
enum IPProtocolFieldId
{
  IP_PROT_HOPOPT = 0,
  IP_PROT_ICMP 	= 1,
  IP_PROT_IGMP 	= 2,
  IP_PROT_IP 	= 4, // used for IP tunneling. FIXME: RFC1700 specifies =3 (old IPSuite) ???
  IP_PROT_TCP 	= 6,
  IP_PROT_EGP 	= 8,
  IP_PROT_IGP 	= 9,
  IP_PROT_UDP 	= 17,
  IP_PROT_XTP 	= 36,
  IP_PROT_IPv6 	= 41,
  IP_PROT_RSVP  = 46,
  IP_PROT_IPv6_ICMP = 58,
  IP_PROT_NONE = 59,
  IP_PROT_IPv6_MOBILITY = 62
};

/* 
 * String size to hold an address
 * This is the size of the string that should be used to
 * initialize the IPAddress class, and it is the size of the string
 * returned by IPAddress::getString()
 */
const int ADDRESS_STRING_SIZE = 20;

/*
 * Old definition of IPAddress.
 * IPAddress is now a class defined in ip_address.h
 * IPAddrChar is kept for backward compatibility, but is
 * deprecated.
 */
typedef char IPAddrChar[ADDRESS_STRING_SIZE] ;


/*
 * Send/receive IP call: RFC 791 section 3.3 
 * Excerpt:
 *
 *  SEND (src, dst, prot, TOS, TTL, BufPTR, len, Id, DF, opt => result)
 *  RECV (BufPTR, prot, => result, src, dst, TOS, len, opt)
 *
 *    where:
 *
 *      src = source address
 *      dst = destination address
 *      prot = protocol
 *      TOS = type of service
 *      TTL = time to live
 *      BufPTR = buffer pointer
 *      len = length of buffer
 *      Id  = Identifier
 *      DF = Don't Fragment
 *      opt = option data
 *      result = response
 *        OK = datagram sent/received ok
 *        Error = error in arguments or local network error
 */

/*
 *	Interface class
 *	The class contains control fields that are used by IP
 *	IP Address format: string in dotted number notation 
 *		(e.g. "129.13.35.5")
 *
 *	Used existing fields:
 *		kind: -1 (MK_PACKET)
 *		length: transPacket->length() + 1
 *		priority: 0
 *	required fields:
 *		encapsulation of cPacket
 *		protocol: use IPProtocolFieldId as defined above
 *		destAddr 
 *	optional fields:
 *		srcAddr: default to first interface address
 *		codepoint: used in DS_Field (RFC 2474), instead of TOS; default: 0
 *		timeToLive: default defined, usually 64
 *		dontFragment: default: false
 *	not implemented IP parameters:
 *		options: IP Options currently not used
 *		buffer length
 *		buffer pointer
 *		Identifier: currently always chosen by IP layer
 *		result: currently no result returned
 */
class IPInterfacePacket: public cPacket
{
  private:

	void initValues();

	IPAddrChar _dest;
	IPAddrChar _src;
	IPProtocolFieldId _protocol;
	unsigned char _codepoint;
	short _ttl;
	bool _dontFragment;

  public:
	IPInterfacePacket();
	IPInterfacePacket(const IPInterfacePacket& );

	IPInterfacePacket& operator=(const IPInterfacePacket& ip);
        virtual cObject *dup() const
			{ return new IPInterfacePacket(*this); }

	virtual void info(char *buf);
	virtual void writeContents(ostream& os);

	// override encapsulate/decapsulate for cPacket
	virtual void encapsulate(cPacket *);
	virtual cPacket *decapsulate();

	// accessing control fields
	virtual void setDestAddr(const char *destAddr)
			{ strcpy(_dest, destAddr); }
	virtual const char *destAddr() const { return _dest; } 

	virtual void setSrcAddr(const char *srcAddr)
			{ strcpy(_src, srcAddr); }
	virtual const char *srcAddr() const { return _src; } 

	//void setProtocol (IPProtocolFieldId prot) { _protocol = prot; }
	//IPProtocolFieldId protocol() { return _protocol; }

	void setCodepoint (unsigned char cp) { _codepoint = cp; }
	unsigned char codepoint() { return _codepoint; }

	void setTimeToLive (short ttl) { _ttl = ttl; }
	short timeToLive() { return _ttl; }

	void setDontFragment (bool df) { _dontFragment = df; }
	bool dontFragment() { return _dontFragment; }
};

#endif
