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
 *  File: ip_address.h
 *	Purpose: Represention of an IPv4 address
 *	Author: Vincent Oberle
 *	Date: Jan-March 2001
 */

#ifndef __IP_ADDRESS_H
#define __IP_ADDRESS_H

#include <omnetpp.h>
#include "IPInterfacePacket.h"

/*
 * An IPv4 address class.
 * Replaces the old IPAddrChar definitions which is deprecated.
 *
 * All public functions are const: when an address is created,
 * it always stays the same, and to modify it, 
 * you have to create a new one.
 */
class IPAddress : public cObject {
  protected:
	// Coded in the form of 4 numbers, following the
	// format "addr[0].addr[1].addr[2].addr[3]"
	// Exemple for the adress 192.24.65.10,
	// addr[0]=192, addr[1]=24 etc.
	int addr[4];
    // to store the return value for getString()
    mutable char addrString[ADDRESS_STRING_SIZE];

  protected:
	// Only keeps the n first bits of the address, completing it
	// with zeros.
	virtual void keepFirstBits (unsigned int n);
	
  public:
	IPAddress() { addr[0] = addr[1] = addr[2] = addr[3] = 0; }

	IPAddress(int i, const char *n = NULL);
	IPAddress(int i0, int i1, int i2, int i3, const char *n = NULL);
	IPAddress(const char *t, const char *n = NULL);

	IPAddress(const IPAddress& obj);
	virtual ~IPAddress() { }
	virtual cObject *dup() const { return new IPAddress(*this); }
	virtual void info(char *buf);
	virtual void writeContents(ostream& os) const;
	virtual IPAddress& operator=(const IPAddress& obj);

	virtual bool isEqualTo(const IPAddress& toCmp) const;

	virtual IPAddress doAnd(const IPAddress& ip) const;
	
	// Returns a string (size ADDRESS_STRING_SIZE) representing the address
        // The returned string is NOT allocated dynamically
	virtual const char *getString() const;
	// Returns the address as an int
	virtual int getInt() const;
	// Returns the corresponding part of the address specified by the index "[0].[1].[2].[3]
	virtual int getDByte(int i) const { return addr[i]; }

	// Returns the network class of the address in the form of a
	// char 'A', 'B', 'C', 'D' or 'E' (case sensitive)
	virtual char getIPClass() const;
	
	virtual bool isMulticast() const { return (getIPClass() == 'D'); }
	
	// Returns a new address with the network part of the address
	// (the bits of the hosts part are to 0)
	virtual IPAddress* getNetwork() const;
	
	// Returns the network mask corresponding to the address class.
	virtual IPAddress* getNetworkMask() const;

	// Indicates if the address is from the same network
	virtual bool isNetwork(IPAddress *toCmp) const;

	// Compares both addresses, but ONLY the first nbbits BITS
	virtual bool compareTo(IPAddress *to_cmp, unsigned int nbbits) const;

	// Indicates how many bits from the to_cmp address, starting counting
	// from the left, matches the address.
	virtual int nbBitsMatching(IPAddress *to_cmp) const;


	// Test if the masked addresses (ie the mask is applied to addr1 and
	// addr2) are equal. Warning: netmask == NULL is treated as all 1,
	// ie (*addr1 == *addr2) is returned.
	static bool maskedAddrAreEqual (const IPAddress *addr1,
									const IPAddress *addr2, 
									const IPAddress *netmask);

	friend ostream& operator<<(ostream& os, const IPAddress& obj);
	friend cEnvir& operator<<(cEnvir& ev, const IPAddress& obj);
	bool operator==(const IPAddress& toCmp) const {return isEqualTo(toCmp);}
	bool operator!=(const IPAddress& toCmp) const {return !isEqualTo(toCmp);}


//	virtual int operator==(const IPAddress& toCmp) const;
//	virtual IPAddress operator&(const IPAddress& ip) const;
//	virtual operator const char *() const  { return getString();}
//	virtual operator int() const           { return getInt();}

};

inline ostream& operator<<(ostream& os, const IPAddress& obj) {
	obj.writeContents(os);
	return os;
}
inline cEnvir& operator<<(cEnvir& ev, const IPAddress& obj) {
	ev.printf("%d.%d.%d.%d", obj.addr[0], obj.addr[1], obj.addr[2], obj.addr[3]);
	return ev;
}



#endif // __IP_ADDRESS_H
