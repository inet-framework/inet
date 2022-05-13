//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/ShortBitVector.h"

#include <algorithm>

namespace inet {

ShortBitVector::ShortBitVector()
{
    size = 0;
    bits = 0;
}

ShortBitVector::ShortBitVector(const char *bitString)
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
        throw cRuntimeError("size = %u must be less than %zu", size, (size_t)(sizeof(unsigned int) * 8));
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

