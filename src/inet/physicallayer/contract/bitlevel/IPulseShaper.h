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

#ifndef __INET_IPULSESHAPER_H
#define __INET_IPULSESHAPER_H

#include "inet/physicallayer/contract/bitlevel/ISignalSymbolModel.h"
#include "inet/physicallayer/contract/bitlevel/ISignalSampleModel.h"

namespace inet {

namespace physicallayer {

class INET_API IPulseShaper : public IPrintableObject
{
  public:
    virtual const ITransmissionSampleModel *shape(const ITransmissionSymbolModel *symbolModel) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IPULSESHAPER_H

