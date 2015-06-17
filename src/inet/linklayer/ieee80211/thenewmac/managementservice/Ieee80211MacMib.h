//
// Copyright (C) 2015 OpenSim Ltd.
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

#ifndef __INET_IEEE80211MACMIB_H
#define __INET_IEEE80211MACMIB_H

#include "inet/common/INETDefs.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ieee80211/thenewmac/base/Ieee80211MacMacProcessBase.h"
#include "inet/linklayer/ieee80211/thenewmac/macmib/Ieee80211MacMacmib.h"

namespace inet {
namespace ieee80211 {

class INET_API IIeee80211MacMib : public Ieee80211MacMacProcessBase
{
    protected:
        virtual void handleMlmeResetRequest(Ieee80211MacSignalMlmeResetRequest *resetRequest) = 0;
        virtual void handleMlmeGetRequest(Ieee80211MacSignalMlmeGetRequest *getRequest) = 0;
        virtual void handleMlmeSetRequest(Ieee80211MacSignalMlmeSetRequest *setRequest) = 0;

        virtual void emitResetMac() = 0;
        virtual void emitMlmeGetConfirm() = 0;
        virtual void emitMlmeSetConfirm() = 0;
        virtual void emitMlmeResetConfirm() = 0;

};

class INET_API Ieee80211MacMib : public IIeee80211MacMib
{
    protected:
        enum MacMibState
        {
            MAC_MIB_STATE_MIB_IDLE
        };
    protected:
        MacMibState state = MAC_MIB_STATE_MIB_IDLE;

        bool dflt; // dcl dflt Boolean ;
        std::string x; // dcl x MibAtrib ;
        int v; // dcl v MibValue ;
        MACAddress adr;

        /* Declarations of MIB attributes exported from
        this process */

        /* Read-Write attributes */
//        dot11AuthenticationAlgorithms AuthTypeSet:=incl(open_system, shared_key),
        bool dot11ExcludeUnencrypted;
        int dot11FragmentationThreshold;
        std::vector<MACAddress> dot11GroupAddresses;
        int dot11LongRetryLimit;
        simtime_t dot11MaxReceiveLifetime;
        int dot11MaxTransmitMsduLifetime;
        simtime_t dot11MediumOccupancyLimit;
        bool dot11PrivacyInvoked;
        bool mReceiveDTIMs;
        int dot11CfpPeriod;
        simtime_t dot11CfpMaxDuration;
        simtime_t dot11AuthenticationResponseTimeout;
        int dot11RtsThreshold;
        int dot11ShortRetryLimit;
        int dot11WepDefaultKeyId;
        int dot11CurrentChannelNumber;
        int dot11CurrentSet;
        int dot11CurrentPattern;
        int dot11CurrentIndex;

        /* Write-Only attributes */

    protected:
        void handleMessage(cMessage *msg) override;
        void initialize(int stage) override;
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACMIB_H
