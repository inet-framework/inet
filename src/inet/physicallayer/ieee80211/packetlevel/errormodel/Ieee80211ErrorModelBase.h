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

#ifndef __INET_IEEE80211ERRORMODELBASE_H
#define __INET_IEEE80211ERRORMODELBASE_H

#include "inet/physicallayer/base/packetlevel/ErrorModelBase.h"
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211ErrorModelBase : public ErrorModelBase
{
  protected:
    virtual double getHeaderSuccessRate(const IIeee80211Mode *mode, unsigned int bitLength, double snr) const = 0;
    virtual double getDataSuccessRate(const IIeee80211Mode *mode, unsigned int bitLength, double snr) const = 0;

    virtual Packet *computeCorruptedPacket(const Packet *packet, double ber) const override;

  public:
    Ieee80211ErrorModelBase();

    virtual double computePacketErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
    virtual double computeBitErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
    virtual double computeSymbolErrorRate(const ISnir *snir, IRadioSignal::SignalPart part) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE80211ERRORMODELBASE_H

