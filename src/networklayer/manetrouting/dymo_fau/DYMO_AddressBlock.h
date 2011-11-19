/*
 *  Copyright (C) 2005 Mohamed Louizi
 *  Copyright (C) 2006,2007 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef DYMO_ADDRESSBLOCK_H_45948759
#define DYMO_ADDRESSBLOCK_H_45948759

#include <stdexcept>

class DYMO_AddressBlock
{
  protected:
    uint32_t address; /**< The IPv4 address of an additional node that can be reached via the DYMO router adding this information.  Each AdditionalNode.Address must have an associated Node.SeqNum in the address TLV block. */
    uint16_t seqNum; /**< The DYMO sequence number associated with this routing information. */
    uint8_t prefix; /**< The Node.Address is a network address with a particular prefix length. */
    uint8_t dist; /**< A metric of the distance to reach the associated Node.Address. This field is incremented by at least one at each intermediate DYMO router, except the TargetNode.AddTLV.Dist.  The TargetNode's distance information is not modified. */
    bool hasAddress_var; /**< true if the @c address member was assigned a value */
    bool hasSeqNum_var; /**< true if the @c seqNum member was assigned a value */
    bool hasPrefix_var; /**< true if the @c prefix member was assigned a value */
    bool hasDist_var; /**< true if the @c dist member was assigned a value */

  public:
    DYMO_AddressBlock() : hasAddress_var(false), hasSeqNum_var(false), hasPrefix_var(false), hasDist_var(false)
    {
    }

    DYMO_AddressBlock(const DYMO_AddressBlock& other)
    {
        copy(other);
    }

    DYMO_AddressBlock& operator=(const DYMO_AddressBlock& other)
    {
        if (this==&other) return *this;
        copy(other);
        return *this;
    }

  private:
    void copy(const DYMO_AddressBlock& other)
    {
        this->address = other.address;
        this->seqNum = other.seqNum;
        this->prefix = other.prefix;
        this->dist = other.dist;
        this->hasAddress_var = other.hasAddress_var;
        this->hasSeqNum_var = other.hasSeqNum_var;
        this->hasPrefix_var = other.hasPrefix_var;
        this->hasDist_var = other.hasDist_var;
    }

  public:
    bool hasAddress() const
    {
        return hasAddress_var;
    }

    uint32_t getAddress() const
    {
        if (!hasAddress()) throw std::runtime_error("Tried to read Address from AddressBlock, but found none");
        return address;
    }

    void setAddress(uint32_t address)
    {
        this->hasAddress_var = true;
        this->address = address;
    }

    bool hasSeqNum() const
    {
        return hasSeqNum_var;
    }

    uint16_t getSeqNum() const
    {
        if (!hasSeqNum()) throw std::runtime_error("Tried to read SeqNum from AddressBlock, but found none");
        return seqNum;
    }

    void setSeqNum(uint16_t seqNum)
    {
        this->hasSeqNum_var = true;
        this->seqNum = seqNum;
    }

    bool hasPrefix() const
    {
        return hasPrefix_var;
    }

    uint8_t getPrefix() const
    {
        if (!hasPrefix()) throw std::runtime_error("Tried to read Prefix from AddressBlock, but found none");
        return prefix;
    }

    void setPrefix(uint8_t prefix)
    {
        this->hasPrefix_var = true;
        this->prefix = prefix;
    }

    bool hasDist() const
    {
        return hasDist_var;
    }

    uint8_t getDist() const
    {
        if (!hasDist()) throw std::runtime_error("Tried to read Dist from AddressBlock, but found none");
        return dist;
    }

    void setDist(uint8_t dist)
    {
        this->hasDist_var = true;
        this->dist = dist;
    }

    void incrementDistIfAvailable()
    {
        if (!this->hasDist()) return;
        if (this->getDist() == 0xFF) throw std::runtime_error("Tried to increment Dist from AddressBlock, but uint8_t would overflow");
        this->setDist(this->getDist() + 1);
    }

};

#endif

