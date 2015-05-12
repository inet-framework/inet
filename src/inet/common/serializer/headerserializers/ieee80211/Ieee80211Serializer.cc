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

#include "inet/common/serializer/SerializerUtil.h"
#include "inet/common/serializer/headerserializers/ieee80211/Ieee80211Serializer.h"

#include "inet/common/RawPacket.h"
#include "inet/common/serializer/headerserializers/ieee80211/headers/ieee80211.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h"

#include "inet/common/serializer/headerserializers/EthernetCRC.h"


namespace inet {
namespace serializer {

using namespace ieee80211;

Register_Serializer(Ieee80211Frame, LINKTYPE, LINKTYPE_IEEE802_11, Ieee80211Serializer);

namespace {
    class SnapHeaderTest {
      public:
        SnapHeaderTest() {
            ASSERT(IEEE80211_ADDR_LEN == MAC_ADDRESS_SIZE);
        }
    };

    SnapHeaderTest snapHeaderTest;
}

void Ieee80211Serializer::serialize(const cPacket *pkt, Buffer &b, Context& c)
{
    if (dynamic_cast<const Ieee80211ACKFrame *>(pkt))
    {
        const Ieee80211ACKFrame *ackFrame = static_cast<const Ieee80211ACKFrame *>(pkt);
        b.writeByte(0xD4);
        b.writeByte(0);
        b.writeUint16(ackFrame->getDuration().inUnit(SIMTIME_MS));
        b.writeMACAddress(ackFrame->getReceiverAddress());
    }
    else if (dynamic_cast<const Ieee80211RTSFrame *>(pkt))
    {
        const Ieee80211RTSFrame *rtsFrame = static_cast<const Ieee80211RTSFrame *>(pkt);
        b.writeByte(0xB4);
        b.writeByte(0);
        b.writeUint16(rtsFrame->getDuration().inUnit(SIMTIME_MS));
        b.writeMACAddress(rtsFrame->getReceiverAddress());
        b.writeMACAddress(rtsFrame->getTransmitterAddress());
    }
    else if (dynamic_cast<const Ieee80211CTSFrame *>(pkt))
    {
        const Ieee80211CTSFrame *ctsFrame = static_cast<const Ieee80211CTSFrame *>(pkt);
        b.writeByte(0xC4);
        b.writeByte(0);
        b.writeUint16(ctsFrame->getDuration().inUnit(SIMTIME_MS));
        b.writeMACAddress(ctsFrame->getReceiverAddress());
    }
    else if (dynamic_cast<const Ieee80211DataOrMgmtFrame *>(pkt))
    {
        const Ieee80211DataOrMgmtFrame *dataOrMgmtFrame = static_cast<const Ieee80211DataOrMgmtFrame *>(pkt);

        b.writeByte(0x08);  //without Qos=0x08, with Qos=0x88
        uint8_t fc1;
        // TODO: Order, Protected Frame, MoreData, Power Mgt
        fc1 = (dataOrMgmtFrame->getRetry() ? 8 : 0)
                | (dataOrMgmtFrame->getMoreFragments() ? 4 : 0)
                | (dataOrMgmtFrame->getFromDS() ? 2 : 0)
                | (dataOrMgmtFrame->getToDS() ? 1 : 0);
        b.writeByte(fc1);
        b.writeUint16(dataOrMgmtFrame->getDuration().inUnit(SIMTIME_MS));
        b.writeMACAddress(dataOrMgmtFrame->getReceiverAddress());
        b.writeMACAddress(dataOrMgmtFrame->getTransmitterAddress());
        b.writeMACAddress(dataOrMgmtFrame->getAddress3());
        b.writeUint16(dataOrMgmtFrame->getSequenceNumber() << 4
                | dataOrMgmtFrame->getFragmentNumber());


        if (dynamic_cast<const Ieee80211DataFrameWithSNAP *>(pkt))
        {
            const Ieee80211DataFrameWithSNAP *dataFrame = static_cast<const Ieee80211DataFrameWithSNAP *>(pkt);
            if (dataFrame->getFromDS() && dataFrame->getToDS())
            {
                b.writeMACAddress(dataFrame->getAddress4());
            }
            if (dataFrame->getType() == ST_DATA_WITH_QOS) {
                b.writeUint16(dataFrame->getQos());
            }

            // snap header:
            b.writeByte(0xAA);    // snap_hdr.dsap
            b.writeByte(0xAA);    // snap_hdr.ssap
            b.writeByte(0x03);    // snap_hdr.ctrl
            b.writeNBytes(3, "\0\0\0");   // snap_hdr.oui
            b.writeUint16(dataFrame->getEtherType());  // snap_hdr.ethertype

            const cPacket *encapPacket = dataFrame->getEncapsulatedPacket();
            SerializerBase::lookupAndSerialize(encapPacket, b, c, ETHERTYPE, dataFrame->getEtherType(), b.getRemainingSize(4));   // 4 byte for store crc at end of packet
        }
        else if (dynamic_cast<const Ieee80211AuthenticationFrame *>(pkt))
        {
            //type = ST_AUTHENTICATION;
            const Ieee80211AuthenticationFrame *Frame = static_cast<const Ieee80211AuthenticationFrame *>(pkt);
            // 1    Authentication algorithm number
            b.writeUint16(0);
            // 2    Authentication transaction sequence number
            b.writeUint16(Frame->getBody().getSequenceNumber());
            // 3    Status code                                 The status code information is reserved in certain Authentication frames as defined in Table 7-17.
            b.writeUint16(Frame->getBody().getStatusCode());
            // 4    Challenge text                              The challenge text information is present only in certain Authentication frames as defined in Table 7-17.
            // Last Vendor Specific                             One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
        }
        else if (dynamic_cast<const Ieee80211DeauthenticationFrame *>(pkt))
        {
            //type = ST_DEAUTHENTICATION;
            const Ieee80211DeauthenticationFrame *Frame = static_cast<const Ieee80211DeauthenticationFrame *>(pkt);
            b.writeUint16(Frame->getBody().getReasonCode());
        }
        else if (dynamic_cast<const Ieee80211DisassociationFrame *>(pkt))
        {
            //type = ST_DISASSOCIATION;
            const Ieee80211DisassociationFrame *Frame = static_cast<const Ieee80211DisassociationFrame *>(pkt);
            b.writeUint16(Frame->getBody().getReasonCode());
        }
        else if (dynamic_cast<const Ieee80211ProbeRequestFrame *>(pkt))
        {
            //type = ST_PROBEREQUEST;
            const Ieee80211ProbeRequestFrame *Frame = static_cast<const Ieee80211ProbeRequestFrame *>(pkt);
            // 1    SSID
            const char *SSID = Frame->getBody().getSSID();
            unsigned int length = strlen(SSID);
            b.writeByte(0);    //FIXME dummy, what is it?
            b.writeByte(length);
            b.writeNBytes(length, SSID);
            // 2    Supported rates
            const Ieee80211SupportedRatesElement& supportedRates = Frame->getBody().getSupportedRates();
            b.writeByte(1);
            b.writeByte(supportedRates.numRates);
            for (int i = 0; i < supportedRates.numRates; i++)
            {
                uint8_t rate = ceil(supportedRates.rate[i]/0.5);
                // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
                b.writeByte(rate);
            }
            // 3    Request information         May be included if dot11MultiDomainCapabilityEnabled is true.
            // 4    Extended Supported Rates    The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
            // Last Vendor Specific             One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
        }
        else if (dynamic_cast<const Ieee80211AssociationRequestFrame *>(pkt))
        {
            //type = ST_ASSOCIATIONREQUEST;
            const Ieee80211AssociationRequestFrame *Frame = static_cast<const Ieee80211AssociationRequestFrame *>(pkt);
            // 1    Capability
            b.writeUint16(0);    //FIXME
            // 2    Listen interval
            b.writeUint16(0);    //FIXME
            // 3    SSID
            const char *SSID = Frame->getBody().getSSID();
            unsigned int length = strlen(SSID);
            b.writeByte(0);    //FIXME dummy, what is it?
            b.writeByte(length);
            b.writeNBytes(length, SSID);
            // 4    Supported rates
            const Ieee80211SupportedRatesElement& supportedRates = Frame->getBody().getSupportedRates();
            b.writeByte(1);
            b.writeByte(supportedRates.numRates);
            for (int i = 0; i < supportedRates.numRates; i++) {
                uint8_t rate = ceil(supportedRates.rate[i]/0.5);
                // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
                b.writeByte(rate);
            }
            // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
            // 6    Power Capability           The Power Capability element shall be present if dot11SpectrumManagementRequired is true.
            // 7    Supported Channel          The Supported Channels element shall be present if dot11SpectrumManagementRequired is true.
            // 8    RSN                        The RSN information element is only present within Association Request frames generated by STAs that have dot11RSNAEnabled set to TRUE.
            // 9    QoS Capability             The QoS Capability element is present when dot11QosOption- Implemented is true.
            // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
        }
        else if (dynamic_cast<const Ieee80211ReassociationRequestFrame *>(pkt))
        {
            //type = ST_REASSOCIATIONREQUEST;
            const Ieee80211ReassociationRequestFrame *Frame = dynamic_cast<const Ieee80211ReassociationRequestFrame *>(pkt);
            // 1    Capability
            b.writeUint16(0);    //FIXME
            // 2    Listen interval
            b.writeUint16(0);    //FIXME
            // 3    Current AP address
            b.writeMACAddress(Frame->getBody().getCurrentAP());
            // 4    SSID
            const char *SSID = Frame->getBody().getSSID();
            unsigned int length = strlen(SSID);
            //FIXME buffer.writeByte(buf + packetLength, ???);
            b.writeByte(0);    //FIXME
            b.writeByte(length);
            b.writeNBytes(length, SSID);
            // 5    Supported rates
            const Ieee80211SupportedRatesElement& supportedRates = Frame->getBody().getSupportedRates();
            b.writeByte(1);
            b.writeByte(supportedRates.numRates);
            for (int i = 0; i < supportedRates.numRates; i++)
            {
                uint8_t rate = ceil(supportedRates.rate[i]/0.5);
                // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
                b.writeByte(rate);
            }
            // 6    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
            // 7    Power Capability           The Power Capability element shall be present if dot11SpectrumManagementRequired is true.
            // 8    Supported Channels         The Supported Channels element shall be present if dot11SpectrumManagementRequired is true.
            // 9    RSN                        The RSN information element is only present within Reassociation Request frames generated by STAs that have dot11RSNAEnabled set to TRUE.
            // 10   QoS Capability             The QoS Capability element is present when dot11QosOption- Implemented is true.
            // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
        }
        else if (dynamic_cast<const Ieee80211AssociationResponseFrame *>(pkt))
        {
            //type = ST_ASSOCIATIONRESPONSE;
            const Ieee80211AssociationResponseFrame *Frame = static_cast<const Ieee80211AssociationResponseFrame *>(pkt);
            // 1    Capability
            b.writeUint16(0);    //FIXME
            // 2    Status code
            b.writeUint16(Frame->getBody().getStatusCode());
            // 3    AID
            b.writeUint16(Frame->getBody().getAid());
            // 4    Supported rates
            b.writeByte(1);
            b.writeByte(Frame->getBody().getSupportedRates().numRates);
            for (int i = 0; i < Frame->getBody().getSupportedRates().numRates; i++)
            {
                uint8_t rate = ceil(Frame->getBody().getSupportedRates().rate[i]/0.5);
                // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
                b.writeByte(rate);
            }
            // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
            // 6    EDCA Parameter Set
            // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
        }
        else if (dynamic_cast<const Ieee80211ReassociationResponseFrame *>(pkt))
        {
            //type = ST_REASSOCIATIONRESPONSE;
            const Ieee80211ReassociationResponseFrame *Frame = dynamic_cast<const Ieee80211ReassociationResponseFrame *>(pkt);
            // 1    Capability
            b.writeUint16(0);    //FIXME
            // 2    Status code
            b.writeUint16(Frame->getBody().getStatusCode());
            // 3    AID
            b.writeUint16(Frame->getBody().getAid());
            // 4    Supported rates
            b.writeByte(1);
            b.writeByte(Frame->getBody().getSupportedRates().numRates);
            for (int i = 0; i < Frame->getBody().getSupportedRates().numRates; i++)
            {
                uint8_t rate = ceil(Frame->getBody().getSupportedRates().rate[i]/0.5);
                // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
                b.writeByte(rate);
            }
            // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
            // 6    EDCA Parameter Set
            // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
        }
        else if (dynamic_cast<const Ieee80211BeaconFrame *>(pkt))
        {
            //type = ST_BEACON;
            const Ieee80211BeaconFrame *Frame = static_cast<const Ieee80211BeaconFrame *>(pkt);
            // 1    Timestamp
            b.writeUint64(Frame->getTimestamp().raw());   //FIXME
            // 2    Beacon interval
            b.writeUint16((uint16_t)(Frame->getBody().getBeaconInterval().inUnit(SIMTIME_US)/1024));
            // 3    Capability
            b.writeUint16(0);    //FIXME set  capability
            // 4    Service Set Identifier (SSID)
            const char *SSID = Frame->getBody().getSSID();
            unsigned int length = strlen(SSID);
            b.writeByte(0);    //FIXME
            b.writeByte(length);
            b.writeNBytes(length, SSID);
            // 5    Supported rates
            b.writeByte(1);
            b.writeByte(Frame->getBody().getSupportedRates().numRates);
            for (int i = 0; i < Frame->getBody().getSupportedRates().numRates; i++)
            {
                uint8_t rate = ceil(Frame->getBody().getSupportedRates().rate[i]/0.5);
                // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
                b.writeByte(rate);
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
        else if (dynamic_cast<const Ieee80211ProbeResponseFrame *>(pkt))
        {
            //type = ST_PROBERESPONSE;
            const Ieee80211ProbeResponseFrame *Frame = static_cast<const Ieee80211ProbeResponseFrame *>(pkt);
            // 1      Timestamp
            b.writeUint64(Frame->getTimestamp().raw());   //FIXME
            // 2      Beacon interval
            b.writeUint16((uint16_t)(Frame->getBody().getBeaconInterval().inUnit(SIMTIME_US)/1024));
            // 3      Capability
            b.writeUint16(0);    //FIXME
            // 4      SSID
            const char *SSID = Frame->getBody().getSSID();
            unsigned int length = strlen(SSID);
            b.writeByte(0);    //FIXME
            b.writeByte(length);
            b.writeNBytes(length, SSID);
            // 5      Supported rates
            b.writeByte(1);
            b.writeByte(Frame->getBody().getSupportedRates().numRates);
            for (int i = 0; i < Frame->getBody().getSupportedRates().numRates; i++)
            {
                uint8_t rate = ceil(Frame->getBody().getSupportedRates().rate[i]/0.5);
                // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
                b.writeByte(rate);
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

        else if (dynamic_cast<const Ieee80211ActionFrame *>(pkt))
        {
            //type = ST_ACTION;
            // 1    Action
            // Last One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
        }
    }
    else
        throw cRuntimeError("Ieee80211Serializer: cannot serialize the frame");

    // CRC
    b.writeUint32(ethernetCRC(b._getBuf(), b.getPos()));
}

void Ieee80211Serializer::parseDataOrMgmtFrame(const Buffer &b, Ieee80211DataOrMgmtFrame *Frame, short type)
{
    Frame->setType(type);
    b.seek(0);
    b.readByte();   // i_fc[0]
    uint8_t fc1 = b.readByte();   // i_fc[1]
    Frame->setToDS(fc1 & 0x1);
    Frame->setFromDS(fc1 & 0x2);
    Frame->setMoreFragments(fc1 & 0x4);
    Frame->setRetry(fc1 & 0x8);
    Frame->setDuration(SimTime(b.readUint16() / 1000.0)); // i_dur
    Frame->setReceiverAddress(b.readMACAddress());
    Frame->setTransmitterAddress(b.readMACAddress());
    Frame->setAddress3(b.readMACAddress());
    uint16_t seq = b.readUint16();   // i_seq
    Frame->setSequenceNumber(seq >> 4);
    Frame->setFragmentNumber(seq & 0x0F);

    if ((type == ST_DATA || type == ST_DATA_WITH_QOS) && Frame->getFromDS() && Frame->getToDS())
    {
        check_and_cast<Ieee80211DataFrame*>(Frame)->setAddress4(b.readMACAddress());
    }
    if (type == ST_DATA_WITH_QOS) {
        check_and_cast<Ieee80211DataFrame*>(Frame)->setQos(b.readUint16());
    }
}

cPacket* Ieee80211Serializer::deserialize(const Buffer &b, Context& c)
{
    ASSERT(b.getPos() == 0);
    cPacket *frame = nullptr;

    uint8_t type = b.readByte();
    uint8_t fc_1 = b.readByte();  (void)fc_1; // fc_1
    switch(type)
    {
        case 0xD4: // ST_ACK    //TODO ((ST_ACK & 0x0F) << 4) | ((ST_ACK & 0x30) >> 2)
        {
            Ieee80211ACKFrame *ackFrame = new Ieee80211ACKFrame();
            ackFrame->setType(ST_ACK);
            ackFrame->setToDS(false);
            ackFrame->setFromDS(false);
            ackFrame->setRetry(false);
            ackFrame->setMoreFragments(false);
            ackFrame->setDuration(SimTime((double)b.readUint16()/1000.0));    //i_dur
            ackFrame->setReceiverAddress(b.readMACAddress());
            frame = ackFrame;
            break;
        }
        case 0xB4: // ST_RTS
        {
            Ieee80211RTSFrame *rtsFrame = new Ieee80211RTSFrame();
            rtsFrame->setType(ST_RTS);
            rtsFrame->setToDS(false);
            rtsFrame->setFromDS(false);
            rtsFrame->setRetry(false);
            rtsFrame->setMoreFragments(false);
            rtsFrame->setDuration(SimTime(b.readUint16(), SIMTIME_MS));    //i_dur
            rtsFrame->setReceiverAddress(b.readMACAddress());
            rtsFrame->setTransmitterAddress(b.readMACAddress());
            frame = rtsFrame;
            break;
        }
        case 0xC4: // ST_CTS
        {
            Ieee80211CTSFrame *ctsFrame = new Ieee80211CTSFrame();
            ctsFrame->setType(ST_CTS);
            ctsFrame->setToDS(false);
            ctsFrame->setFromDS(false);
            ctsFrame->setRetry(false);
            ctsFrame->setMoreFragments(false);
            ctsFrame->setDuration(SimTime(b.readUint16(),SIMTIME_MS));    //i_dur
            ctsFrame->setReceiverAddress(b.readMACAddress());
            frame = ctsFrame;
            break;
        }
        case 0x08: // ST_DATA
        case 0x88: // ST_DATA_WITH_QOS
        {
            Ieee80211DataFrameWithSNAP *dataFrame = new Ieee80211DataFrameWithSNAP();
            parseDataOrMgmtFrame(b, dataFrame, type == 0x08 ? ST_DATA : ST_DATA_WITH_QOS);

            // snap_header:
            b.accessNBytes(6);
            dataFrame->setEtherType(b.readUint16());    // ethertype

            cPacket *encapPacket = SerializerBase::lookupAndDeserialize(b, c, ETHERTYPE, dataFrame->getEtherType(), b.getRemainingSize(4));
            if (encapPacket) {
                dataFrame->encapsulate(encapPacket);
                dataFrame->setName(encapPacket->getName());
            }
            frame = dataFrame;
            break;
        }

        case 0xB0: // ST_AUTHENTICATION
        {
            Ieee80211AuthenticationFrame *pkt = new Ieee80211AuthenticationFrame();
            parseDataOrMgmtFrame(b, pkt, ST_AUTHENTICATION);
            Ieee80211AuthenticationFrameBody body;
            b.readUint16();
            body.setSequenceNumber(b.readUint16());
            body.setStatusCode(b.readUint16());

            pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0xC0: //ST_ST_DEAUTHENTICATION
        {
            Ieee80211DeauthenticationFrame *pkt = new Ieee80211DeauthenticationFrame();
            parseDataOrMgmtFrame(b, pkt, ST_DEAUTHENTICATION);
            Ieee80211DeauthenticationFrameBody body;

            body.setReasonCode(b.readUint16());

            pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0xA0: // ST_DISASSOCIATION
        {
            Ieee80211DisassociationFrame *pkt = new Ieee80211DisassociationFrame();
            parseDataOrMgmtFrame(b, pkt, ST_DISASSOCIATION);
            Ieee80211DisassociationFrameBody body;

            body.setReasonCode(b.readUint16());

            pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0x40: // ST_PROBEREQUEST
        {
            Ieee80211ProbeRequestFrame *pkt = new Ieee80211ProbeRequestFrame();
            parseDataOrMgmtFrame(b, pkt, ST_PROBEREQUEST);
            Ieee80211ProbeRequestFrameBody body;

            char SSID[256];
            b.readByte();
            unsigned int length = b.readByte();
            b.readNBytes(length, SSID);
            SSID[length] = '\0';
            body.setSSID(SSID);

            Ieee80211SupportedRatesElement supRat;
            b.readByte();
            supRat.numRates = b.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(b.readByte() & 0x7F) * 0.5;
            body.setSupportedRates(supRat);

            pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0x00: // ST_ASSOCIATIONREQUEST
        {
            Ieee80211AssociationRequestFrame *pkt = new Ieee80211AssociationRequestFrame;
            parseDataOrMgmtFrame(b, pkt, ST_ASSOCIATIONREQUEST);
            Ieee80211AssociationRequestFrameBody body;

            char SSID[256];
            b.readByte();
            unsigned int length = b.readByte();
            b.readNBytes(length, SSID);
            SSID[length] = '\0';
            body.setSSID(SSID);

            Ieee80211SupportedRatesElement supRat;
            b.readByte();
            supRat.numRates = b.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(b.readByte() & 0x7F) * 0.5;
            body.setSupportedRates(supRat);

            pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0x02: // ST_REASSOCIATIONREQUEST
        {
            Ieee80211ReassociationRequestFrame *pkt = new Ieee80211ReassociationRequestFrame;
            parseDataOrMgmtFrame(b, pkt, ST_REASSOCIATIONREQUEST);
            Ieee80211ReassociationRequestFrameBody body;
            b.readUint16();
            b.readUint16();

            body.setCurrentAP(b.readMACAddress());

            char SSID[256];
            b.readByte();
            unsigned int length = b.readByte();
            b.readNBytes(length, SSID);
            SSID[length] = '\0';
            body.setSSID(SSID);

            Ieee80211SupportedRatesElement supRat;
            b.readByte();
            supRat.numRates = b.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(b.readByte() & 0x7F) * 0.5;
            body.setSupportedRates(supRat);

            pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0x01: // ST_ASSOCIATIONRESPONSE
        {
            Ieee80211AssociationResponseFrame *pkt = new Ieee80211AssociationResponseFrame;
            parseDataOrMgmtFrame(b, pkt, ST_ASSOCIATIONRESPONSE);
            Ieee80211AssociationResponseFrameBody body;
            b.readUint16();
            body.setStatusCode(b.readUint16());
            body.setAid(b.readUint16());

            Ieee80211SupportedRatesElement supRat;
            b.readByte();
            supRat.numRates = b.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(b.readByte() & 0x7F) * 0.5;
            body.setSupportedRates(supRat);

            pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0x03: // ST_REASSOCIATIONRESPONSE
        {
            Ieee80211ReassociationResponseFrame *pkt = new Ieee80211ReassociationResponseFrame;
            parseDataOrMgmtFrame(b, pkt, ST_REASSOCIATIONRESPONSE);
            Ieee80211ReassociationResponseFrameBody body;
            b.readUint16();
            body.setStatusCode(b.readUint16());
            body.setAid(b.readUint16());

            Ieee80211SupportedRatesElement supRat;
            b.readByte();
            supRat.numRates = b.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(b.readByte() & 0x7F) * 0.5;
            body.setSupportedRates(supRat);

            pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0x80: // ST_BEACON
        {
            Ieee80211BeaconFrame *pkt = new Ieee80211BeaconFrame();
            parseDataOrMgmtFrame(b, pkt, ST_BEACON);
            Ieee80211BeaconFrameBody body;

            simtime_t t; t.setRaw(b.readUint64()); pkt->setTimestamp(t);  //timestamp   //FIXME

            body.setBeaconInterval(SimTime((int64_t)b.readUint16()*1024, SIMTIME_US));
            b.readUint16();     // Capability

            char SSID[256];
            b.readByte();
            unsigned int length = b.readByte();
            b.readNBytes(length, SSID);
            SSID[length] = '\0';
            body.setSSID(SSID);

            Ieee80211SupportedRatesElement supRat;
            b.readByte();
            supRat.numRates = b.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(b.readByte() & 0x7F) * 0.5;
            body.setSupportedRates(supRat);

            pkt->setBody(body);
            frame = pkt;
            break;
        }

        case 0x50: // ST_PROBERESPONSE
        {
            Ieee80211ProbeResponseFrame *pkt = new Ieee80211ProbeResponseFrame();
            parseDataOrMgmtFrame(b, pkt, ST_PROBERESPONSE);
            Ieee80211ProbeResponseFrameBody body;

            simtime_t t; t.setRaw(b.readUint64()); pkt->setTimestamp(t);  //timestamp   //FIXME

            body.setBeaconInterval(SimTime((int64_t)b.readUint16() * 1024, SIMTIME_US));
            b.readUint16();

            char SSID[256];
            b.readByte();
            unsigned int length = b.readByte();
            b.readNBytes(length, SSID);
            SSID[length] = '\0';
            body.setSSID(SSID);

            Ieee80211SupportedRatesElement supRat;
            b.readByte();
            supRat.numRates = b.readByte();
            for (int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(b.readByte() & 0x7F) * 0.5;
            body.setSupportedRates(supRat);

            pkt->setBody(body);
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
            b.setError();
            return nullptr;
        }
    }
    uint32_t calculatedCrc = ethernetCRC(b._getBuf(), b.getPos());
    uint32_t receivedCrc = b.readUint32();
    EV_DEBUG << "Calculated CRC: " << calculatedCrc << ", received CRC: " << receivedCrc << endl;
    if (receivedCrc != calculatedCrc)
        frame->setBitError(true);

    // TODO: don't set this directly, it should be computed above separately in each case
    if (frame->getByteLength() != b.getPos()) {
        throw cRuntimeError("ieee802.11 deserializer: packet length error: generated=%i v.s. read=%i", (int)frame->getByteLength(), b.getPos());
    }
    //frame->setByteLength(b.getPos());
    return frame;
}

} // namespace serializer

} // namespace inet
