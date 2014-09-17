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

#include "inet/common/ShortBitVector.h"
#include <algorithm>

namespace inet {

const ShortBitVector ShortBitVector::UNDEF = ShortBitVector();

ShortBitVector::ShortBitVector()
{
    undef = true;
    size = 0;
    bitVector = 0;
}

std::ostream& operator<<(std::ostream& out, const ShortBitVector& bitVector)
{
#ifndef NDEBUG
    if (bitVector.isUndef())
        out << "**UNDEFINED BITVECTOR**";
    else
    {
#endif
        if (bitVector.getBit(0))
            out << "1";
        else
            out << "0";
        for (unsigned int i = 1; i < bitVector.getSize(); i++)
        {
            if (bitVector.getBit(i))
                out << " 1";
            else
                out << " 0";
        }
#ifndef NDEBUG
    }
#endif
    return out;
}

std::string ShortBitVector::toString() const
{
#ifndef NDEBUG
    if (undef)
        throw cRuntimeError("You can't convert an undefined ShortBitVector to a string");
#endif
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

ShortBitVector::ShortBitVector(const char* str)
{
    undef = false;
    size = 0;
    bitVector = 0;
    stringToBitVector(str);
}

ShortBitVector::ShortBitVector(unsigned int num)
{
    undef = false;
    if (num < 0)
        throw cRuntimeError("num = %d must be a positive integer", num);
    if (num == 0)
        size = 1;
    else
        size = (int)(log(num) / log(2)) + 1;
    bitVector = num;
}

ShortBitVector::ShortBitVector(unsigned int num, unsigned int fixedSize)
{
    undef = false;
    if (fixedSize > MAX_LENGTH)
        throw cRuntimeError("fixedSize = %d must be less then 32", fixedSize);
    size = fixedSize;
    if (num < 0)
        throw cRuntimeError("num = %d must be a positive integer", num);
    bitVector = num;
}

} /* namespace inet */

