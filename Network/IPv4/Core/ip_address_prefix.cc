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
 *  File: ip_address_prefix.cc
 *	Purpose: Represention of an IPv4 address prefix
 *	Author: Vincent Oberle
 *	Date: March 2001
 */

#include "ip_address_prefix.h"

// IPAddressPrefix functions

Register_Class( IPAddressPrefix );

// Each constructors calls keepFirstBits, so there isn't to care anymore if
// there are 0 at the end and most of the functions of IPAddress work right.

IPAddressPrefix::IPAddressPrefix(int ip, unsigned int lp, const char *n) : IPAddress(ip, n)
{
	length = lp;
	keepFirstBits(length);
}

IPAddressPrefix::IPAddressPrefix(int i0, int i1, int i2, int i3, unsigned int lp,
								 const char *n) : IPAddress(i0, i1, i2, i3, n)
{
	length = lp;
	keepFirstBits(length);	
}

IPAddressPrefix::IPAddressPrefix(const char *text, unsigned int lp, const char *n) : IPAddress(text, n)
{
	length = lp;
	keepFirstBits(length);	
}

IPAddressPrefix::IPAddressPrefix(IPAddress *ip, unsigned int lp, const char *n) : 
	IPAddress(ip->getDByte(0), ip->getDByte(1), ip->getDByte(2), ip->getDByte(3), n)
{
	length = lp;
	keepFirstBits(length);	
}


IPAddressPrefix::IPAddressPrefix(const IPAddressPrefix& obj) : IPAddress()
{
	setName(obj.name());
	operator=(obj);
}


IPAddressPrefix& IPAddressPrefix::operator=(const IPAddressPrefix& obj)
{
	IPAddress::operator=(obj);
	length = obj.length;
	return *this;
}

