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

#ifndef __INET_IRECEPTIONRESULT_H
#define __INET_IRECEPTIONRESULT_H

#include "inet/physicallayer/contract/packetlevel/IReception.h"
#include "inet/physicallayer/contract/packetlevel/IReceptionDecision.h"
#include "inet/physicallayer/contract/packetlevel/RadioControlInfo_m.h"

namespace inet {

namespace physicallayer {

/**
 * This interface represents the result of a receiver's reception process.
 *
 * This interface is strictly immutable to safely support parallel computation.
 */
class INET_API IReceptionResult : public IPrintableObject
{
  public:
    /**
     * Returns the corresponding reception that also specifies the receiver
     * and the received transmission. This function never returns nullptr.
     */
    virtual const IReception *getReception() const = 0;

    /**
     * Returns the reception decisions made by the receiver in the order of
     * received signal parts. This function never returns an empty vector.
     */
    virtual const std::vector<const IReceptionDecision *> *getDecisions() const = 0;

    /**
     * Returns the packet corresponding to this reception. This function never
     * returns nullptr.
     */
    virtual const Packet *getPacket() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IRECEPTIONRESULT_H

