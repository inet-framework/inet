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
 *  File: ip_address_prefix.h
 *    Purpose: Represention of an IPv4 address prefix
 *    Author: Vincent Oberle
 *    Date: March 2001
 */

#ifndef __IP_ADDRESS_PREFIX_H
#define __IP_ADDRESS_PREFIX_H

#include <omnetpp.h>
#include "IPAddress.h"


/* Default value for the length, ie all bits */
const unsigned int DEFAULT_IP_ADDRESS_PREFIX_LEN  = 32;


//
// FIXME this class IPAddressPrefix never seems to be used in IPSuite.
//
// it IS used a few places in LDP and LIBTable, its extra functionality over IPAddress
// seems to be unused!!!!  It is always created with constant length
// (ConstType::prefixLength), and getLength() is never called.
//
// can this file be removed?
//

/**
 * An IPv4 address prefix (ie an IP address and a prefix len).
 */
class IPAddressPrefix : public IPAddress
{
  protected:
    // Prefix length in BITS
    unsigned int length;

  public:
    IPAddressPrefix() { length = DEFAULT_IP_ADDRESS_PREFIX_LEN; }

    /**
     * IP address as int
     */
    IPAddressPrefix(int i, unsigned int lp = DEFAULT_IP_ADDRESS_PREFIX_LEN);

    /**
     * "i0.i1.i2.i3" format
     */
    IPAddressPrefix(int i0, int i1, int i2, int i3,
                    unsigned int lp = DEFAULT_IP_ADDRESS_PREFIX_LEN);
    /**
     * IP address as text
     */
    IPAddressPrefix(const char *t, unsigned int lp = DEFAULT_IP_ADDRESS_PREFIX_LEN);
    IPAddressPrefix(const IPAddress& ip, unsigned int lp = DEFAULT_IP_ADDRESS_PREFIX_LEN);

    IPAddressPrefix(const IPAddressPrefix& obj);
    virtual ~IPAddressPrefix() {}
    IPAddressPrefix& operator=(const IPAddressPrefix& obj);

    unsigned int getLength () const { return length; }
};

#endif // __IP_ADDRESS_PREFIX_H
