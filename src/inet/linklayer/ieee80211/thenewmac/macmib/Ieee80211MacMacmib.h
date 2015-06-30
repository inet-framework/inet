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

#ifndef __INET_IEEE80211MACMACMIB_H
#define __INET_IEEE80211MACMACMIB_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacMacsorts.h"
#include "inet/linklayer/ieee80211/thenewmac/macsorts/Ieee80211MacEnumeratedMacStaTypes.h"
#include "inet/linklayer/common/MACAddress.h"

namespace inet {
namespace ieee80211 {

/*
 * StationConfig Table -- p. 2382
 */
class INET_API Ieee80211MacMacmibStationConfigTable
{
    protected:
        Tu dot11MediumOccupancyLimit;
        bool dot11CfPollable;
        int dot11CfpPeriod;
        int dot11CfpMaxDuration;
        Tu dot11AuthenticationResponseTimeout;
        bool dot11PrivacyOptionImplemented = true;
        PsMode dot11PowerMangementMode = PsMode::PsMode_sta_active;
        std::string dot11DesiredSSID;
        BssType dot11DesiredBSSType;
        std::string dot11OperationalRateSet;
        Tu dot11BeaconPeriod;
        int dot11DtimPeriod;
        Tu dot11AssociationResponseTimeout;
        std::string dot11DisassociateReason;
        MACAddress dot11DisassociateStation;
        std::string dot11DeauthenticateReason;
        MACAddress dot11DeauthenticateStation;
        int dot11AuthenticateFailStatus;
        MACAddress dot11AuthenticateFailStation;
        bool dot11MultiDomainCapabilityImplemented = false;

    public:
       Tu getDot11AssociationResponseTimeout() const { return dot11AssociationResponseTimeout; }
       void setDot11AssociationResponseTimeout(Tu dot11AssociationResponseTimeout) { this->dot11AssociationResponseTimeout = dot11AssociationResponseTimeout; }
       const MACAddress& getDot11AuthenticateFailStation() const { return dot11AuthenticateFailStation; }
       void setDot11AuthenticateFailStation(const MACAddress& dot11AuthenticateFailStation) { this->dot11AuthenticateFailStation = dot11AuthenticateFailStation; }
       int getDot11AuthenticateFailStatus() const { return dot11AuthenticateFailStatus; }
       void setDot11AuthenticateFailStatus(int dot11AuthenticateFailStatus) { this->dot11AuthenticateFailStatus = dot11AuthenticateFailStatus; }
       const Tu getDot11AuthenticationResponseTimeout() const { return dot11AuthenticationResponseTimeout; }
       void setDot11AuthenticationResponseTimeout(Tu dot11AuthenticationResponseTimeout) { this->dot11AuthenticationResponseTimeout = dot11AuthenticationResponseTimeout; }
       Tu getDot11BeaconPeriod() const { return dot11BeaconPeriod; }
       void setDot11BeaconPeriod(const Tu dot11BeaconPeriod) { this->dot11BeaconPeriod = dot11BeaconPeriod; }
       int getDot11CfpMaxDuration() const { return dot11CfpMaxDuration; }
       void setDot11CfpMaxDuration(int dot11CfpMaxDuration) { this->dot11CfpMaxDuration = dot11CfpMaxDuration; }
       bool isDot11CfPollable() const { return dot11CfPollable; }
       void setDot11CfPollable(bool dot11CfPollable) { this->dot11CfPollable = dot11CfPollable; }
       int getDot11CfpPeriod() const { return dot11CfpPeriod; }
       void setDot11CfpPeriod(int dot11CfpPeriod) { this->dot11CfpPeriod = dot11CfpPeriod; }
       const std::string& getDot11DeauthenticateReason() const { return dot11DeauthenticateReason; }
       void setDot11DeauthenticateReason(const std::string& dot11DeauthenticateReason) { this->dot11DeauthenticateReason = dot11DeauthenticateReason; }
       const MACAddress& getDot11DeauthenticateStation() const { return dot11DeauthenticateStation; }
       void setDot11DeauthenticateStation(const MACAddress& dot11DeauthenticateStation) { this->dot11DeauthenticateStation = dot11DeauthenticateStation; }
       BssType getDot11DesiredBssType() const { return dot11DesiredBSSType; }
       void setDot11DesiredBssType(BssType dot11DesiredBssType) { dot11DesiredBSSType = dot11DesiredBssType; }
       const std::string& getDot11DesiredSsid() const { return dot11DesiredSSID; }
       void setDot11DesiredSsid(const std::string& dot11DesiredSsid) { dot11DesiredSSID = dot11DesiredSsid; }
       const std::string& getDot11DisassociateReason() const { return dot11DisassociateReason; }
       void setDot11DisassociateReason(const std::string& dot11DisassociateReason) { this->dot11DisassociateReason = dot11DisassociateReason; }
       const MACAddress& getDot11DisassociateStation() const { return dot11DisassociateStation; }
       void setDot11DisassociateStation(const MACAddress& dot11DisassociateStation) { this->dot11DisassociateStation = dot11DisassociateStation; }
       int getDot11DtimPeriod() const { return dot11DtimPeriod; }
       void setDot11DtimPeriod(int dot11DtimPeriod) { this->dot11DtimPeriod = dot11DtimPeriod; }
       Tu getDot11MediumOccupancyLimit() const { return dot11MediumOccupancyLimit; }
       void setDot11MediumOccupancyLimit(Tu dot11MediumOccupancyLimit) { this->dot11MediumOccupancyLimit = dot11MediumOccupancyLimit; }
       bool isDot11MultiDomainCapabilityImplemented() const { return dot11MultiDomainCapabilityImplemented; }
       void setDot11MultiDomainCapabilityImplemented(bool dot11MultiDomainCapabilityImplemented = false) { this->dot11MultiDomainCapabilityImplemented = dot11MultiDomainCapabilityImplemented; }
       const std::string& getDot11OperationalRateSet() const { return dot11OperationalRateSet; }
       void setDot11OperationalRateSet(const std::string& dot11OperationalRateSet) {this->dot11OperationalRateSet = dot11OperationalRateSet; }
       PsMode getDot11PowerMangementMode() const { return dot11PowerMangementMode; }
       void setDot11PowerMangementMode(PsMode dot11PowerMangementMode) { this->dot11PowerMangementMode = dot11PowerMangementMode; }
       bool isDot11PrivacyOptionImplemented() const { return dot11PrivacyOptionImplemented; }
       void setDot11PrivacyOptionImplemented(bool dot11PrivacyOptionImplemented = true) { this->dot11PrivacyOptionImplemented = dot11PrivacyOptionImplemented; }
};

/*
 * Counters Table -- p. 2384
 */
class INET_API Ieee80211MacMacmibCountersTable
{
    protected:
        int dot11TransmittedFragmentCount = 0;
        int dot11MulticastTransmittedFrameCount = 0;
        int dot11FailedCount = 0;
        int dot11RetryCount = 0;
        int dot11MultipleRetryCount = 0;
        int dot11RtsSuccessCount = 0;
        int dot11RtsFailureCount = 0;
        int dot11AckFailureCount = 0;
        int dot11ReceivedFragmentCount = 0;
        int dot11MulticastReceivedFrameCount = 0;
        int dot11FcsErrorCount = 0;
        int dot11FrameDuplicateCount = 0;

    public:
        int getDot11AckFailureCount() const { return dot11AckFailureCount; }
        void setDot11AckFailureCount(int dot11AckFailureCount = 0) { this->dot11AckFailureCount = dot11AckFailureCount; }
        void incDot11AckFailureCount() { this->dot11AckFailureCount++; }
        int getDot11FailedCount() const { return dot11FailedCount; }
        void setDot11FailedCount(int dot11FailedCount = 0) { this->dot11FailedCount = dot11FailedCount; }
        void incDot11FailedCount() { this->dot11FailedCount++; }
        int getDot11FcsErrorCount() const { return dot11FcsErrorCount; }
        void setDot11FcsErrorCount(int dot11FcsErrorCount = 0) { this->dot11FcsErrorCount = dot11FcsErrorCount; }
        void incDot11FcsErrorCount() { this->dot11FcsErrorCount++; }
        int getDot11FrameDuplicateCount() const { return dot11FrameDuplicateCount; }
        void setDot11FrameDuplicateCount(int dot11FrameDuplicateCount = 0) { this->dot11FrameDuplicateCount = dot11FrameDuplicateCount; }
        void incDot11FrameDuplicateCount() { this->dot11FrameDuplicateCount++; }
        int getDot11MulticastReceivedFrameCount() const { return dot11MulticastReceivedFrameCount; }
        void setDot11MulticastReceivedFrameCount(int dot11MulticastReceivedFrameCount = 0) { this->dot11MulticastReceivedFrameCount = dot11MulticastReceivedFrameCount; }
        void incDot11MulticastReceivedFrameCount() { this->dot11MulticastReceivedFrameCount++; }
        int getDot11MulticastTransmittedFrameCount() const { return dot11MulticastTransmittedFrameCount; }
        void setDot11MulticastTransmittedFrameCount(int dot11MulticastTransmittedFrameCount = 0) { this->dot11MulticastTransmittedFrameCount = dot11MulticastTransmittedFrameCount; }
        void incDot11MulticastTransmittedFrameCount() { this->dot11MulticastTransmittedFrameCount++; }
        int getDot11MultipleRetryCount() const { return dot11MultipleRetryCount; }
        void setDot11MultipleRetryCount(int dot11MultipleRetryCount = 0) { this->dot11MultipleRetryCount = dot11MultipleRetryCount; }
        void incDot11MultipleRetryCount() { this->dot11MultipleRetryCount++; }
        int getDot11ReceivedFragmentCount() const { return dot11ReceivedFragmentCount; }
        void setDot11ReceivedFragmentCount(int dot11ReceivedFragmentCount = 0) { this->dot11ReceivedFragmentCount = dot11ReceivedFragmentCount; }
        void setDot11ReceivedFragmentCount() { this->dot11ReceivedFragmentCount++; }
        int getDot11RetryCount() const { return dot11RetryCount; }
        void setDot11RetryCount(int dot11RetryCount = 0) { this->dot11RetryCount = dot11RetryCount; }
        void incDot11RetryCount() { this->dot11RetryCount++; }
        int getDot11RtsFailureCount() const { return dot11RtsFailureCount; }
        void setDot11RtsFailureCount(int dot11RtsFailureCount = 0) { this->dot11RtsFailureCount = dot11RtsFailureCount; }
        void incDot11RtsFailureCount() { this->dot11RtsFailureCount++; }
        int getDot11RtsSuccessCount() const { return dot11RtsSuccessCount; }
        void incDot11RtsSuccessCount() { this->dot11RtsSuccessCount++; }
        int getDot11TransmittedFragmentCount() const { return dot11TransmittedFragmentCount; }
        void setDot11TransmittedFragmentCount(int dot11TransmittedFragmentCount = 0) { this->dot11TransmittedFragmentCount = dot11TransmittedFragmentCount; }
        void incDot11TransmittedFragmentCount() { this->dot11TransmittedFragmentCount++; }
};

/*
 * Operation table -- p. 2383
 */
class INET_API Ieee80211MacMacmibOperationTable
{
        // TODO: Mib process should set these values.
    protected:
        MACAddress dot11MacAddress;
        int dot11RtsThreshold = 3000;
        int dot11ShortRetryLimit = 7;
        int dot11LongRetryLimit = 4;
        int dot11FragmentationThreshold = 2346;
        Tu dot11MaxTransmitMsduLifetime = 512;
        Tu dot11MaxReceiveLifetime = 512;

    public:
        int getDot11FragmentationThreshold() const { return dot11FragmentationThreshold; }
        void setDot11FragmentationThreshold(int dot11FragmentationThreshold) { this->dot11FragmentationThreshold = dot11FragmentationThreshold; }
        int getDot11LongRetryLimit() const { return dot11LongRetryLimit; }
        void setDot11LongRetryLimit(int dot11LongRetryLimit) { this->dot11LongRetryLimit = dot11LongRetryLimit; }
        const Tu getDot11MaxReceiveLifetime() const { return dot11MaxReceiveLifetime; }
        void setDot11MaxReceiveLifetime(const Tu dot11MaxReceiveLifetime) { this->dot11MaxReceiveLifetime = dot11MaxReceiveLifetime; }
        const Tu getDot11MaxTransmitMsduLifetime() const { return dot11MaxTransmitMsduLifetime; }
        void setDot11MaxTransmitMsduLifetime(const Tu dot11MaxTransmitMsduLifetime) { this->dot11MaxTransmitMsduLifetime = dot11MaxTransmitMsduLifetime; }
        int getDot11RtsThreshold() const { return dot11RtsThreshold; }
        void setDot11RtsThreshold(int dot11RtsThreshold) { this->dot11RtsThreshold = dot11RtsThreshold; }
        int getDot11ShortRetryLimit() const { return dot11ShortRetryLimit; }
        void setDot11ShortRetryLimit(int dot11ShortRetryLimit) { this->dot11ShortRetryLimit = dot11ShortRetryLimit; }
        const MACAddress& getDot11MacAddress() const { return dot11MacAddress; }
        void setDot11MacAddress(const MACAddress& dot11MacAddress) { this->dot11MacAddress = dot11MacAddress; }
};

/*
 * PhyCharacteristic Parameters, with default FH values (TODO)
 */
class INET_API Ieee80211MacMacmibPhyOperationTable
{
    protected:
        Usec aSlotTime = (aCcaTime + aRxTxTurnaroundTime + aAirPropagationTime + aMacProcessingTime);
        Usec aCcaTime;
        Usec aRxTxTurnaroundTime = aTxPlcpDelay + aRxTxSwitchTime + aTxRampOnTime + aTxRfDelay;
        Usec aTxPlcpDelay;
        Usec aRxTxSwitchTime;
        Usec aTxRampOnTime;
        Usec aTxRfDelay;
        Usec aSifsTime = (aRxRfDelay + aRxPlcpDelay + aMacProcessingTime + aRxTxTurnaroundTime);
        Usec aRxRfDelay;
        Usec aRxPlcpDelay = 2;
        Usec aMacProcessingTime = 2;
        Usec aTxRampOffTime = 8;
        Usec aPreambleLength = 96;
        Usec aPlcpHeaderLength = 32;
        int aMpduMaxLength = 4095;
        Usec aAirPropagationTime = 1;
        int aCWmax = 1023;
        int aCWmin = 15;

    public:
        Usec getAirPropagationTime() const
        {
            return aAirPropagationTime;
        }

        void setAirPropagationTime(Usec airPropagationTime )
        {
            aAirPropagationTime = airPropagationTime;
        }

        Usec getCcaTime() const
        {
            return aCcaTime;
        }

        void setCcaTime(Usec ccaTime)
        {
            aCcaTime = ccaTime;
        }

        int getCWmax() const
        {
            return aCWmax;
        }

        void setCWmax(int cWmax)
        {
            aCWmax = cWmax;
        }

        int getCWmin() const
        {
            return aCWmin;
        }

        void setCWmin(int cWmin)
        {
            aCWmin = cWmin;
        }

        Usec getMacProcessingTime() const
        {
            return aMacProcessingTime;
        }

        void setMacProcessingTime(Usec macProcessingTime)
        {
            aMacProcessingTime = macProcessingTime;
        }

        int getMpduMaxLength() const
        {
            return aMpduMaxLength;
        }

        void setMpduMaxLength(int mpduMaxLength)
        {
            aMpduMaxLength = mpduMaxLength;
        }

        Usec getPlcpHeaderLength() const
        {
            return aPlcpHeaderLength;
        }

        void setPlcpHeaderLength(Usec plcpHeaderLength)
        {
            aPlcpHeaderLength = plcpHeaderLength;
        }

        Usec getPreambleLength() const
        {
            return aPreambleLength;
        }

        void setPreambleLength(Usec preambleLength)
        {
            aPreambleLength = preambleLength;
        }

        Usec getRxPlcpDelay() const
        {
            return aRxPlcpDelay;
        }

        void setRxPlcpDelay(Usec rxPlcpDelay)
        {
            aRxPlcpDelay = rxPlcpDelay;
        }

        Usec getRxRfDelay() const
        {
            return aRxRfDelay;
        }

        void setRxRfDelay(Usec rxRfDelay)
        {
            aRxRfDelay = rxRfDelay;
        }

        Usec getRxTxSwitchTime() const
        {
            return aRxTxSwitchTime;
        }

        void setRxTxSwitchTime(Usec rxTxSwitchTime)
        {
            aRxTxSwitchTime = rxTxSwitchTime;
        }

        Usec getRxTxTurnaroundTime() const
        {
            return aRxTxTurnaroundTime;
        }

        void setRxTxTurnaroundTime(Usec rxTxTurnaroundTime)
        {
            aRxTxTurnaroundTime = rxTxTurnaroundTime;
        }

        Usec getSifsTime() const
        {
            return aSifsTime;
        }

        void setSifsTime(Usec sifsTime)
        {
            aSifsTime = sifsTime;
        }

        Usec getSlotTime() const
        {
            return aSlotTime;
        }

        void setSlotTime(Usec slotTime)
        {
            aSlotTime = slotTime;
        }

        Usec getTxPlcpDelay() const
        {
            return aTxPlcpDelay;
        }

        void setTxPlcpDelay(Usec txPlcpDelay)
        {
            aTxPlcpDelay = txPlcpDelay;
        }

        Usec getTxRampOffTime() const
        {
            return aTxRampOffTime;
        }

        void setTxRampOffTime(Usec txRampOffTime)
        {
            aTxRampOffTime = txRampOffTime;
        }

        Usec getTxRampOnTime() const
        {
            return aTxRampOnTime;
        }

        void setTxRampOnTime(Usec txRampOnTime)
        {
            aTxRampOnTime = txRampOnTime;
        }

        Usec getTxRfDelay() const
        {
            return aTxRfDelay;
        }

        void setTxRfDelay(Usec txRfDelay)
        {
            aTxRfDelay = txRfDelay;
        }
};

/*
 * This Package contains definitions of the MAC MIB attributes
 * and the subset of the PHY MIB attributes used by the MAC state
 * machines. p. 2382
*/
class INET_API Ieee80211MacMacmibPackage: public cSimpleModule
{

    protected:
        Ieee80211MacMacmibCountersTable *countersTable = nullptr;
        Ieee80211MacMacmibOperationTable *operationTable = nullptr;
        Ieee80211MacMacmibStationConfigTable *stationConfigTable = nullptr;
        Ieee80211MacMacmibPhyOperationTable *phyOperationTable = nullptr;

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        void handleMessage(cMessage *msg) override;
        void initialize(int stage) override;

    public:
        virtual ~Ieee80211MacMacmibPackage();

        Ieee80211MacMacmibCountersTable* getCountersTable() const { return countersTable; }
        Ieee80211MacMacmibOperationTable* getOperationTable() const { return operationTable; }
        Ieee80211MacMacmibStationConfigTable *getStationConfigTable() const { return stationConfigTable; }
        Ieee80211MacMacmibPhyOperationTable *getPhyOperationTable() const { return phyOperationTable; }

};

} /* namespace inet */
} /* namespace ieee80211 */

#endif // ifndef __INET_IEEE80211MACMACMIB_H
