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

BitVector::BitVector()
{
    size = 0;
    bytes.push_back(uint8(0));
}

BitVector::BitVector(const char* bits)
{
    size = 0;
    stringToBitVector(bits);
}

BitVector::BitVector(unsigned int bits)
{
    size = 0;
    if (bits == 0)
        appendBit(false);
    while (bits > 0)
    {
        appendBit(bits % 2);
        bits /= 2;
    }
}

BitVector::BitVector(unsigned int bits, unsigned int size)
{
    this->size = 0;
    if (bits == 0)
        appendBit(false);
    while (bits > 0)
    {
        appendBit(bits % 2);
        bits /= 2;
    }
    if (getSize() < size)
        appendBit(false, size - getSize());
}

void BitVector::setBit(int pos, bool value)
{
    while (containerSize() <= pos)
        bytes.push_back(uint8(0));
    if (pos + 1 > size)
        size = pos + 1;
    uint8& field = bytes[pos / UINT8_LENGTH];
    if (value)
        field |= 1 << (pos % UINT8_LENGTH);
    else
        field &= ~(1 << (pos % UINT8_LENGTH));
}

void BitVector::toggleBit(int pos)
{
    if (pos >= size)
        throw cRuntimeError("Out of range with bit position %d", pos);
    uint8& field = bytes[pos / UINT8_LENGTH];
    field ^= 1 << (pos % UINT8_LENGTH);
}

bool BitVector::getBit(int pos) const
{
    if (pos >= size)
        throw cRuntimeError("Out of range with bit position %d", pos);
    uint8 field = bytes[pos / UINT8_LENGTH];
    return field & (1 << (pos % UINT8_LENGTH));
}

std::ostream& operator<<(std::ostream& out, const BitVector& bitVector)
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

void BitVector::appendByte(uint8_t value)
{
    for (unsigned int i = 0; i < 8; i++)
        appendBit(value & (1 << i));
}

std::string BitVector::toString() const
{
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
    if (getSize() != u.getSize())
        throw cRuntimeError("You can't compute Hamming distance between two vectors with different sizes");
    int hammingDistance = 0;
    for (unsigned int i = 0; i < getSize(); i++)
        if (u.getBit(i) != getBit(i))
            hammingDistance++;
    return hammingDistance;
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

void BitVector::copy(const BitVector& other)
{
    size = 0;
    bytes.clear();
    for (unsigned int i = 0; i < other.getSize(); i++)
        appendBit(other.getBit(i));
}

} /* namespace inet */

