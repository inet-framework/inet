/*
 * Copyright (C) 2003 CTIE, Monash University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef MACADDRESS_H_
#define MACADDRESS_H_

#include <omnetpp.h>
#include "MACAddress_m.h"


#define MAC_ADDRESS_BYTES    6


/**
 * Stores an IEEE 802 MAC address (6 octets = 48 bits).
 */
class MACAddress : public MACAddress_Base
{
  private:
    unsigned char address[6];

  public:
    /**
     * Default constructor initializes address bytes to zero.
     */
    MACAddress();
    /**
     * Constructor which accepts hex string or the string "auto".
     */
    MACAddress(const char *hexstr);
    /**
     * Copy constructor.
     */
    MACAddress(const MACAddress& other) : MACAddress_Base() {operator=(other);}
    /**
     * Assignment.
     */
    MACAddress& operator=(const MACAddress& other);
    /**
     * Returns 6.
     */
    virtual unsigned int getAddressArraySize() const;
    /**
     * Returns the kth byte of the address.
     */
    virtual unsigned char getAddress(unsigned int k) const;
    /**
     * Sets the kth byte of the address.
     */
    virtual void setAddress(unsigned int k, unsigned char addrbyte);
    /**
     * Converts address value from hex string. The string "auto" is also
     * accepted, it'll generate a unique address starting with "A0 00".
     */
    void setAddress(const char *hexstr);
    /**
     * Returns pointer to internal binary representation of address (array of 6 unsigned chars).
     */
    unsigned char *getAddressBytes() {return address;}
    /**
     * Sets address bytes. The argument should point to an array of 6 unsigned chars.
     */
    void setAddressBytes(unsigned char *addrbytes);
    /**
     * Sets the address to the broadcast address (hex ff:ff:ff:ff:ff:ff).
     */
    void setBroadcast();
    /**
     * Returns true this is the broadcast address (hex ff:ff:ff:ff:ff:ff).
     */
    bool isBroadcast() const;
    /**
     * Returns true this is a multicast logical address (starts with bit 1).
     */
    bool isMulticast() const  {return address[0]&0x80;};
    /**
     * Returns true if all address bytes are zero.
     */
    bool isEmpty() const;
    /**
     * Converts address to hext string and places result into passed buffer.
     */
    const char *toHexString(char *buf) const;
    /**
     * Returns true if 2 addresses are equal.
     */
    bool equals(const MACAddress& other) const;
    /**
     * Returns -1, 0 or 1 as result of comparison of 2 addresses.
     */
    int compareTo(const MACAddress& other) const;
};

#endif
