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

#ifndef __INET_IEEE80211LAYEREDTRANSMISSION_H
#define __INET_IEEE80211LAYEREDTRANSMISSION_H

#include "inet/physicallayer/common/bitlevel/LayeredTransmission.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211TransmissionBase.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211LayeredTransmission : public LayeredTransmission, public Ieee80211TransmissionBase
{
  public:
    Ieee80211LayeredTransmission(const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel, const ITransmissionSymbolModel *symbolModel, const ITransmissionSampleModel *sampleModel, const ITransmissionAnalogModel *analogModel, const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const EulerAngles startOrientation, const EulerAngles endOrientation, const IIeee80211Mode *mode, const Ieee80211Channel *channel);

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE80211LAYEREDTRANSMISSION_H

