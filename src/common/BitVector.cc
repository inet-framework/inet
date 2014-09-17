//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/BitVector.h"
#include <algorithm>

namespace inet {

const BitVector BitVector::UNDEF = BitVector();

BitVector::BitVector()
{
    undef = true;
    size = 0;
    fields.push_back(uint8(0));
}

void BitVector::setBit(int pos, bool value)
{
    undef = false;
    while (containerSize() <= pos)
        fields.push_back(uint8(0));
    if (pos + 1 > size)
        size = pos + 1;
    uint8& field = fields[pos / UINT8_LENGTH];
    if (value)
        field |= 1 << (pos % UINT8_LENGTH);
    else
        field &= ~(1 << (pos % UINT8_LENGTH));
}

void BitVector::toggleBit(int pos)
{
    if (undef)
        throw cRuntimeError("You can't toggle bits in an undefined BitVector");
    if (pos >= size)
        throw cRuntimeError("Out of range with bit position %d", pos);
    uint8& field = fields[pos / UINT8_LENGTH];
    field ^= 1 << (pos % UINT8_LENGTH);
}

bool BitVector::getBit(int pos) const
{
    if (undef)
        throw cRuntimeError("You can't get bits from an undefined BitVector");
    if (pos >= size)
        throw cRuntimeError("Out of range with bit position %d", pos);
    uint8 field = fields[pos / UINT8_LENGTH];
    return field & (1 << (pos % UINT8_LENGTH));
}

std::ostream& operator<<(std::ostream& out, const BitVector& bitVector)
{
    if (bitVector.isUndef())
        out << "**UNDEFINED BITVECTOR**";
    else
    {
        if (bitVector.getBit(0))
            out << "1";
        else
            out << "0";
        for (int i = 1; i < bitVector.size; i++)
        {
            if (bitVector.getBit(i))
                out << " 1";
            else
                out << " 0";
        }
    }
    return out;
}

void BitVector::appendBit(bool value)
{
    setBit(size, value);
}

void BitVector::appendBit(bool value, int n)
{
    while (n--)
        appendBit(value);
}

bool BitVector::getBitAllowOutOfRange(int pos) const
{
    if (undef)
        throw cRuntimeError("You can't get bits from an undefined BitVector");
    if (pos >= size)
        return false;
    return getBit(pos);
}

std::string BitVector::toString() const
{
    if (undef)
        throw cRuntimeError("You can't convert an undefined BitVector to a string");
    std::string str;
    for (unsigned int i = 0; i < getSize(); i++)
    {
        if (getBit(i))
            str += "1";
        else
            str += "0";
    }
    return str;
}

BitVector::BitVector(const char* str)
{
    undef = false;
    size = 0;
    stringToBitVector(str);
}

void BitVector::stringToBitVector(const char *str)
{
    int strSize = strlen(str);
    for (int i = 0; i < strSize; i++)
    {
        if (str[i] == '1')
            appendBit(true);
        else if (str[i] == '0')
            appendBit(false);
        else
            throw cRuntimeError("str must represent a binary number");
    }
}

unsigned int BitVector::toDecimal() const
{
    if (undef)
        throw cRuntimeError("You can't compute the decimal value of an undefined BitVector");
    unsigned int dec = 0;
    unsigned int powerOfTwo = 1;
    for (unsigned int i = 0; i < getSize(); i++)
    {
        if (getBit(i))
            dec += powerOfTwo;
        powerOfTwo *= 2;
    }
    return dec;
}
unsigned int BitVector::reverseToDecimal() const
{
    if (undef)
        throw cRuntimeError("You can't compute the decimal value of an undefined BitVector");
    unsigned int dec = 0;
    unsigned int powerOfTwo = 1;
    for (int i = getSize() - 1; i >= 0; i--)
    {
        if (getBit(i))
            dec += powerOfTwo;
        powerOfTwo *= 2;
    }
    return dec;
}

int BitVector::computeHammingDistance(const BitVector& u) const
{
    if (u.isUndef() || isUndef())
        throw cRuntimeError("You can't compute the Hamming distance between undefined BitVectors");
    if (getSize() != u.getSize())
        throw cRuntimeError("You can't compute Hamming distance between two vectors with different sizes");
    int hammingDistance = 0;
    for (unsigned int i = 0; i < getSize(); i++)
        if (u.getBit(i) != getBit(i))
            hammingDistance++;
    return hammingDistance;
}

BitVector::BitVector(unsigned int num)
{
    undef = false;
    size = 0;
    if (num == 0)
        appendBit(false);
    else if (num < 0)
        throw cRuntimeError("num = %d must be a positive integer", num);
    while (num > 0)
    {
        appendBit(num % 2);
        num /= 2;
    }
}

BitVector& BitVector::operator=(const BitVector& rhs)
{
    if (this == &rhs)
        return *this;
    copy(rhs);
    return *this;
}

bool BitVector::operator==(const BitVector& rhs) const
{
    if (rhs.isUndef() && isUndef())
        return true;
    if (rhs.isUndef() || isUndef())
        throw cRuntimeError("You can't compare undefined BitVectors");
    if (rhs.getSize() != getSize())
        return false;
    for (unsigned int i = 0; i < getSize(); i++)
        if (getBit(i) != rhs.getBit(i))
            return false;
    return true;
}

bool BitVector::operator!=(const BitVector& rhs) const
{
    return !(rhs == *this);
}

BitVector::BitVector(unsigned int num, unsigned int fixedSize)
{
    undef = false;
    size = 0;
    if (num == 0)
        appendBit(false);
    else if (num < 0)
        throw cRuntimeError("num = %d must be a positive integer", num);
    while (num > 0)
    {
        appendBit(num % 2);
        num /= 2;
    }
    if (getSize() < fixedSize)
        appendBit(false, fixedSize - getSize());
}

void BitVector::copy(const BitVector& other)
{
    undef = other.undef;
    size = 0;
    fields.clear();
    for (unsigned int i = 0; i < other.getSize(); i++)
        appendBit(other.getBit(i));
}

} /* namespace inet */

