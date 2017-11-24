//
// Copyright (C) 2006 Andras Varga
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

#ifndef __INET_IEEE80211MGMTAPBASE_H
#define __INET_IEEE80211MGMTAPBASE_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h"

namespace inet {

class EtherFrame;

namespace ieee80211 {

/**
 * Used in 802.11 infrastructure mode: abstract base class for management frame
 * handling for access points (APs). This class extends Ieee80211MgmtBase
 * with utility functions that are useful for implementing AP functionality.
 *
 * @author Andras Varga
 */
class INET_API Ieee80211MgmtApBase : public Ieee80211MgmtBase
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int) override;
};

} // namespace ieee80211

} // namespace inet

#endif // ifndef __INET_IEEE80211MGMTAPBASE_H

