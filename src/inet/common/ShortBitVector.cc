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

#include <algorithm>

#include "inet/common/ShortBitVector.h"

namespace inet {

ShortBitVector::ShortBitVector()
{
    size = 0;
    bits = 0;
}

ShortBitVector::ShortBitVector(const char* bitString)
{
    size = 0;
    bits = 0;
    int strSize = strlen(bitString);
    for (int i = 0; i < strSize; i++) {
        if (bitString[i] == '1')
            appendBit(true);
        else if (bitString[i] == '0')
            appendBit(false);
        else
            throw cRuntimeError("str must represent a binary number");
    }
}

ShortBitVector::ShortBitVector(unsigned int num)
{
    if (num == 0)
        size = 1;
    else
        size = (int)(log(num) / log(2)) + 1;
    bits = num;
}

ShortBitVector::ShortBitVector(unsigned int bits, unsigned int size)
{
    if (size > sizeof(unsigned int) * 8)
        throw cRuntimeError("size = %u must be less than %lu", size, sizeof(unsigned int) * 8);
    this->size = size;
    this->bits = bits;
}

std::ostream& operator<<(std::ostream& out, const ShortBitVector& bitVector)
{
    for (unsigned int i = 0; i < bitVector.getSize(); i++) {
        if (i != 0)
            out << " ";
        if (bitVector.getBit(i))
            out << "1";
        else
            out << "0";
    }
    return out;
}

std::string ShortBitVector::toString() const
{
    std::string str;
    for (unsigned int i = 0; i < getSize(); i++) {
        if (getBit(i))
            str += "1";
        else
            str += "0";
    }
    return str;
}

} // namespace inet

