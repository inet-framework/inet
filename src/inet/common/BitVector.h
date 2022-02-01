//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BITVECTOR_H
#define __INET_BITVECTOR_H

#include "inet/common/INETDefs.h"

namespace inet {

#define UINT8_LENGTH    8

class INET_API BitVector
{
  private:
    std::vector<uint8_t> bytes;
    int size;

  private:
    int containerSize() const { return bytes.size() * 8; }
    void stringToBitVector(const char *str);
    void copy(const BitVector& other);

  public:
    BitVector();
    BitVector(const char *bits);
    BitVector(unsigned int bits);
    BitVector(unsigned int bits, unsigned int fixedSize);
    BitVector(const BitVector& other) { copy(other); }
    BitVector(const std::vector<uint8_t>& bytes) : bytes(bytes), size(bytes.size() * 8) {}

    unsigned int toDecimal() const;
    unsigned int reverseToDecimal() const;
    void appendBit(bool value);
    void appendBit(bool value, int n);
    void setBit(int pos, bool value);
    void toggleBit(int pos);
    bool getBit(int pos) const;
    void appendByte(uint8_t value);
    unsigned int getSize() const { return size; }
    unsigned int getNumberOfBytes() const { return bytes.size(); }
    const std::vector<uint8_t>& getBytes() const { return bytes; }
    int computeHammingDistance(const BitVector& u) const;
    friend std::ostream& operator<<(std::ostream& out, const BitVector& bitVector);
    BitVector& operator=(const BitVector& rhs);
    bool operator==(const BitVector& rhs) const;
    bool operator!=(const BitVector& rhs) const;
    std::string toString() const;
};

} // namespace inet

#endif

