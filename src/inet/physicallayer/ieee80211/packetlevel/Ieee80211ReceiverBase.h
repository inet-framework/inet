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

#ifndef __INET_IEEE80211RECEIVERBASE_H
#define __INET_IEEE80211RECEIVERBASE_H

#include "inet/physicallayer/base/packetlevel/FlatReceiverBase.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211Channel.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211Band.h"

namespace inet {

namespace physicallayer {

class INET_API Ieee80211ReceiverBase : public FlatReceiverBase
{
  protected:
    const Ieee80211ModeSet *modeSet;
    const IIeee80211Band *band;
    const Ieee80211Channel *channel;

  protected:
    virtual void initialize(int stage) override;
    virtual ReceptionIndication *createReceptionIndication() const override;
    virtual const ReceptionIndication *computeReceptionIndication(const ISNIR *snir) const override;

  public:
    Ieee80211ReceiverBase();
    virtual ~Ieee80211ReceiverBase();

    virtual std::ostream& printToStream(std::ostream& stream, int level) const override;

    virtual void setModeSet(const Ieee80211ModeSet *modeSet);
    virtual void setBand(const IIeee80211Band *band);
    virtual void setChannel(const Ieee80211Channel *channel);
    virtual void setChannelNumber(int channelNumber);
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IEEE80211RECEIVERBASE_H

