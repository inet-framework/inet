// -*- C++ -*-
//
// Copyright (C) 2001  Vincent Oberle (vincent@oberle.com)
// Institute of Telematics, University of Karlsruhe, Germany.
// University Comillas, Madrid, Spain.
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
 *  File: ip_address.cc
 *	Purpose: Represention of an IPv4 address
 *	Author: Vincent Oberle
 *	Date: Jan-March 2001
 */

#include "ip_address.h"

// IPAddress functions

Register_Class( IPAddress );

IPAddress::IPAddress(int ip, const char *n) : cObject(n)
{
	addr[0] = (ip >> 24) & 0xFF;
	addr[1] = (ip >> 16) & 0xFF;
	addr[2] = (ip >> 8) & 0xFF;
	addr[3] = ip & 0xFF;
}

int IPAddress::getInt () const
{
	return (addr[0] << 24)
		+  (addr[1] << 16)
		+  (addr[2] << 8)
		+  (addr[3]);
}

IPAddress::IPAddress(int i0, int i1, int i2, int i3, const char *n) : cObject(n)
{
	addr[0] = i0;
	addr[1] = i1;
	addr[2] = i2;
	addr[3] = i3;
}

IPAddress::IPAddress(const char *text, const char *n) : cObject(n)
{
	if (text == NULL)
          opp_error("IP address string is NULL");

	int i, idx;
	idx = 0;
	for (i = 0; i < 4; i++) {
		addr[i] = 0;
		while ( (text[idx] != '.') && (text[idx] != '\0') ) {
			if ((text[idx] < '0') || (text[idx] > '9'))
                          opp_error("Incorrect IP address string: %s", text);
			addr[i] = addr[i] * 10 + (text[idx] - '0');
			//ev <<"text[ " <<idx <<"] = " <<text[idx] 
			//   <<" addr[" <<i <<"] = " <<addr[i] <<endl;
			idx++;
		}
		idx++;
	}
	return;
}


IPAddress::IPAddress(const IPAddress& obj) : cObject()
{
	setName(obj.name());
	operator=(obj);
}

void IPAddress::info(char *buf)
{
	cObject::info(buf);
	sprintf(buf+strlen(buf), "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
}

void IPAddress::writeContents(ostream& os) const
{
	os <<addr[0] <<'.' <<addr[1] <<'.' <<addr[2] <<'.' <<addr[3] <<"\n";
}

IPAddress& IPAddress::operator=(const IPAddress& obj)
{
	cObject::operator=(obj);
	addr[0] = obj.addr[0];
	addr[1] = obj.addr[1];
	addr[2] = obj.addr[2];
	addr[3] = obj.addr[3];
	return *this;
}

/* Returns a string of the size ADDRESS_STRING_SIZE representing the address */
const char *IPAddress::getString() const
{
	// char *c = new char[ADDRESS_STRING_SIZE];
	sprintf(addrString, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
	return addrString;
}


/* Bit to bit comparaison */
bool IPAddress::isEqualTo(const IPAddress& toCmp) const
{
  
  if ((addr[0] == toCmp.addr[0]) &&
      (addr[1] == toCmp.addr[1]) &&
      (addr[2] == toCmp.addr[2]) &&
      (addr[3] == toCmp.addr[3]))
    return true;
  else
    return false;
}

/*
 * Does a bit to bit and between both addresses.
 * NB: Is not called and because and is a C++ keyword.
 */
IPAddress IPAddress::doAnd(const IPAddress& ip) const
{
	return IPAddress(addr[0] & ip.addr[0],
                         addr[1] & ip.addr[1],
                         addr[2] & ip.addr[2],
                         addr[3] & ip.addr[3]);
}


/*
 * Returns the network class of the address in the form of a
 * char 'A', 'B', 'C', 'D' or 'E' (case sensitive)
 */
char IPAddress::getIPClass() const
{
	if ((addr[0] & 0x80) == 0x00)		// 0xxxx
		return 'A';
	else if ((addr[0] & 0xC0) == 0x80)	// 10xxx
		return 'B';
	else if ((addr[0] & 0xE0) == 0xC0)	// 110xx
		return 'C';
	else if ((addr[0] & 0xF0) == 0xE0)	// 1110x
		return 'D';
	else if ((addr[0] & 0xF8) == 0xF0)	// 11110
		return 'E';
	else {
		// limited diffusion address probably (all 1)
		// but we don't know exactly
		ev <<"Couldn't determine the class of the IP address";
		return 0;
	}
}

/*
 * Returns a new address with the network part of the address
 * (the bits of the hosts part are to 0)
 */
IPAddress *IPAddress::getNetwork() const
{
	switch (getIPClass()) {
	case 'A':
		// Class A: network = 7 bits
		return new IPAddress(addr[0], 0, 0, 0);
	case 'B':
		// Class B: network = 14 bits
		return new IPAddress(addr[0], addr[1], 0, 0);
	case 'C':
		// Class C: network = 21 bits
		return new IPAddress(addr[0], addr[1], addr[2], 0);
	default:
		// Class D or E
		return NULL;
	}
}

/*
 * Returns the network mask corresponding to the address class.
 */
IPAddress* IPAddress::getNetworkMask() const
{
	switch (getIPClass()) {
	case 'A':
		// Class A: network = 7 bits
		return new IPAddress(255, 0, 0, 0);
	case 'B':
		// Class B: network = 14 bits
		return new IPAddress(255, 255, 0, 0);
	case 'C':
		// Class C: network = 21 bits
		return new IPAddress(255, 255, 255, 0);
	default:
		// Class D or E
		return NULL;
	}
}


/* Indicates if the address is from the same network */
bool IPAddress::isNetwork(IPAddress *toCmp) const
{
	switch (getIPClass()) {
	case 'A':
		if (addr[0] == toCmp->addr[0])
			return true;
		break;
	case 'B':
		if ((addr[0] == toCmp->addr[0]) &&
		    (addr[1] == toCmp->addr[1]))
			return true;
		break;
	case 'C':
		if ((addr[0] == toCmp->addr[0]) &&
		    (addr[1] == toCmp->addr[1]) &&
		    (addr[2] == toCmp->addr[2]))
			return true;
		break;
	default:
		// Class D or E
		return false;
	}
	// not equal
	return false;
}


/*
 * Compares both addresses, but ONLY the first nbbits BITS
 * Typical usage for comparing IP prefixes.
 */
bool IPAddress::compareTo(IPAddress *to_cmp, unsigned int nbbits) const
{
	if (nbbits < 1)
		return true;

	int addr1 = getInt();
	int addr2 = to_cmp->getInt();

	if (nbbits > 31)
		return (addr1 == addr2);

	// The right shift on an unsigned int produces 
	// 0 on the left
	unsigned int mask = 0xFFFFFFFF;
	mask = ~(mask >> nbbits);
	
	return ((addr1 & mask) == (addr2 & mask));
}


/*
 * Indicates how many bits from the to_cmp address, starting counting
 * from the left, matches the address.
 * E.g. if the address is 130.206.72.237, and to_cmp 130.206.72.0,
 * 24 will be returned.
 *
 * Typical usage for comparing IP prefixes.
 */
int IPAddress::nbBitsMatching(IPAddress *to_cmp) const
{
	int addr1 = getInt();
	int addr2 = to_cmp->getInt();

	int res = addr1 ^ addr2;
	// If the bits are equal, there is a 0, so counting
	// the zeros from the left
	int i;
	for (i = 31; i >= 0; i--) {
		if (res & (1 << i)) {
			// 1, means not equal, so stop
			return 31 - i;
		}
	}
	return 32;
}

/*
 * Only keeps the n first bits of the address, completing it
 * with zeros.
 * Typical usage is when the length of an IP prefix is done
 * and to check the address ends with the right number of 0.
 */
void IPAddress::keepFirstBits (unsigned int n)
{
	if (n > 31) return;

	int len_bytes = n / 8;

	unsigned int mask = 0xFF;
	mask = ~(mask >> ((n - (len_bytes * 8))));

	addr[len_bytes] = addr[len_bytes] & mask;

	for (int i = len_bytes + 1; i < 4; i++) {
		addr[i] = 0;
	}
}


/*
 * Test if the masked addresses (ie the mask is applied to addr1 and
 * addr2) are equal.
 * Warning: netmask == NULL is treated as all 1, ie (*addr1 == *addr2) is returned.
 *
 * NB: Used in routing. Static function.
 */
bool IPAddress::maskedAddrAreEqual (const IPAddress *addr1,
									const IPAddress *addr2, 
									const IPAddress *netmask)
{
	if (netmask == NULL) {
		return (addr1->isEqualTo(*addr2));
	}

	if ( addr1->doAnd(*netmask).isEqualTo(addr2->doAnd(*netmask)))
          {
            return true;
          }

	return false;
}
