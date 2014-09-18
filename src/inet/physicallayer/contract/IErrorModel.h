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

#ifndef __INET_IERRORMODEL_H
#define __INET_IERRORMODEL_H

#include "IReception.h"

namespace inet {

namespace physicallayer {

// TODO: revise and use
// TODO: what kind of SNIR should we use: min, max, average, dimensional, polymorphism?
class INET_API IErrorModel : public IPrintableObject
{
  public:
    virtual double computePacketErrorRate(const IReception *reception, double minSNIR) const = 0;

    virtual double computeBitErrorRate(const IReception *reception, double minSNIR) const = 0;

    virtual double computeSymbolErrorRate(const IReception *reception, double minSNIR) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IERRORMODEL_H

