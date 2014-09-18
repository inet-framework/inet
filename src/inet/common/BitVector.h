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

#ifndef __INET_BITVECTOR_H_
#define __INET_BITVECTOR_H_

#include "inet/common/INETDefs.h"

namespace inet {

#define UINT8_LENGTH 8

class BitVector
{
    public:
        static const BitVector UNDEF;

    protected:
        std::vector<uint8> fields;
        bool undef;
        int size;
        int containerSize() const { return fields.size() * 8; }
        void stringToBitVector(const char *str);
        void copy(const BitVector& other);

    public:
        unsigned int toDecimal() const;
        unsigned int reverseToDecimal() const;
        void appendBit(bool value);
        void appendBit(bool value, int n);
        void setBit(int pos, bool value);
        void toggleBit(int pos);
        bool getBit(int pos) const;
        bool isUndef() const { return undef; }
        bool getBitAllowOutOfRange(int pos) const;
        unsigned int getSize() const { return size; }
        int computeHammingDistance(const BitVector& u) const;
        friend std::ostream& operator<<(std::ostream& out, const BitVector& bitVector);
        BitVector& operator=(const BitVector& rhs);
        bool operator==(const BitVector& rhs) const;
        bool operator!=(const BitVector& rhs) const;
        std::string toString() const;
        BitVector();
        BitVector(const char *str);
        BitVector(unsigned int num);
        BitVector(unsigned int num, unsigned int fixedSize);
        BitVector(const BitVector& other) { copy(other); }
};

} /* namespace inet */

#endif /* __INET_BITVECTOR_H_ */
