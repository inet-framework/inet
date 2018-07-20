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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrame_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrameSerializer.h"

namespace inet {

namespace ieee80211 {

Register_Serializer(Ieee80211AssociationRequestFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211AssociationResponseFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211AuthenticationFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211BeaconFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211DeauthenticationFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211DisassociationFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211ProbeRequestFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211ProbeResponseFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211ReassociationRequestFrame, Ieee80211MgmtFrameSerializer);
Register_Serializer(Ieee80211ReassociationResponseFrame, Ieee80211MgmtFrameSerializer);

void Ieee80211MgmtFrameSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    if (auto authenticationFrame = dynamicPtrCast<const Ieee80211AuthenticationFrame>(chunk))
    {
        //type = ST_AUTHENTICATION;
        // 1    Authentication algorithm number
        stream.writeUint16Be(0);
        // 2    Authentication transaction sequence number
        stream.writeUint16Be(authenticationFrame->getSequenceNumber());
        // 3    Status code                                 The status code information is reserved in certain Authentication frames as defined in Table 7-17.
        stream.writeUint16Be(authenticationFrame->getStatusCode());
        // 4    Challenge text                              The challenge text information is present only in certain Authentication frames as defined in Table 7-17.
        // Last Vendor Specific                             One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto deauthenticationFrame = dynamicPtrCast<const Ieee80211DeauthenticationFrame>(chunk))
    {
        //type = ST_DEAUTHENTICATION;
        stream.writeUint16Be(deauthenticationFrame->getReasonCode());
    }
    else if (auto disassociationFrame =dynamicPtrCast<const Ieee80211DisassociationFrame>(chunk))
    {
        //type = ST_DISASSOCIATION;
        stream.writeUint16Be(disassociationFrame->getReasonCode());
    }
    else if (auto probeRequestFrame = dynamicPtrCast<const Ieee80211ProbeRequestFrame>(chunk))
    {
        //type = ST_PROBEREQUEST;
        // 1    SSID
        const char *SSID = probeRequestFrame->getSSID();
        unsigned int length = strlen(SSID);
        stream.writeByte(0);    //FIXME dummy, what is it?
        stream.writeByte(length);
        stream.writeBytes((uint8_t *)SSID, B(length));
        // 2    Supported rates
        const Ieee80211SupportedRatesElement& supportedRates = probeRequestFrame->getSupportedRates();
        stream.writeByte(1);
        stream.writeByte(supportedRates.numRates);
        for (int i = 0; i < supportedRates.numRates; i++)
        {
            uint8_t rate = ceil(supportedRates.rate[i]/0.5);
            // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
            stream.writeByte(rate);
        }
        // 3    Request information         May be included if dot11MultiDomainCapabilityEnabled is true.
        // 4    Extended Supported Rates    The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
        // Last Vendor Specific             One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto associationRequestFrame = dynamicPtrCast<const Ieee80211AssociationRequestFrame>(chunk))
    {
        //type = ST_ASSOCIATIONREQUEST;
        // 1    Capability
        stream.writeUint16Be(0);    //FIXME
        // 2    Listen interval
        stream.writeUint16Be(0);    //FIXME
        // 3    SSID
        const char *SSID = associationRequestFrame->getSSID();
        unsigned int length = strlen(SSID);
        stream.writeByte(0);    //FIXME dummy, what is it?
        stream.writeByte(length);
        stream.writeBytes((uint8_t *)SSID, B(length));
        // 4    Supported rates
        const Ieee80211SupportedRatesElement& supportedRates = associationRequestFrame->getSupportedRates();
        stream.writeByte(1);
        stream.writeByte(supportedRates.numRates);
        for (int i = 0; i < supportedRates.numRates; i++) {
            uint8_t rate = ceil(supportedRates.rate[i]/0.5);
            // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
            stream.writeByte(rate);
        }
        // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
        // 6    Power Capability           The Power Capability element shall be present if dot11SpectrumManagementRequired is true.
        // 7    Supported Channel          The Supported Channels element shall be present if dot11SpectrumManagementRequired is true.
        // 8    RSN                        The RSN information element is only present within Association Request frames generated by STAs that have dot11RSNAEnabled set to TRUE.
        // 9    QoS Capability             The QoS Capability element is present when dot11QosOption- Implemented is true.
        // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto reassociationRequestFrame = dynamicPtrCast<const Ieee80211ReassociationRequestFrame>(chunk))
    {
        //type = ST_REASSOCIATIONREQUEST;
        // 1    Capability
        stream.writeUint16Be(0);    //FIXME
        // 2    Listen interval
        stream.writeUint16Be(0);    //FIXME
        // 3    Current AP address
        stream.writeMacAddress(reassociationRequestFrame->getCurrentAP());
        // 4    SSID
        const char *SSID = reassociationRequestFrame->getSSID();
        unsigned int length = strlen(SSID);
        //FIXME buffer.writeByte(buf + packetLength, ???);
        stream.writeByte(0);    //FIXME
        stream.writeByte(length);
        stream.writeBytes((uint8_t *)SSID, B(length));
        // 5    Supported rates
        const Ieee80211SupportedRatesElement& supportedRates = reassociationRequestFrame->getSupportedRates();
        stream.writeByte(1);
        stream.writeByte(supportedRates.numRates);
        for (int i = 0; i < supportedRates.numRates; i++)
        {
            uint8_t rate = ceil(supportedRates.rate[i]/0.5);
            // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
            stream.writeByte(rate);
        }
        // 6    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
        // 7    Power Capability           The Power Capability element shall be present if dot11SpectrumManagementRequired is true.
        // 8    Supported Channels         The Supported Channels element shall be present if dot11SpectrumManagementRequired is true.
        // 9    RSN                        The RSN information element is only present within Reassociation Request frames generated by STAs that have dot11RSNAEnabled set to TRUE.
        // 10   QoS Capability             The QoS Capability element is present when dot11QosOption- Implemented is true.
        // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto associationResponseFrame = dynamicPtrCast<const Ieee80211AssociationResponseFrame>(chunk))
    {
        //type = ST_ASSOCIATIONRESPONSE;
        // 1    Capability
        stream.writeUint16Be(0);    //FIXME
        // 2    Status code
        stream.writeUint16Be(associationResponseFrame->getStatusCode());
        // 3    AID
        stream.writeUint16Be(associationResponseFrame->getAid());
        // 4    Supported rates
        stream.writeByte(1);
        stream.writeByte(associationResponseFrame->getSupportedRates().numRates);
        for (int i = 0; i < associationResponseFrame->getSupportedRates().numRates; i++)
        {
            uint8_t rate = ceil(associationResponseFrame->getSupportedRates().rate[i]/0.5);
            // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
            stream.writeByte(rate);
        }
        // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
        // 6    EDCA Parameter Set
        // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto reassociationResponseFrame = dynamicPtrCast<const Ieee80211ReassociationResponseFrame>(chunk))
    {
        //type = ST_REASSOCIATIONRESPONSE;
        // 1    Capability
        stream.writeUint16Be(0);    //FIXME
        // 2    Status code
        stream.writeUint16Be(reassociationResponseFrame->getStatusCode());
        // 3    AID
        stream.writeUint16Be(reassociationResponseFrame->getAid());
        // 4    Supported rates
        stream.writeByte(1);
        stream.writeByte(reassociationResponseFrame->getSupportedRates().numRates);
        for (int i = 0; i < reassociationResponseFrame->getSupportedRates().numRates; i++)
        {
            uint8_t rate = ceil(reassociationResponseFrame->getSupportedRates().rate[i]/0.5);
            // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
            stream.writeByte(rate);
        }
        // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
        // 6    EDCA Parameter Set
        // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto beaconFrame = dynamicPtrCast<const Ieee80211BeaconFrame>(chunk))
    {
        //type = ST_BEACON;
        // 1    Timestamp
        stream.writeUint64Be(simTime().raw());   //FIXME
        // 2    Beacon interval
        stream.writeUint16Be((uint16_t)(beaconFrame->getBeaconInterval().inUnit(SIMTIME_US)/1024));
        // 3    Capability
        stream.writeUint16Be(0);    //FIXME set  capability
        // 4    Service Set Identifier (SSID)
        const char *SSID = beaconFrame->getSSID();
        unsigned int length = strlen(SSID);
        stream.writeByte(0);    //FIXME
        stream.writeByte(length);
        stream.writeBytes((uint8_t *)SSID, B(length));
        // 5    Supported rates
        stream.writeByte(1);
        stream.writeByte(beaconFrame->getSupportedRates().numRates);
        for (int i = 0; i < beaconFrame->getSupportedRates().numRates; i++)
        {
            uint8_t rate = ceil(beaconFrame->getSupportedRates().rate[i]/0.5);
            // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
            stream.writeByte(rate);
        }
        // 6    Frequency-Hopping (FH) Parameter Set   The FH Parameter Set information element is present within Beacon frames generated by STAs using FH PHYs.
        // 7    DS Parameter Set                       The DS Parameter Set information element is present within Beacon frames generated by STAs using Clause 15, Clause 18, and Clause 19 PHYs.
        // 8    CF Parameter Set                       The CF Parameter Set information element is present only within Beacon frames generated by APs supporting a PCF.
        // 9    IBSS Parameter Set                     The IBSS Parameter Set information element is present only within Beacon frames generated by STAs in an IBSS.
        // 10   Traffic indication map (TIM)           The TIM information element is present only within Beacon frames generated by APs.
        // 11   Country                                The Country information element shall be present when dot11MultiDomainCapabilityEnabled is true or dot11SpectrumManagementRequired is true.
        // 12   FH Parameters                          FH Parameters as specified in 7.3.2.10 may be included if dot11MultiDomainCapabilityEnabled is true.
        // 13   FH Pattern Table                       FH Pattern Table information as specified in 7.3.2.11 may be included if dot11MultiDomainCapabilityEnabled is true.
        // 14   Power Constraint                       Power Constraint element shall be present if dot11SpectrumManagementRequired is true.
        // 15   Channel Switch Announcement            Channel Switch Announcement element may be present if dot11SpectrumManagementRequired is true.
        // 16   Quiet                                  Quiet element may be present if dot11SpectrumManagementRequired is true.
        // 17   IBSS DFS                               IBSS DFS element shall be present if dot11SpectrumManagementRequired is true in an IBSS.
        // 18   TPC Report                             TPC Report element shall be present if dot11SpectrumManagementRequired is true.
        // 19   ERP Information                        The ERP Information element is present within Beacon frames generated by STAs using extended rate PHYs (ERPs) defined in Clause 19 and is optionally present in other cases.
        // 20   Extended Supported Rates               The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
        // 21   RSN                                    The RSN information element shall be present within Beacon frames generated by STAs that have dot11RSNAEnabled set to TRUE.
        // 22   BSS Load                               The BSS Load element is present when dot11QosOption- Implemented and dot11QBSSLoadImplemented are both true.
        // 23   EDCA Parameter Set                     The EDCA Parameter Set element is present when dot11QosOptionImplemented is true and the QoS Capability element is not present.
        // 24   QoS Capability                         The QoS Capability element is present when dot11QosOption- Implemented is true and EDCA Parameter Set element is not present.
        // Last Vendor Specific                        One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
    }
    else if (auto probeResponseFrame = dynamicPtrCast<const Ieee80211ProbeResponseFrame>(chunk))
    {
        //type = ST_PROBERESPONSE;
        // 1      Timestamp
        stream.writeUint64Be(simTime().raw());   //FIXME
        // 2      Beacon interval
        stream.writeUint16Be((uint16_t)(probeResponseFrame->getBeaconInterval().inUnit(SIMTIME_US)/1024));
        // 3      Capability
        stream.writeUint16Be(0);    //FIXME
        // 4      SSID
        const char *SSID = probeResponseFrame->getSSID();
        unsigned int length = strlen(SSID);
        stream.writeByte(0);    //FIXME
        stream.writeByte(length);
        stream.writeBytes((uint8_t *)SSID, B(length));
        // 5      Supported rates
        stream.writeByte(1);
        stream.writeByte(probeResponseFrame->getSupportedRates().numRates);
        for (int i = 0; i < probeResponseFrame->getSupportedRates().numRates; i++)
        {
            uint8_t rate = ceil(probeResponseFrame->getSupportedRates().rate[i]/0.5);
            // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
            stream.writeByte(rate);
        }
        // 6      FH Parameter Set                The FH Parameter Set information element is present within Probe Response frames generated by STAs using FH PHYs.
        // 7      DS Parameter Set                The DS Parameter Set information element is present within Probe Response frames generated by STAs using Clause 15, Clause 18, and Clause 19 PHYs.
        // 8      CF Parameter Set                The CF Parameter Set information element is present only within Probe Response frames generated by APs supporting a PCF.
        // 9      IBSS Parameter Set              The IBSS Parameter Set information element is present only within Probe Response frames generated by STAs in an IBSS.
        // 10     Country                         Included if dot11MultiDomainCapabilityEnabled or dot11SpectrumManagementRequired is true.
        // 11     FH Parameters                   FH Parameters, as specified in 7.3.2.10, may be included if dot11MultiDomainCapabilityEnabled is true.
        // 12     FH Pattern Table                FH Pattern Table information, as specified in 7.3.2.11, may be included if dot11MultiDomainCapabilityEnabled is true.
        // 13     Power Constraint                Shall be included if dot11SpectrumManagementRequired is true.
        // 14     Channel Switch Announcement     May be included if dot11SpectrumManagementRequired is true.
        // 15     Quiet                           May be included if dot11SpectrumManagementRequired is true.
        // 16     IBSS DFS                        Shall be included if dot11SpectrumManagementRequired is true in an IBSS.
        // 17     TPC Report                      Shall be included if dot11SpectrumManagementRequired is true.
        // 18     ERP Information                 The ERP Information element is present within Probe Response frames generated by STAs using ERPs and is optionally present in other cases.
        // 19     Extended Supported Rates        The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
        // 20     RSN                             The RSN information element is only present within Probe Response frames generated by STAs that have dot11RSNA- Enabled set to TRUE.
        // 21     BSS Load                        The BSS Load element is present when dot11QosOption- Implemented and dot11QBSSLoadImplemented are both true.
        // 22     EDCA Parameter Set              The EDCA Parameter Set element is present when dot11QosOptionImplemented is true.
        // Last�1 Vendor Specific                 One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements, except the Requested Information elements.
        // Last�n Requested information elements  Elements requested by the Request information element of the Probe Request frame.
    }
    else
        throw cRuntimeError("Cannot serialize frame");
}

const Ptr<Chunk> Ieee80211MgmtFrameSerializer::deserialize(MemoryInputStream& stream) const
{
    switch(0) // TODO: receive and dispatch on type_info parameter
    {
        case 0xB0: // ST_AUTHENTICATION
        {
            auto frame = makeShared<Ieee80211AuthenticationFrame>();
            stream.readUint16Be();
            frame->setSequenceNumber(stream.readUint16Be());
            frame->setStatusCode((Ieee80211StatusCode)stream.readUint16Be());
            return frame;
        }

        case 0xC0: //ST_ST_DEAUTHENTICATION
        {
            auto frame = makeShared<Ieee80211DeauthenticationFrame>();
            frame->setReasonCode((Ieee80211ReasonCode)stream.readUint16Be());
            return frame;
        }

        case 0xA0: // ST_DISASSOCIATION
        {
            auto frame = makeShared<Ieee80211DisassociationFrame>();
            frame->setReasonCode((Ieee80211ReasonCode)stream.readUint16Be());
            return frame;
        }

        case 0x40: // ST_PROBEREQUEST
        {
            auto frame = makeShared<Ieee80211ProbeRequestFrame>();

            char SSID[256];
            stream.readByte();
            unsigned int length = stream.readByte();
            stream.readBytes((uint8_t *)SSID, B(length));
            SSID[length] = '\0';
            frame->setSSID(SSID);

            Ieee80211SupportedRatesElement supRat;
            stream.readByte();
            supRat.numRates = stream.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
            frame->setSupportedRates(supRat);
            return frame;
        }

        case 0x00: // ST_ASSOCIATIONREQUEST
        {
            auto frame = makeShared<Ieee80211AssociationRequestFrame>();

            char SSID[256];
            stream.readByte();
            unsigned int length = stream.readByte();
            stream.readBytes((uint8_t *)SSID, B(length));
            SSID[length] = '\0';
            frame->setSSID(SSID);

            Ieee80211SupportedRatesElement supRat;
            stream.readByte();
            supRat.numRates = stream.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
            frame->setSupportedRates(supRat);
            return frame;
        }

        case 0x02: // ST_REASSOCIATIONREQUEST
        {
            auto frame = makeShared<Ieee80211ReassociationRequestFrame>();
            stream.readUint16Be();
            stream.readUint16Be();

            frame->setCurrentAP(stream.readMacAddress());

            char SSID[256];
            stream.readByte();
            unsigned int length = stream.readByte();
            stream.readBytes((uint8_t *)SSID, B(length));
            SSID[length] = '\0';
            frame->setSSID(SSID);

            Ieee80211SupportedRatesElement supRat;
            stream.readByte();
            supRat.numRates = stream.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
            frame->setSupportedRates(supRat);
            return frame;
        }

        case 0x01: // ST_ASSOCIATIONRESPONSE
        {
            auto frame = makeShared<Ieee80211AssociationResponseFrame>();
            stream.readUint16Be();
            frame->setStatusCode((Ieee80211StatusCode)stream.readUint16Be());
            frame->setAid(stream.readUint16Be());

            Ieee80211SupportedRatesElement supRat;
            stream.readByte();
            supRat.numRates = stream.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
            frame->setSupportedRates(supRat);
            return frame;
        }

        case 0x03: // ST_REASSOCIATIONRESPONSE
        {
            auto frame = makeShared<Ieee80211ReassociationResponseFrame>();
            stream.readUint16Be();
            frame->setStatusCode((Ieee80211StatusCode)stream.readUint16Be());
            frame->setAid(stream.readUint16Be());

            Ieee80211SupportedRatesElement supRat;
            stream.readByte();
            supRat.numRates = stream.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
            frame->setSupportedRates(supRat);
            return frame;
        }

        case 0x80: // ST_BEACON
        {
            auto frame = makeShared<Ieee80211BeaconFrame>();

            simtime_t timetstamp;
            timetstamp.setRaw(stream.readUint64Be()); // TODO: store timestamp

            frame->setBeaconInterval(SimTime((int64_t)stream.readUint16Be()*1024, SIMTIME_US));
            stream.readUint16Be();     // Capability

            char SSID[256];
            stream.readByte();
            unsigned int length = stream.readByte();
            stream.readBytes((uint8_t *)SSID, B(length));
            SSID[length] = '\0';
            frame->setSSID(SSID);

            Ieee80211SupportedRatesElement supRat;
            stream.readByte();
            supRat.numRates = stream.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
            frame->setSupportedRates(supRat);
            return frame;
        }

        case 0x50: // ST_PROBERESPONSE
        {
            auto frame = makeShared<Ieee80211ProbeResponseFrame>();

            simtime_t timestamp;
            timestamp.setRaw(stream.readUint64Be()); // TODO: store timestamp

            frame->setBeaconInterval(SimTime((int64_t)stream.readUint16Be() * 1024, SIMTIME_US));
            stream.readUint16Be();

            char SSID[256];
            stream.readByte();
            unsigned int length = stream.readByte();
            stream.readBytes((uint8_t *)SSID, B(length));
            SSID[length] = '\0';
            frame->setSSID(SSID);

            Ieee80211SupportedRatesElement supRat;
            stream.readByte();
            supRat.numRates = stream.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
            frame->setSupportedRates(supRat);
            return frame;
        }

        default:
            throw cRuntimeError("Cannot deserialize frame");
    }
}

} // namespace ieee80211

} // namespace inet

