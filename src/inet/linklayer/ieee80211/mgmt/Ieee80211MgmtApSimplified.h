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

#ifndef __INET_IEEE80211MGMTAPSIMPLIFIED_H
#define __INET_IEEE80211MGMTAPSIMPLIFIED_H

#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtApBase.h"

namespace inet {

namespace ieee80211 {

/**
 * Used in 802.11 infrastructure mode: handles management frames for
 * an access point (AP). See corresponding NED file for a detailed description.
 * This implementation ignores many details.
 *
 * @author Andras Varga
 */
class INET_API Ieee80211MgmtApSimplified : public Ieee80211MgmtApBase
{
  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleTimer(cMessage *msg) override;

    /** Implements abstract Ieee80211MgmtBase method -- throws an error (no commands supported) */
    virtual void handleCommand(int msgkind, cObject *ctrl) override;

    /** @name Processing of different frame types */
    //@{
    virtual void handleAuthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleDeauthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleAssociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleAssociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleReassociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleReassociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleDisassociationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleBeaconFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleProbeRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleProbeResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    //@}
};

} // namespace ieee80211

} // namespace inet

#endif // ifndef __INET_IEEE80211MGMTAPSIMPLIFIED_H

