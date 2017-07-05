//
// Copyright (C) OpenSim Ltd.
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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#ifndef __INET_DS_H
#define __INET_DS_H

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ieee80211/mac/contract/IDs.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
#include "inet/linklayer/ieee80211/mib/Ieee80211Mib.h"

namespace inet {
namespace ieee80211 {

/**
 * The default implementation of IDs.
 */
class INET_API Ds : public cSimpleModule, public IDs
{
  protected:
    Ieee80211Mib *mib = nullptr;
    Ieee80211Mac *mac = nullptr;

  protected:
    virtual void initialize(int stage) override;

    /**
     * Utility function for APs: sends back a data frame we received from a
     * STA to the wireless LAN, after tweaking fromDS/toDS bits and shuffling
     * addresses as needed.
     */
    virtual void distributeDataFrame(Packet *frame, const Ptr<const Ieee80211DataOrMgmtHeader>& header);

  public:
    virtual void processDataFrame(Packet *frame, const Ptr<const Ieee80211DataHeader>& header) override;
};

} // namespace ieee80211
} // namespace inet

#endif
