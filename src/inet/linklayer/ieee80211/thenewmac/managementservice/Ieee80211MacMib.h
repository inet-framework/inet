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
#include "inet/linklayer/ieee80211/thenewmac/signals/Ieee80211MacSignals_m.h"
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
        virtual void emitMlmeGetConfirm(MibStatus status, std::string mibAttrib, int mibValue) = 0;
        virtual void emitMlmeSetConfirm(MibStatus status, std::string mibAttrib) = 0;
        virtual void emitMlmeResetConfirm(MlmeStatus status) = 0;

};

class INET_API Ieee80211MacMib : public IIeee80211MacMib
{
    protected:
        enum MacMibState
        {
            MAC_MIB_STATE_MIB_IDLE
        };
        enum MibAttribType
        {
            MIB_ATTRIB_TYPE_INVALID,
            MIB_ATTRIB_TYPE_VALID,
            MIB_ATTRIB_TYPE_READ_ONLY,
            MIB_ATTRIB_TYPE_WRITE_ONLY
        };

    protected:
        MacMibState state = MAC_MIB_STATE_MIB_IDLE;

        bool dflt; // dcl dflt Boolean ;
        std::string x; // dcl x MibAtrib ;
        int v; // dcl v MibValue ;
        MACAddress adr;

        Ieee80211MacMacmibPackage *macmib = nullptr;
        Ieee80211MacMacsorts *macsorts = nullptr;

        // Read-Write attributes
//        dot11AuthenticationAlgorithms AuthTypeSet:=incl(open_system, shared_key),
        bool dot11ExcludeUnencrypted;
        int dot11FragmentationThreshold;
        std::vector<MACAddress> dot11GroupAddresses;
        int dot11LongRetryLimit;
        Tu dot11MaxReceiveLifetime;
        int dot11MaxTransmitMsduLifetime;
        Tu dot11MediumOccupancyLimit;
        bool dot11PrivacyInvoked;
        bool mReceiveDTIMs;
        int dot11CfpPeriod;
        Tu dot11CfpMaxDuration;
        Tu dot11AuthenticationResponseTimeout;
        int dot11RtsThreshold;
        int dot11ShortRetryLimit;
        int dot11WepDefaultKeyId;
        int dot11CurrentChannelNumber;
        int dot11CurrentSet;
        int dot11CurrentPattern;
        int dot11CurrentIndex;

    protected:
        void handleMessage(cMessage *msg) override;
        void initialize(int stage) override;

        virtual void handleMlmeResetRequest(Ieee80211MacSignalMlmeResetRequest *resetRequest);
        virtual void handleMlmeGetRequest(Ieee80211MacSignalMlmeGetRequest *getRequest);
        virtual void handleMlmeSetRequest(Ieee80211MacSignalMlmeSetRequest *setRequest);

        virtual void emitResetMac();
        virtual void emitMlmeGetConfirm(MibStatus status, std::string mibAttrib, int mibValue);
        virtual void emitMlmeSetConfirm(MibStatus status, std::string mibAttrib);
        virtual void emitMlmeResetConfirm(MlmeStatus status);

        MibAttribType getMibAttribType(std::string mibAttrib) const;
        bool isDeclaredHere(std::string mibAttrib) const;
        int importMibValue(std::string mibAttrib) const;
        void exportMibValue(std::string mibAttrib) const;
        int getMibValue(std::string mibAttrib) const;
        void setMibValue(std::string mibAttrib, int mibValue);
        void exportValuesOfAttributesDeclaredHere();
};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACMIB_H
