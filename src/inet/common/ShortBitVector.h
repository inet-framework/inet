//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SHORTBITVECTOR_H
#define __INET_SHORTBITVECTOR_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Optimized version of the BitVector class to store short bit vectors.
 */
class INET_API ShortBitVector
{
  private:
    unsigned int bits;
    unsigned int size;

  private:
    inline void copy(const ShortBitVector& other)
    {
        size = other.size;
        bits = other.bits;
    }

  public:
    ShortBitVector();
    ShortBitVector(const char *bitString);
    ShortBitVector(unsigned int bits);
    ShortBitVector(unsigned int bits, unsigned int size);
    ShortBitVector(const ShortBitVector& other) { copy(other); }

    inline unsigned int toDecimal() const { return bits; }

    inline unsigned int reverseToDecimal() const
    {
        unsigned int decimal = 0;
        unsigned int powerOfTwo = 1;
        for (int i = getSize() - 1; i >= 0; i--) {
            if (getBit(i))
                decimal += powerOfTwo;
            powerOfTwo *= 2;
        }
        return decimal;
    }

    inline void rightShift(int with) { bits = bits >> with; }

    inline void leftShift(int with) { bits = bits << with; }

    inline void appendBit(bool value) { setBit(size, value); }

    inline void appendBit(bool value, int n) { while (n--) appendBit(value); }

    inline bool getBit(unsigned int pos) const
    {
#ifndef NDEBUG
        if (pos >= size)
            throw cRuntimeError("Out of range with bit position %d", pos);
#endif
        return bits & (1 << pos);
    }

    inline void setBit(unsigned int pos, bool value)
    {
#ifndef NDEBUG
        if (pos >= sizeof(unsigned int) * 8)
            throw cRuntimeError("Out of range with bit position: %d", pos);
#endif
        if (pos + 1 > size)
            size = pos + 1;
        if (value)
            bits |= 1 << pos;
        else
            bits &= ~(1 << pos);
    }

    inline void toggleBit(unsigned int pos)
    {
#ifndef NDEBUG
        if (pos >= size)
            throw cRuntimeError("Out of range with bit position %d", pos);
#endif
        bits ^= 1 << pos;
    }

    inline bool isEmpty() const { return size == 0; }

    inline unsigned int getSize() const { return size; }

    inline unsigned int computeHammingDistance(const ShortBitVector& u) const
    {
#ifndef NDEBUG
        if (getSize() != u.getSize())
            throw cRuntimeError("You can't compute Hamming distance between two vectors with different sizes");
#endif
        unsigned int hammingDistance = bits ^ u.toDecimal();
        hammingDistance = hammingDistance - ((hammingDistance >> 1) & 0x55555555);
        hammingDistance = (hammingDistance & 0x33333333) + ((hammingDistance >> 2) & 0x33333333);
        return (((hammingDistance + (hammingDistance >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
    }

    inline ShortBitVector& operator=(const ShortBitVector& other)
    {
        if (this == &other)
            return *this;
        copy(other);
        return *this;
    }

    inline bool operator==(const ShortBitVector& rhs) const { return rhs.bits == bits; }

    inline bool operator!=(const ShortBitVector& rhs) const { return !(rhs == *this); }

    friend std::ostream& operator<<(std::ostream& out, const ShortBitVector& bitVector);

    std::string toString() const;
};

} // namespace inet

#endif

