//
// Copyright (C) 2014 OpenSim Ltd.
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
#include "inet/common/serializer/ieee80211/Ieee80211MacHeaderSerializer.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h"

namespace inet {

namespace serializer {

Register_Serializer(Ieee80211Frame, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211DataOrMgmtFrame, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211DataFrame, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211ManagementHeader, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211MsduSubframe, Ieee80211MacHeaderSerializer);

Register_Serializer(Ieee80211ACKFrame, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211RTSFrame, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211CTSFrame, Ieee80211MacHeaderSerializer);

Register_Serializer(Ieee80211BasicBlockAckReq, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211CompressedBlockAckReq, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211MultiTidBlockAckReq, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211BasicBlockAck, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211CompressedBlockAck, Ieee80211MacHeaderSerializer);
Register_Serializer(Ieee80211MultiTidBlockAck, Ieee80211MacHeaderSerializer);

Register_Serializer(Ieee80211MacTrailer, Ieee80211MacTrailerSerializer);

void Ieee80211MacHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<Chunk>& chunk) const
{
    if (auto ackFrame = std::dynamic_pointer_cast<Ieee80211ACKFrame>(chunk))
    {
        stream.writeByte(0xD4);
        stream.writeByte(0);
        stream.writeUint16Be(ackFrame->getDuration().inUnit(SIMTIME_US));
        stream.writeMACAddress(ackFrame->getReceiverAddress());
    }
    else if (auto rtsFrame = std::dynamic_pointer_cast<Ieee80211RTSFrame>(chunk))
    {
        stream.writeByte(0xB4);
        stream.writeByte(0);
        stream.writeUint16Be(rtsFrame->getDuration().inUnit(SIMTIME_US));
        stream.writeMACAddress(rtsFrame->getReceiverAddress());
        stream.writeMACAddress(rtsFrame->getTransmitterAddress());
    }
    else if (auto ctsFrame = std::dynamic_pointer_cast<Ieee80211CTSFrame>(chunk))
    {
        stream.writeByte(0xC4);
        stream.writeByte(0);
        stream.writeUint16Be(ctsFrame->getDuration().inUnit(SIMTIME_US));
        stream.writeMACAddress(ctsFrame->getReceiverAddress());
    }
    else if (auto dataOrMgmtFrame = std::dynamic_pointer_cast<Ieee80211DataOrMgmtFrame>(chunk))
    {
        uint8_t type = dataOrMgmtFrame->getType();
        stream.writeByte(((type & 0x0F) << 4) | ((type & 0x30) >> 2));  //without Qos=0x08, with Qos=0x88
        uint8_t fc1;
        // TODO: Order, Protected Frame, MoreData, Power Mgt
        fc1 = (dataOrMgmtFrame->getRetry() ? 8 : 0)
                | (dataOrMgmtFrame->getMoreFragments() ? 4 : 0)
                | (dataOrMgmtFrame->getFromDS() ? 2 : 0)
                | (dataOrMgmtFrame->getToDS() ? 1 : 0);
        stream.writeByte(fc1);
        stream.writeUint16Be(dataOrMgmtFrame->getDuration().inUnit(SIMTIME_US));
        stream.writeMACAddress(dataOrMgmtFrame->getReceiverAddress());
        stream.writeMACAddress(dataOrMgmtFrame->getTransmitterAddress());
        stream.writeMACAddress(dataOrMgmtFrame->getAddress3());
        stream.writeUint16Le(dataOrMgmtFrame->getSequenceNumber() << 4
                | dataOrMgmtFrame->getFragmentNumber());

        if (auto dataFrame = std::dynamic_pointer_cast<Ieee80211DataFrame>(chunk)) {
            if (dataFrame->getFromDS() && dataFrame->getToDS())
                stream.writeMACAddress(dataFrame->getAddress4());

            if (dataOrMgmtFrame->getType() == ST_DATA_WITH_QOS)
                stream.writeUint16Le(dataFrame->getQos() | dataFrame->getTid() | (dataFrame->getAMsduPresent() ? 0x0080 : 0x0000));
        }

        if (auto authenticationFrame = std::dynamic_pointer_cast<Ieee80211AuthenticationFrameBody>(chunk))
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
        else if (auto deauthenticationFrame = std::dynamic_pointer_cast<Ieee80211DeauthenticationFrameBody>(chunk))
        {
            //type = ST_DEAUTHENTICATION;
            stream.writeUint16Be(deauthenticationFrame->getReasonCode());
        }
        else if (auto disassociationFrame =std::dynamic_pointer_cast<Ieee80211DisassociationFrameBody>(chunk))
        {
            //type = ST_DISASSOCIATION;
            stream.writeUint16Be(disassociationFrame->getReasonCode());
        }
        else if (auto probeRequestFrame = std::dynamic_pointer_cast<Ieee80211ProbeRequestFrameBody>(chunk))
        {
            //type = ST_PROBEREQUEST;
            // 1    SSID
            const char *SSID = probeRequestFrame->getSSID();
            unsigned int length = strlen(SSID);
            stream.writeByte(0);    //FIXME dummy, what is it?
            stream.writeByte(length);
            stream.writeBytes((uint8_t *)SSID, byte(length));
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
        else if (auto associationRequestFrame = std::dynamic_pointer_cast<Ieee80211AssociationRequestFrameBody>(chunk))
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
            stream.writeBytes((uint8_t *)SSID, byte(length));
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
        else if (auto reassociationRequestFrame = std::dynamic_pointer_cast<Ieee80211ReassociationRequestFrameBody>(chunk))
        {
            //type = ST_REASSOCIATIONREQUEST;
            // 1    Capability
            stream.writeUint16Be(0);    //FIXME
            // 2    Listen interval
            stream.writeUint16Be(0);    //FIXME
            // 3    Current AP address
            stream.writeMACAddress(reassociationRequestFrame->getCurrentAP());
            // 4    SSID
            const char *SSID = reassociationRequestFrame->getSSID();
            unsigned int length = strlen(SSID);
            //FIXME buffer.writeByte(buf + packetLength, ???);
            stream.writeByte(0);    //FIXME
            stream.writeByte(length);
            stream.writeBytes((uint8_t *)SSID, byte(length));
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
        else if (auto associationResponseFrame = std::dynamic_pointer_cast<Ieee80211AssociationResponseFrameBody>(chunk))
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
        else if (auto reassociationResponseFrame = std::dynamic_pointer_cast<Ieee80211ReassociationResponseFrameBody>(chunk))
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
        else if (auto beaconFrame = std::dynamic_pointer_cast<Ieee80211BeaconFrameBody>(chunk))
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
            stream.writeBytes((uint8_t *)SSID, byte(length));
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
        else if (auto probeResponseFrame = std::dynamic_pointer_cast<Ieee80211ProbeResponseFrameBody>(chunk))
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
            stream.writeBytes((uint8_t *)SSID, byte(length));
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
        else if (std::dynamic_pointer_cast<Ieee80211ActionFrame>(chunk))
        {
            //type = ST_ACTION;
            // 1    Action
            // Last One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
        }
    }
    else if (auto msduSubframe = std::dynamic_pointer_cast<Ieee80211MsduSubframe>(chunk))
    {
        stream.writeMACAddress(msduSubframe->getDa());
        stream.writeMACAddress(msduSubframe->getSa());
        stream.writeUint16Be(msduSubframe->getLength());
    }
    else
        throw cRuntimeError("Ieee80211Serializer: cannot serialize the frame");
}

void Ieee80211MacHeaderSerializer::parseDataOrMgmtFrame(MemoryInputStream &stream, Ptr<Ieee80211DataOrMgmtFrame> frame, short type, uint8_t fc1) const
{
    frame->setType(type);
    frame->setToDS(fc1 & 0x1);
    frame->setFromDS(fc1 & 0x2);
    frame->setMoreFragments(fc1 & 0x4);
    frame->setRetry(fc1 & 0x8);
    frame->setDuration(SimTime(stream.readUint16Be() / 1000.0)); // i_dur
    frame->setReceiverAddress(stream.readMACAddress());
    frame->setTransmitterAddress(stream.readMACAddress());
    frame->setAddress3(stream.readMACAddress());
    uint16_t seq = stream.readUint16Le();   // i_seq
    frame->setSequenceNumber(seq >> 4);
    frame->setFragmentNumber(seq & 0x0F);

    if ((type == ST_DATA || type == ST_DATA_WITH_QOS) && frame->getFromDS() && frame->getToDS())
        std::dynamic_pointer_cast<Ieee80211DataFrame>(frame)->setAddress4(stream.readMACAddress());
    if (type == ST_DATA_WITH_QOS)
        std::dynamic_pointer_cast<Ieee80211DataFrame>(frame)->setQos(stream.readUint16Le());
}

Ptr<Chunk> Ieee80211MacHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    Ptr<Ieee80211Frame> frame = nullptr;

    uint8_t type = stream.readByte();
    uint8_t fc_1 = stream.readByte();  (void)fc_1; // fc_1
    switch(type)
    {
        case 0xD4: // ST_ACK    //TODO ((ST_ACK & 0x0F) << 4) | ((ST_ACK & 0x30) >> 2)
        {
            auto ackFrame = std::make_shared<Ieee80211ACKFrame>();
            ackFrame->setType(ST_ACK);
            ackFrame->setToDS(false);
            ackFrame->setFromDS(false);
            ackFrame->setRetry(false);
            ackFrame->setMoreFragments(false);
            ackFrame->setDuration(SimTime((double)stream.readUint16Be()/1000.0));    //i_dur
            ackFrame->setReceiverAddress(stream.readMACAddress());
            frame = ackFrame;
            break;
        }
        case 0xB4: // ST_RTS
        {
            auto rtsFrame = std::make_shared<Ieee80211RTSFrame>();
            rtsFrame->setType(ST_RTS);
            rtsFrame->setToDS(false);
            rtsFrame->setFromDS(false);
            rtsFrame->setRetry(false);
            rtsFrame->setMoreFragments(false);
            rtsFrame->setDuration(SimTime(stream.readUint16Be(), SIMTIME_US));    //i_dur
            rtsFrame->setReceiverAddress(stream.readMACAddress());
            rtsFrame->setTransmitterAddress(stream.readMACAddress());
            frame = rtsFrame;
            break;
        }
        case 0xC4: // ST_CTS
        {
            auto ctsFrame = std::make_shared<Ieee80211CTSFrame>();
            ctsFrame->setType(ST_CTS);
            ctsFrame->setToDS(false);
            ctsFrame->setFromDS(false);
            ctsFrame->setRetry(false);
            ctsFrame->setMoreFragments(false);
            ctsFrame->setDuration(SimTime(stream.readUint16Be(),SIMTIME_US));    //i_dur
            ctsFrame->setReceiverAddress(stream.readMACAddress());
            frame = ctsFrame;
            break;
        }
        case 0x08: // ST_DATA
        case 0x88: // ST_DATA_WITH_QOS
        {
            auto dataFrame = std::make_shared<Ieee80211DataFrame>();
            parseDataOrMgmtFrame(stream, dataFrame, type == 0x08 ? ST_DATA : ST_DATA_WITH_QOS, fc_1);
            frame = dataFrame;
            break;
        }

        case 0xB0: // ST_AUTHENTICATION
        {
            auto pkt = std::make_shared<Ieee80211ManagementHeader>();
            parseDataOrMgmtFrame(stream, pkt, ST_AUTHENTICATION, fc_1);
            auto body = std::make_shared<Ieee80211AuthenticationFrameBody>();
            stream.readUint16Be();
            body->setSequenceNumber(stream.readUint16Be());
            body->setStatusCode(stream.readUint16Be());

            // REVIVE: pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0xC0: //ST_ST_DEAUTHENTICATION
        {
            auto pkt = std::make_shared<Ieee80211ManagementHeader>();
            parseDataOrMgmtFrame(stream, pkt, ST_DEAUTHENTICATION, fc_1);
            auto body = std::make_shared<Ieee80211DeauthenticationFrameBody>();

            body->setReasonCode(stream.readUint16Be());

            // REVIVE: pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0xA0: // ST_DISASSOCIATION
        {
            auto pkt = std::make_shared<Ieee80211ManagementHeader>();
            parseDataOrMgmtFrame(stream, pkt, ST_DISASSOCIATION, fc_1);
            auto body = std::make_shared<Ieee80211DisassociationFrameBody>();

            body->setReasonCode(stream.readUint16Be());

            // REVIVE: pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0x40: // ST_PROBEREQUEST
        {
            auto pkt = std::make_shared<Ieee80211ManagementHeader>();
            parseDataOrMgmtFrame(stream, pkt, ST_PROBEREQUEST, fc_1);
            auto body = std::make_shared<Ieee80211ProbeRequestFrameBody>();

            char SSID[256];
            stream.readByte();
            unsigned int length = stream.readByte();
            stream.readBytes((uint8_t *)SSID, byte(length));
            SSID[length] = '\0';
            body->setSSID(SSID);

            Ieee80211SupportedRatesElement supRat;
            stream.readByte();
            supRat.numRates = stream.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
            body->setSupportedRates(supRat);

            // REVIVE: pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0x00: // ST_ASSOCIATIONREQUEST
        {
            auto pkt = std::make_shared<Ieee80211ManagementHeader>();
            parseDataOrMgmtFrame(stream, pkt, ST_ASSOCIATIONREQUEST, fc_1);
            auto body = std::make_shared<Ieee80211AssociationRequestFrameBody>();

            char SSID[256];
            stream.readByte();
            unsigned int length = stream.readByte();
            stream.readBytes((uint8_t *)SSID, byte(length));
            SSID[length] = '\0';
            body->setSSID(SSID);

            Ieee80211SupportedRatesElement supRat;
            stream.readByte();
            supRat.numRates = stream.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
            body->setSupportedRates(supRat);

            // REVIVE: pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0x02: // ST_REASSOCIATIONREQUEST
        {
            auto pkt = std::make_shared<Ieee80211ManagementHeader>();
            parseDataOrMgmtFrame(stream, pkt, ST_REASSOCIATIONREQUEST, fc_1);
            auto body = std::make_shared<Ieee80211ReassociationRequestFrameBody>();
            stream.readUint16Be();
            stream.readUint16Be();

            body->setCurrentAP(stream.readMACAddress());

            char SSID[256];
            stream.readByte();
            unsigned int length = stream.readByte();
            stream.readBytes((uint8_t *)SSID, byte(length));
            SSID[length] = '\0';
            body->setSSID(SSID);

            Ieee80211SupportedRatesElement supRat;
            stream.readByte();
            supRat.numRates = stream.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
            body->setSupportedRates(supRat);

            // REVIVE: pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0x01: // ST_ASSOCIATIONRESPONSE
        {
            auto pkt = std::make_shared<Ieee80211ManagementHeader>();
            parseDataOrMgmtFrame(stream, pkt, ST_ASSOCIATIONRESPONSE, fc_1);
            auto body = std::make_shared<Ieee80211AssociationResponseFrameBody>();
            stream.readUint16Be();
            body->setStatusCode(stream.readUint16Be());
            body->setAid(stream.readUint16Be());

            Ieee80211SupportedRatesElement supRat;
            stream.readByte();
            supRat.numRates = stream.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
            body->setSupportedRates(supRat);

            // REVIVE: pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0x03: // ST_REASSOCIATIONRESPONSE
        {
            auto pkt = std::make_shared<Ieee80211ManagementHeader>();
            parseDataOrMgmtFrame(stream, pkt, ST_REASSOCIATIONRESPONSE, fc_1);
            auto body = std::make_shared<Ieee80211ReassociationResponseFrameBody>();
            stream.readUint16Be();
            body->setStatusCode(stream.readUint16Be());
            body->setAid(stream.readUint16Be());

            Ieee80211SupportedRatesElement supRat;
            stream.readByte();
            supRat.numRates = stream.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
            body->setSupportedRates(supRat);

            // REVIVE: pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0x80: // ST_BEACON
        {
            auto pkt = std::make_shared<Ieee80211ManagementHeader>();
            parseDataOrMgmtFrame(stream, pkt, ST_BEACON, fc_1);
            auto body = std::make_shared<Ieee80211BeaconFrameBody>();

            simtime_t timetstamp;
            timetstamp.setRaw(stream.readUint64Be()); // TODO: store timestamp

            body->setBeaconInterval(SimTime((int64_t)stream.readUint16Be()*1024, SIMTIME_US));
            stream.readUint16Be();     // Capability

            char SSID[256];
            stream.readByte();
            unsigned int length = stream.readByte();
            stream.readBytes((uint8_t *)SSID, byte(length));
            SSID[length] = '\0';
            body->setSSID(SSID);

            Ieee80211SupportedRatesElement supRat;
            stream.readByte();
            supRat.numRates = stream.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
            body->setSupportedRates(supRat);

            // REVIVE: pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0x50: // ST_PROBERESPONSE
        {
            auto pkt = std::make_shared<Ieee80211ManagementHeader>();
            parseDataOrMgmtFrame(stream, pkt, ST_PROBERESPONSE, fc_1);
            auto body = std::make_shared<Ieee80211ProbeResponseFrameBody>();

            simtime_t timestamp;
            timestamp.setRaw(stream.readUint64Be()); // TODO: store timestamp

            body->setBeaconInterval(SimTime((int64_t)stream.readUint16Be() * 1024, SIMTIME_US));
            stream.readUint16Be();

            char SSID[256];
            stream.readByte();
            unsigned int length = stream.readByte();
            stream.readBytes((uint8_t *)SSID, byte(length));
            SSID[length] = '\0';
            body->setSSID(SSID);

            Ieee80211SupportedRatesElement supRat;
            stream.readByte();
            supRat.numRates = stream.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(stream.readByte() & 0x7F) * 0.5;
            body->setSupportedRates(supRat);

            // REVIVE: pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0xD0: // type = ST_ACTION
        {
            // Ieee80211ActionFrame
            return nullptr;
        }

        default:
        {
            EV_ERROR << "Cannot deserialize Ieee80211 packet: type " << type << " not supported.";
            frame->markImproperlyRepresented();
            return frame;
        }
    }
    return frame;
}

void Ieee80211MacTrailerSerializer::serialize(MemoryOutputStream& stream, const Ptr<Chunk>& chunk) const
{
    const auto& macTrailer = std::dynamic_pointer_cast<Ieee80211MacTrailer>(chunk);
    auto fcsMode = macTrailer->getFcsMode();
//    if (fcsMode != CRC_COMPUTED)
//        throw cRuntimeError("Cannot serialize Ieee80211FcsTrailer without properly computed FCS, try changing the value of the fcsMode parameter (e.g. in the Ieee80211Mac module)");
    stream.writeUint32Be(macTrailer->getFcs());
}

Ptr<Chunk> Ieee80211MacTrailerSerializer::deserialize(MemoryInputStream& stream) const
{
    auto macTrailer = std::make_shared<Ieee80211MacTrailer>();
    auto fcs = stream.readUint32Be();
    macTrailer->setFcs(fcs);
    macTrailer->setFcsMode(CRC_COMPUTED);
    return macTrailer;
}

} // namespace serializer

} // namespace inet

