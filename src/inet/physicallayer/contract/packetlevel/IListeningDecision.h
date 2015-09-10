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

#ifndef __INET_ILISTENINGDECISION_H
#define __INET_ILISTENINGDECISION_H

#include "inet/physicallayer/contract/packetlevel/IPrintableObject.h"
#include "inet/physicallayer/contract/packetlevel/IListening.h"

namespace inet {

namespace physicallayer {

/**
 * This interface represents the result of a receiver's listening process.
 *
 * This interface is strictly immutable to safely support parallel computation.
 */
class INET_API IListeningDecision : public IPrintableObject
{
  public:
    virtual const IListening *getListening() const = 0;

    virtual bool isListeningPossible() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_ILISTENINGDECISION_H

