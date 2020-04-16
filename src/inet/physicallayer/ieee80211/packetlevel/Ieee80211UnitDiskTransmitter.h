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

#ifndef __INET_IEEE80211UNITDISKTRANSMITTER_H
#define __INET_IEEE80211UNITDISKTRANSMITTER_H

#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211TransmitterBase.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211UnitDiskTransmitter : public Ieee80211TransmitterBase
{
  protected:
    m communicationRange = m(NaN);
    m interferenceRange = m(NaN);
    m detectionRange = m(NaN);

  protected:
    virtual void initialize(int stage) override;

  public:
    Ieee80211UnitDiskTransmitter();
    virtual m getMaxCommunicationRange() const override { return communicationRange; }
    virtual m getMaxInterferenceRange() const override { return interferenceRange; }
    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual const ITransmission *createTransmission(const IRadio *radio, const Packet *packet, simtime_t startTime) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE80211UNITDISKTRANSMITTER_H

