//
// Copyright (C) 2013 OpenSim Ltd.
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

#ifndef __INET_IFORWARDERRORCORRECTION_H
#define __INET_IFORWARDERRORCORRECTION_H

#include "IPrintableObject.h"
#include "BitVector.h"

namespace inet {

namespace physicallayer {

class INET_API IForwardErrorCorrection : public IPrintableObject
{
    public:
        virtual ~IForwardErrorCorrection() {};
};

class INET_API IFECCoder
{
    public:
        virtual BitVector encode(const BitVector& informationBits) const = 0;
        virtual BitVector decode(const BitVector& encodedBits) const = 0;
        virtual const IForwardErrorCorrection *getForwardErrorCorrection() const = 0;
        virtual ~IFECCoder() {};
};

class INET_API FecCoderBase : public IFECCoder
{
    protected:
        const IForwardErrorCorrection *forwardErrorCorrection;

    public:
        FecCoderBase(const IForwardErrorCorrection *convolutionalCode) : forwardErrorCorrection(convolutionalCode) {}
        virtual BitVector encode(const BitVector& informationBits) const = 0;
        virtual BitVector decode(const BitVector& encodedBits) const = 0;
        virtual const IForwardErrorCorrection *getForwardErrorCorrection() { return forwardErrorCorrection; }
        virtual ~FecCoderBase() { delete forwardErrorCorrection; }
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IFORWARDERRORCORRECTION_H
