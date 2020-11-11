//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_IWIRELESSSIGNAL_H
#define __INET_IWIRELESSSIGNAL_H

#include "inet/physicallayer/wireless/common/contract/packetlevel/IArrival.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IListening.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IPhysicalLayerFrame.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IReception.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/ITransmission.h"

namespace inet {

namespace physicallayer {

/**
 * This purely virtual interface provides an abstraction for different signals.
 */
class INET_API IWirelessSignal : public IPhysicalLayerFrame, public IPrintableObject
{
  public:
    /**
     * Returns the radio signal transmission that this signal represents.
     * This function never returns nullptr.
     */
    virtual const ITransmission *getTransmission() const = 0;

    /**
     * This function may return nullptr if this is not yet computed.
     */
    virtual const IArrival *getArrival() const = 0;

    /**
     * This function may return nullptr if this is not yet computed.
     */
    virtual const IListening *getListening() const = 0;

    /**
     * This function may return nullptr if this is not yet computed.
     */
    virtual const IReception *getReception() const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif

