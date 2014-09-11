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

#ifndef __INET_IMODULATIONSCHEME_H
#define __INET_IMODULATIONSCHEME_H

#include "IPrintableObject.h"
#include "ShortBitVector.h"
#include "Complex.h"

namespace inet {

namespace physicallayer {

class INET_API IModulationScheme : public IPrintableObject
{
    public:
        virtual int getCodeWordLength() const = 0;
        virtual int getConstellationSize() const = 0;
        virtual const Complex& map(const ShortBitVector& input) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif /* __INET_IMODULATIONSCHEME_H */

