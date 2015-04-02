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
    virtual double getSuccessRate(const IIeee80211Mode *mode, unsigned int headerBitLength, unsigned int payloadBitLength, double snr) const = 0;

  public:
    Ieee80211ErrorModelBase();

    virtual double computePacketErrorRate(const ISNIR *snir) const override;
    virtual double computeBitErrorRate(const ISNIR *snir) const override;
    virtual double computeSymbolErrorRate(const ISNIR *snir) const override;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE80211ERRORMODELBASE_H

