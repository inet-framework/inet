//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "inet/common/serializer/headerserializers/ieee80211/Ieee80211Serializer.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h"
#include "inet/common/serializer/headerserializers/ieee80211/headers/ieee80211.h"

namespace INETFw // load headers into a namespace, to avoid conflicts with platform definitions of the same stuff
{
#include "inet/common/serializer/headers/bsdint.h"
#include "inet/common/serializer/headers/in.h"
#include "inet/common/serializer/headers/in_systm.h"
#include "inet/common/serializer/headerserializers/headers/ethernet.h"
#include "inet/common/serializer/headerserializers/arp/headers/arp.h"
};

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>  // htonl, ntohl, ...
#endif

#ifdef WITH_IPv4
#include "inet/common/serializer/ipv4/IPv4Serializer.h"
#endif

#ifdef WITH_IPv6
#include "inet/common/serializer/ipv6/IPv6Serializer.h"
#endif

#include "inet/common/serializer/headerserializers/arp/ARPSerializer.h"

#include "inet/common/serializer/headerserializers/EthernetCRC.h"

#include "inet/common/serializer/headerserializers/SerializerUtil.h"
#include <ostream>

namespace inet {
namespace serializer {

using namespace INETFw;
using namespace ieee80211;

int Ieee80211Serializer::serialize(Ieee80211Frame *pkt, unsigned char *buf, unsigned int bufsize)
{
    unsigned int packetLength = 0;

    if (NULL != dynamic_cast<Ieee80211ACKFrame *>(pkt))
    {
        Ieee80211ACKFrame *ackFrame = dynamic_cast<Ieee80211ACKFrame *>(pkt);
        struct ieee80211_frame_ack *frame = (struct ieee80211_frame_ack *) (buf);
        frame->i_fc[0] = 0xD4;
        frame->i_fc[1] = 0;
        frame->i_dur = (int)(ackFrame->getDuration().dbl()*1000);
        ackFrame->getReceiverAddress().getAddressBytes(frame->i_ra);

        packetLength = 4 + IEEE80211_ADDR_LEN;
    }
    else if (NULL != dynamic_cast<Ieee80211RTSFrame *>(pkt))
    {
        Ieee80211RTSFrame *rtsFrame = dynamic_cast<Ieee80211RTSFrame *>(pkt);
        struct ieee80211_frame_rts *frame = (struct ieee80211_frame_rts *) (buf);
        frame->i_fc[0] = 0xB4;
        frame->i_fc[1] = 0;
        frame->i_dur = (int)(rtsFrame->getDuration().dbl()*1000);
        rtsFrame->getReceiverAddress().getAddressBytes(frame->i_ra);
        rtsFrame->getTransmitterAddress().getAddressBytes(frame->i_ta);
        packetLength = 4 + 2*IEEE80211_ADDR_LEN;
    }

    else if (NULL != dynamic_cast<Ieee80211CTSFrame *>(pkt))
    {
        Ieee80211CTSFrame *ctsFrame = dynamic_cast<Ieee80211CTSFrame *>(pkt);
        struct ieee80211_frame_cts *frame = (struct ieee80211_frame_cts *) (buf);
        frame->i_fc[0] = 0xC4;
        frame->i_fc[1] = 0;
        frame->i_dur = (int)(ctsFrame->getDuration().dbl()*1000);
        ctsFrame->getReceiverAddress().getAddressBytes(frame->i_ra);
        packetLength = 4 + IEEE80211_ADDR_LEN;
    }
    else if (NULL != dynamic_cast<Ieee80211DataOrMgmtFrame *>(pkt))
    {
        Ieee80211DataOrMgmtFrame *dataOrMgmtFrame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(pkt);

        struct ieee80211_frame *frame = (struct ieee80211_frame *) (buf);
        frame->i_fc[0] = 0x8;
        frame->i_fc[1] = 0; // TODO: Order, Protected Frame, MoreData, Power Mgt
        frame->i_fc[1] |= dataOrMgmtFrame->getRetry();
        frame->i_fc[1] <<= 1;
        frame->i_fc[1] |= dataOrMgmtFrame->getMoreFragments();
        frame->i_fc[1] <<= 1;
        frame->i_fc[1] |= dataOrMgmtFrame->getFromDS();
        frame->i_fc[1] <<= 1;
        frame->i_fc[1] |= dataOrMgmtFrame->getToDS();
        frame->i_dur = (int)(dataOrMgmtFrame->getDuration().dbl()*1000);
        dataOrMgmtFrame->getReceiverAddress().getAddressBytes(frame->i_addr1);
        dataOrMgmtFrame->getTransmitterAddress().getAddressBytes(frame->i_addr2);
        dataOrMgmtFrame->getAddress3().getAddressBytes(frame->i_addr3);
        frame->i_seq = dataOrMgmtFrame->getSequenceNumber();
        frame->i_seq <<= 4;
        frame->i_seq |= dataOrMgmtFrame->getFragmentNumber();

        packetLength = 6 + 3*IEEE80211_ADDR_LEN;

        if (NULL != dynamic_cast<Ieee80211DataFrameWithSNAP *>(pkt))
        {
            Ieee80211DataFrameWithSNAP *dataFrame = dynamic_cast<Ieee80211DataFrameWithSNAP *>(pkt);
            if (dataFrame->getFromDS() && dataFrame->getToDS())
            {
                packetLength += IEEE80211_ADDR_LEN;
                dataFrame->getAddress4().getAddressBytes((uint8_t *) (buf));
            }

            struct snap_header *snap_hdr = (struct snap_header *) (buf + packetLength);

            snap_hdr->dsap = 0xAA;
            snap_hdr->ssap = 0xAA;
            snap_hdr->ctrl = 0x03;
            snap_hdr->snap = htons(dataFrame->getEtherType());
            snap_hdr->snap <<= 24;

            packetLength += 8;
            cMessage *encapPacket = dataFrame->getEncapsulatedPacket();

            switch (dataFrame->getEtherType())
            {
#ifdef WITH_IPv4
                case ETHERTYPE_IP:
                    packetLength += IPv4Serializer().serialize(check_and_cast<IPv4Datagram *>(encapPacket),
                                                                       buf+packetLength, bufsize-packetLength, true);
                    break;
#endif

#ifdef WITH_IPv6
                case ETHERTYPE_IPV6:
                    packetLength += IPv6Serializer().serialize(check_and_cast<IPv6Datagram *>(encapPacket),
                                                                       buf+packetLength, bufsize-packetLength);
                    break;
#endif

#ifdef WITH_IPv4
                case ETHERTYPE_ARP:
                    packetLength += ARPSerializer().serialize(check_and_cast<ARPPacket *>(encapPacket),
                                                                       buf+packetLength, bufsize-packetLength);
                    break;
#endif

                default:
                    throw cRuntimeError("Ieee80211Serializer: cannot serialize protocol %x", dataFrame->getEtherType());
            }
        }

        else if (NULL != dynamic_cast<Ieee80211AuthenticationFrame *>(pkt))
        {
            //type = ST_AUTHENTICATION;
            Ieee80211AuthenticationFrame *Frame = dynamic_cast<Ieee80211AuthenticationFrame *>(pkt);
            // 1    Authentication algorithm number
            setTwoByte(0, buf + packetLength);
            // 2    Authentication transaction sequence number
            setTwoByte(Frame->getBody().getSequenceNumber(), buf + packetLength, 2);
            // 3    Status code                                 The status code information is reserved in certain Authentication frames as defined in Table 7-17.
            setTwoByte(Frame->getBody().getStatusCode(), buf + packetLength, 4);
            // 4    Challenge text                              The challenge text information is present only in certain Authentication frames as defined in Table 7-17.
            // Last Vendor Specific                             One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
            packetLength += 6;
        }

        else if (NULL != dynamic_cast<Ieee80211DeauthenticationFrame *>(pkt))
        {
            //type = ST_DEAUTHENTICATION;
            Ieee80211DeauthenticationFrame *Frame = dynamic_cast<Ieee80211DeauthenticationFrame *>(pkt);
            setTwoByte(Frame->getBody().getReasonCode(), buf + packetLength);
            packetLength += 2;
        }

        else if (NULL != dynamic_cast<Ieee80211DisassociationFrame *>(pkt))
        {
            //type = ST_DISASSOCIATION;
            Ieee80211DisassociationFrame *Frame = dynamic_cast<Ieee80211DisassociationFrame *>(pkt);
            setTwoByte(Frame->getBody().getReasonCode(), buf + packetLength);
            packetLength += 2;
        }

        else if (NULL != dynamic_cast<Ieee80211ProbeRequestFrame *>(pkt))
        {
            //type = ST_PROBEREQUEST;
            Ieee80211ProbeRequestFrame *Frame = dynamic_cast<Ieee80211ProbeRequestFrame *>(pkt);
            // 1    SSID
            const char *SSID = Frame->getBody().getSSID();
            unsigned int length;
            for (length = 0; SSID[length]!=0; length++);
            setOneByte(length, buf + packetLength, 1);
            setNByte((uint8_t*)SSID, length, buf + packetLength, 2);
            packetLength += 2 + length;
            // 2    Supported rates
            setOneByte(1, buf + packetLength);
            setOneByte(Frame->getBody().getSupportedRates().numRates, buf + packetLength, 1);
            for (unsigned int i = 0; i < Frame->getBody().getSupportedRates().numRates; i++)
            {
                uint8_t rate = ceil(Frame->getBody().getSupportedRates().rate[i]/0.5);
                // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
                setOneByte(rate, buf + packetLength, 2 + i);
            }
            packetLength += 2 + Frame->getBody().getSupportedRates().numRates;
            // 3    Request information         May be included if dot11MultiDomainCapabilityEnabled is true.
            // 4    Extended Supported Rates    The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
            // Last Vendor Specific             One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
        }

        else if (NULL != dynamic_cast<Ieee80211AssociationRequestFrame *>(pkt))
        {
            //type = ST_ASSOCIATIONREQUEST;
            Ieee80211AssociationRequestFrame *Frame = dynamic_cast<Ieee80211AssociationRequestFrame *>(pkt);
            // 1    Capability
            packetLength += 2;
            // 2    Listen interval
            packetLength += 2;
            // 3    SSID
            const char *SSID = Frame->getBody().getSSID();
            unsigned int length;
            for (length = 0; SSID[length]!=0; length++);
            setOneByte(length, buf + packetLength, 1);
            setNByte((uint8_t*)SSID, length, buf + packetLength, 2);
            packetLength += 2 + length;
            // 4    Supported rates
            setOneByte(1, buf + packetLength);
            setOneByte(Frame->getBody().getSupportedRates().numRates, buf + packetLength, 1);
            for (unsigned int i = 0; i < Frame->getBody().getSupportedRates().numRates; i++)
            {
                uint8_t rate = ceil(Frame->getBody().getSupportedRates().rate[i]/0.5);
                // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
                setOneByte(rate, buf + packetLength, 2 + i);
            }
            packetLength += 2 + Frame->getBody().getSupportedRates().numRates;
            // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
            // 6    Power Capability           The Power Capability element shall be present if dot11SpectrumManagementRequired is true.
            // 7    Supported Channel          The Supported Channels element shall be present if dot11SpectrumManagementRequired is true.
            // 8    RSN                        The RSN information element is only present within Association Request frames generated by STAs that have dot11RSNAEnabled set to TRUE.
            // 9    QoS Capability             The QoS Capability element is present when dot11QosOption- Implemented is true.
            // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
        }

        else if (NULL != dynamic_cast<Ieee80211ReassociationRequestFrame *>(pkt))
        {
            //type = ST_REASSOCIATIONREQUEST;
            Ieee80211ReassociationRequestFrame *Frame = dynamic_cast<Ieee80211ReassociationRequestFrame *>(pkt);
            // 1    Capability
            packetLength += 2;
            // 2    Listen interval
            packetLength += 2;
            // 3    Current AP address
            uint8_t temp[6];
            Frame->getBody().getCurrentAP().getAddressBytes(temp);
            setNByte(temp, 6, buf + packetLength);
            packetLength += 6;
            // 4    SSID
            const char *SSID = Frame->getBody().getSSID();
            unsigned int length;
            for (length = 0; SSID[length]!=0; length++);
            setOneByte(length, buf + packetLength, 1);
            setNByte((uint8_t*)SSID, length, buf + packetLength, 2);
            packetLength += 2 + length;
            // 5    Supported rates
            setOneByte(1, buf + packetLength);
            setOneByte(Frame->getBody().getSupportedRates().numRates, buf + packetLength, 1);
            for (unsigned int i = 0; i < Frame->getBody().getSupportedRates().numRates; i++)
            {
                uint8_t rate = ceil(Frame->getBody().getSupportedRates().rate[i]/0.5);
                // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
                setOneByte(rate, buf + packetLength, 2 + i);
            }
            packetLength += 2 + Frame->getBody().getSupportedRates().numRates;
            // 6    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
            // 7    Power Capability           The Power Capability element shall be present if dot11SpectrumManagementRequired is true.
            // 8    Supported Channels         The Supported Channels element shall be present if dot11SpectrumManagementRequired is true.
            // 9    RSN                        The RSN information element is only present within Reassociation Request frames generated by STAs that have dot11RSNAEnabled set to TRUE.
            // 10   QoS Capability             The QoS Capability element is present when dot11QosOption- Implemented is true.
            // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
        }

        else if (NULL != dynamic_cast<Ieee80211AssociationResponseFrame *>(pkt))
        {
            //type = ST_ASSOCIATIONRESPONSE;
            Ieee80211AssociationResponseFrame *Frame = dynamic_cast<Ieee80211AssociationResponseFrame *>(pkt);
            // 1    Capability
            packetLength += 2;
            // 2    Status code
            setTwoByte(Frame->getBody().getStatusCode(), buf + packetLength);
            packetLength += 2;
            // 3    AID
            setTwoByte(Frame->getBody().getAid(), buf + packetLength);
            packetLength += 2;
            // 4    Supported rates
            setOneByte(1, buf + packetLength);
            setOneByte(Frame->getBody().getSupportedRates().numRates, buf + packetLength, 1);
            for (unsigned int i = 0; i < Frame->getBody().getSupportedRates().numRates; i++)
            {
                uint8_t rate = ceil(Frame->getBody().getSupportedRates().rate[i]/0.5);
                // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
                setOneByte(rate, buf + packetLength, 2 + i);
            }
            packetLength += 2 + Frame->getBody().getSupportedRates().numRates;
            // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
            // 6    EDCA Parameter Set
            // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
        }

        else if (NULL != dynamic_cast<Ieee80211ReassociationResponseFrame *>(pkt))
        {
            //type = ST_REASSOCIATIONRESPONSE;
            Ieee80211ReassociationResponseFrame *Frame = dynamic_cast<Ieee80211ReassociationResponseFrame *>(pkt);
            // 1    Capability
            packetLength += 2;
            // 2    Status code
            setTwoByte(Frame->getBody().getStatusCode(), buf + packetLength);
            packetLength += 2;
            // 3    AID
            setTwoByte(Frame->getBody().getAid(), buf + packetLength);
            packetLength += 2;
            // 4    Supported rates
            setOneByte(1, buf + packetLength);
            setOneByte(Frame->getBody().getSupportedRates().numRates, buf + packetLength, 1);
            for (unsigned int i = 0; i < Frame->getBody().getSupportedRates().numRates; i++)
            {
                uint8_t rate = ceil(Frame->getBody().getSupportedRates().rate[i]/0.5);
                // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
                setOneByte(rate, buf + packetLength, 2 + i);
            }
            packetLength += 2 + Frame->getBody().getSupportedRates().numRates;
            // 5    Extended Supported Rates   The Extended Supported Rates element is present whenever there are more than eight supported rates, and it is optional otherwise.
            // 6    EDCA Parameter Set
            // Last Vendor Specific            One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
        }

        else if (NULL != dynamic_cast<Ieee80211BeaconFrame *>(pkt))
        {
            //type = ST_BEACON;
            Ieee80211BeaconFrame *Frame = dynamic_cast<Ieee80211BeaconFrame *>(pkt);
            // 1    Timestamp
            packetLength += 8;
            // 2    Beacon interval
            setTwoByte((uint16_t)(Frame->getBody().getBeaconInterval().inUnit(SIMTIME_US)/1024),buf + packetLength);
            packetLength += 2;
            // 3    Capability
            packetLength += 2;
            // 4    Service Set Identifier (SSID)
            const char *SSID = Frame->getBody().getSSID();
            unsigned int length;
            for (length = 0; SSID[length]!=0; length++);
            setOneByte(length, buf + packetLength, 1);
            setNByte((uint8_t*)SSID, length, buf + packetLength, 2);
            packetLength += 2 + length;
            // 5    Supported rates
            setOneByte(1, buf + packetLength);
            setOneByte(Frame->getBody().getSupportedRates().numRates, buf + packetLength, 1);
            for (unsigned int i = 0; i < Frame->getBody().getSupportedRates().numRates; i++)
            {
                uint8_t rate = ceil(Frame->getBody().getSupportedRates().rate[i]/0.5);
                // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
                setOneByte(rate, buf + packetLength, 2 + i);
            }
            packetLength += 2 + Frame->getBody().getSupportedRates().numRates;
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

        else if (NULL != dynamic_cast<Ieee80211ProbeResponseFrame *>(pkt))
        {
            //type = ST_PROBERESPONSE;
            Ieee80211ProbeResponseFrame *Frame = dynamic_cast<Ieee80211ProbeResponseFrame *>(pkt);
            // 1      Timestamp
            packetLength += 8;
            // 2      Beacon interval
            setTwoByte((uint16_t)(Frame->getBody().getBeaconInterval().inUnit(SIMTIME_US)/1024),buf + packetLength);
            packetLength += 2;
            // 3      Capability
            packetLength += 2;
            // 4      SSID
            const char *SSID = Frame->getBody().getSSID();
            unsigned int length;
            for (length = 0; SSID[length]!=0; length++);
            setOneByte(length, buf + packetLength, 1);
            setNByte((uint8_t*)SSID, length, buf + packetLength, 2);
            packetLength += 2 + length;
            // 5      Supported rates
            setOneByte(1, buf + packetLength);
            setOneByte(Frame->getBody().getSupportedRates().numRates, buf + packetLength, 1);
            for (unsigned int i = 0; i < Frame->getBody().getSupportedRates().numRates; i++)
            {
                uint8_t rate = ceil(Frame->getBody().getSupportedRates().rate[i]/0.5);
                // rate |= 0x80 if rate contained in the BSSBasicRateSet parameter
                setOneByte(rate, buf + packetLength, 2 + i);
            }
            packetLength += 2 + Frame->getBody().getSupportedRates().numRates;
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

        else if (NULL != dynamic_cast<Ieee80211ActionFrame *>(pkt))
        {
            //type = ST_ACTION;
            // 1    Action
            // Last One or more vendor-specific information elements may appear in this frame. This information element follows all other information elements.
            return 0;
        }
    }
    else
        throw cRuntimeError("Ieee80211Serializer: cannot serialize the frame");

    // CRC
    uint32_t *fcs = (uint32_t *) (buf + packetLength);
    *fcs = ethernetCRC(buf, packetLength);
    return packetLength + 4;
}

unsigned int parseDataOrMgmtFrame(const unsigned char *buf, unsigned int bufsize, cPacket *pkt, short type)
{
    struct ieee80211_frame_addr4 *frame = (struct ieee80211_frame_addr4 *) (buf);
    Ieee80211DataOrMgmtFrame *Frame = (Ieee80211DataOrMgmtFrame*)pkt;
    Frame->setType(type);
    Frame->setToDS(frame->i_fc[1]&0x1);
    Frame->setFromDS(frame->i_fc[1]&0x2);
    Frame->setMoreFragments(frame->i_fc[1]&0x4);
    Frame->setRetry(frame->i_fc[1]&0x8);
    Frame->setDuration(SimTime((double)frame->i_dur/1000.0));
    MACAddress temp;
    temp.setAddressBytes(frame->i_addr1);
    Frame->setReceiverAddress(temp);
    temp.setAddressBytes(frame->i_addr2);
    Frame->setTransmitterAddress(temp);
    temp.setAddressBytes(frame->i_addr3);
    Frame->setAddress3(temp);
    Frame->setSequenceNumber(frame->i_seq >> 4);
    Frame->setFragmentNumber(frame->i_seq & 0xF);

    if (type == ST_DATA && Frame->getFromDS() && Frame->getToDS())
    {
        temp.setAddressBytes(frame->i_addr4);
        ((Ieee80211DataFrame*)Frame)->setAddress4(temp);
        return 6 + 4*IEEE80211_ADDR_LEN;
    }
    else
        return 6 + 3*IEEE80211_ADDR_LEN;
}

cPacket* Ieee80211Serializer::parse(const unsigned char *buf, unsigned int bufsize)
{
    uint32_t crc = ethernetCRC(buf, bufsize);
    EV_DEBUG << "CRC: "<< crc << " (" << (0x2144DF1C == crc ) << ")"<< endl;
    uint8_t *type = (uint8_t *) (buf);
    switch(*type)
    {
        case 0xD4: // ST_ACK
        {
            struct ieee80211_frame_ack *frame = (struct ieee80211_frame_ack *) (buf);
            cPacket *pkt = new Ieee80211ACKFrame;
            Ieee80211ACKFrame *ackFrame = (Ieee80211ACKFrame*)pkt;
            ackFrame->setType(ST_ACK);
            ackFrame->setToDS(false);
            ackFrame->setFromDS(false);
            ackFrame->setRetry(false);
            ackFrame->setMoreFragments(false);
            ackFrame->setDuration(SimTime((double)frame->i_dur/1000.0));
            MACAddress temp;
            temp.setAddressBytes(frame->i_ra);
            ackFrame->setReceiverAddress(temp);
            return pkt;
        }
        case 0xB4: // ST_RTS
        {
            struct ieee80211_frame_rts *frame = (struct ieee80211_frame_rts *) (buf);
            cPacket *pkt = new Ieee80211RTSFrame;
            Ieee80211RTSFrame *rtsFrame = (Ieee80211RTSFrame*)pkt;
            rtsFrame->setType(ST_RTS);
            rtsFrame->setToDS(false);
            rtsFrame->setFromDS(false);
            rtsFrame->setRetry(false);
            rtsFrame->setMoreFragments(false);
            rtsFrame->setDuration(SimTime((double)frame->i_dur/1000.0));
            MACAddress temp;
            temp.setAddressBytes(frame->i_ra);
            rtsFrame->setReceiverAddress(temp);
            temp.setAddressBytes(frame->i_ta);
            rtsFrame->setTransmitterAddress(temp);
            return pkt;
        }
        case 0xC4: // ST_CTS
        {
            struct ieee80211_frame_cts *frame = (struct ieee80211_frame_cts *) (buf);
            cPacket *pkt = new Ieee80211CTSFrame;
            Ieee80211CTSFrame *ctsFrame = (Ieee80211CTSFrame*)pkt;
            ctsFrame->setType(ST_CTS);
            ctsFrame->setToDS(false);
            ctsFrame->setFromDS(false);
            ctsFrame->setRetry(false);
            ctsFrame->setMoreFragments(false);
            ctsFrame->setDuration(SimTime((double)frame->i_dur/1000.0));
            MACAddress temp;
            temp.setAddressBytes(frame->i_ra);
            ctsFrame->setReceiverAddress(temp);
            return pkt;
        }
        case 0x8: // ST_DATA
        {
            cPacket *pkt = new Ieee80211DataFrameWithSNAP;
            unsigned int packetLength = parseDataOrMgmtFrame(buf, bufsize, pkt, ST_DATA);
            Ieee80211DataFrameWithSNAP *dataFrame = (Ieee80211DataFrameWithSNAP*)pkt;
            struct snap_header *snap_hdr = (struct snap_header *) (buf + packetLength);
            uint64_t temp64 = snap_hdr->snap;
            temp64 >>= 24;
            dataFrame->setEtherType(ntohs(snap_hdr->snap >> 24));

            packetLength += 8;

            cPacket *encapPacket = NULL;

            switch (dataFrame->getEtherType())
            {
#ifdef WITH_IPv4
                case ETHERTYPE_IP:
                    encapPacket = new IPv4Datagram("ipv4-from-wire");
                    IPv4Serializer().parse(buf+packetLength, bufsize-packetLength, (IPv4Datagram *)encapPacket);
                    break;
#endif

#ifdef WITH_IPv6
                case ETHERTYPE_IPV6:
                    encapPacket = new IPv6Datagram("ipv6-from-wire");
                    IPv6Serializer().parse(buf+packetLength, bufsize-packetLength, (IPv6Datagram *)encapPacket);
                    break;
#endif

                case ETHERTYPE_ARP:
                    encapPacket = new ARPPacket("arp-from-wire");
                    ARPSerializer().parse(buf+packetLength, bufsize-packetLength, (ARPPacket *)encapPacket);
                    break;

                default:
                    throw cRuntimeError("Ieee80211Serializer: cannot parse protocol %x", dataFrame->getEtherType());
            }

            ASSERT(encapPacket);
            dataFrame->encapsulate(encapPacket);
            dataFrame->setName(encapPacket->getName());
            return pkt;
        }

        case 0xB0: // ST_AUTHENTICATION
        {
            cPacket *pkt = new Ieee80211AuthenticationFrame;
            unsigned int packetLength = parseDataOrMgmtFrame(buf, bufsize, pkt, ST_AUTHENTICATION);
            Ieee80211AuthenticationFrameBody body;

            body.setSequenceNumber(getTwoByte(buf + packetLength, 2));
            body.setStatusCode(getTwoByte(buf + packetLength, 4));

            ((Ieee80211AuthenticationFrame*)pkt)->setBody(body);
            return pkt;
        }

        case 0xC0: //ST_ST_DEAUTHENTICATION
        {
            cPacket *pkt = new Ieee80211DeauthenticationFrame;
            unsigned int packetLength = parseDataOrMgmtFrame(buf, bufsize, pkt, ST_DEAUTHENTICATION);
            Ieee80211DeauthenticationFrameBody body;

            body.setReasonCode(getTwoByte(buf + packetLength));

            ((Ieee80211DeauthenticationFrame*)pkt)->setBody(body);
            return pkt;
        }

        case 0xA0: // ST_DISASSOCIATION
        {
            cPacket *pkt = new Ieee80211DisassociationFrame;
            unsigned int packetLength = parseDataOrMgmtFrame(buf, bufsize, pkt, ST_DISASSOCIATION);
            Ieee80211DisassociationFrameBody body;

            body.setReasonCode(getTwoByte(buf + packetLength));

            ((Ieee80211DisassociationFrame*)pkt)->setBody(body);
            return pkt;
        }

        case 0x40: // ST_PROBEREQUEST
        {
            cPacket *pkt = new Ieee80211ProbeRequestFrame;
            unsigned int packetLength = parseDataOrMgmtFrame(buf, bufsize, pkt, ST_PROBEREQUEST);
            Ieee80211ProbeRequestFrameBody body;

            char SSID[33];
            unsigned int length = getOneByte(buf + packetLength, 1);
            getNByte((uint8_t*)SSID, length, buf + packetLength, 2);
            SSID[length] = '\0';
            body.setSSID(SSID);
            packetLength += 2 + length;

            Ieee80211SupportedRatesElement supRat;
            supRat.numRates = getOneByte(buf + packetLength, 1);
            for (unsigned int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(getOneByte(buf + packetLength, 2 + i) & 0x7F)*0.5;
            body.setSupportedRates(supRat);

            ((Ieee80211ProbeRequestFrame*)pkt)->setBody(body);
            return pkt;
        }

        case 0x00: // ST_ASSOCIATIONREQUEST
        {
            cPacket *pkt = new Ieee80211AssociationRequestFrame;
            unsigned int packetLength = parseDataOrMgmtFrame(buf, bufsize, pkt, ST_ASSOCIATIONREQUEST);
            Ieee80211AssociationRequestFrameBody body;
            packetLength += 4;

            char SSID[33];
            unsigned int length = getOneByte(buf + packetLength, 1);
            getNByte((uint8_t*)SSID, length, buf + packetLength, 2);
            SSID[length] = '\0';
            body.setSSID(SSID);
            packetLength += 2 + length;

            Ieee80211SupportedRatesElement supRat;
            supRat.numRates = getOneByte(buf + packetLength, 1);
            for (unsigned int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(getOneByte(buf + packetLength, 2 + i) & 0x7F)*0.5;
            body.setSupportedRates(supRat);

            ((Ieee80211AssociationRequestFrame*)pkt)->setBody(body);
            return pkt;
        }

        case 0x02: // ST_REASSOCIATIONREQUEST
        {
            cPacket *pkt = new Ieee80211ReassociationRequestFrame;
            unsigned int packetLength = parseDataOrMgmtFrame(buf, bufsize, pkt, ST_REASSOCIATIONREQUEST);
            Ieee80211ReassociationRequestFrameBody body;
            packetLength += 4;

            uint8_t temp[6];
            getNByte(temp, 6, buf + packetLength);
            MACAddress tempMAC;
            tempMAC.setAddressBytes(temp);
            body.setCurrentAP(tempMAC);
            packetLength += 6;

            char SSID[33];
            unsigned int length = getOneByte(buf + packetLength, 1);
            getNByte((uint8_t*)SSID, length, buf + packetLength, 2);
            SSID[length] = '\0';
            body.setSSID(SSID);
            packetLength += 2 + length;

            Ieee80211SupportedRatesElement supRat;
            supRat.numRates = getOneByte(buf + packetLength, 1);
            for (unsigned int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(getOneByte(buf + packetLength, 2 + i) & 0x7F)*0.5;
            body.setSupportedRates(supRat);

            ((Ieee80211ReassociationRequestFrame*)pkt)->setBody(body);
            return pkt;
        }

        case 0x01: // ST_ASSOCIATIONRESPONSE
        {
            cPacket *pkt = new Ieee80211AssociationResponseFrame;
            unsigned int packetLength = parseDataOrMgmtFrame(buf, bufsize, pkt, ST_ASSOCIATIONRESPONSE);
            Ieee80211AssociationResponseFrameBody body;
            packetLength += 2;

            body.setStatusCode(getTwoByte(buf + packetLength));
            packetLength += 2;

            body.setAid(getTwoByte(buf + packetLength));
            packetLength += 2;

            Ieee80211SupportedRatesElement supRat;
            supRat.numRates = getOneByte(buf + packetLength, 1);
            for (unsigned int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(getOneByte(buf + packetLength, 2 + i) & 0x7F)*0.5;
            body.setSupportedRates(supRat);

            ((Ieee80211AssociationResponseFrame*)pkt)->setBody(body);
            return pkt;
        }

        case 0x03: // ST_REASSOCIATIONRESPONSE
        {
            cPacket *pkt = new Ieee80211ReassociationResponseFrame;
            unsigned int packetLength = parseDataOrMgmtFrame(buf, bufsize, pkt, ST_REASSOCIATIONRESPONSE);
            Ieee80211ReassociationResponseFrameBody body;
            packetLength += 2;

            body.setStatusCode(getTwoByte(buf + packetLength));
            packetLength += 2;

            body.setAid(getTwoByte(buf + packetLength));
            packetLength += 2;

            Ieee80211SupportedRatesElement supRat;
            supRat.numRates = getOneByte(buf + packetLength, 1);
            for (unsigned int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(getOneByte(buf + packetLength, 2 + i) & 0x7F)*0.5;
            body.setSupportedRates(supRat);

            ((Ieee80211ReassociationResponseFrame*)pkt)->setBody(body);
            return pkt;
        }

        case 0x80: // ST_BEACON
        {
            cPacket *pkt = new Ieee80211BeaconFrame;
            unsigned int packetLength = parseDataOrMgmtFrame(buf, bufsize, pkt, ST_BEACON);
            Ieee80211BeaconFrameBody body;
            packetLength += 8;

            body.setBeaconInterval(SimTime((int64_t)getTwoByte(buf + packetLength)*1024,SIMTIME_US));
            packetLength += 4;

            char SSID[33];
            unsigned int length = getOneByte(buf + packetLength, 1);
            getNByte((uint8_t*)SSID, length, buf + packetLength, 2);
            SSID[length] = '\0';
            body.setSSID(SSID);
            packetLength += 2 + length;

            Ieee80211SupportedRatesElement supRat;
            supRat.numRates = getOneByte(buf + packetLength, 1);
            for (unsigned int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(getOneByte(buf + packetLength, 2 + i) & 0x7F)*0.5;
            body.setSupportedRates(supRat);

            ((Ieee80211BeaconFrame*)pkt)->setBody(body);
            return pkt;
        }

        case 0x50: // ST_PROBERESPONSE
        {
            cPacket *pkt = new Ieee80211ProbeResponseFrame;
            unsigned int packetLength = parseDataOrMgmtFrame(buf, bufsize, pkt, ST_PROBERESPONSE);
            Ieee80211ProbeResponseFrameBody body;
            packetLength += 8;

            body.setBeaconInterval(SimTime((int64_t)getTwoByte(buf + packetLength)*1024,SIMTIME_US));
            packetLength += 4;

            char SSID[33];
            unsigned int length = getOneByte(buf + packetLength, 1);
            getNByte((uint8_t*)SSID, length, buf + packetLength, 2);
            SSID[length] = '\0';
            body.setSSID(SSID);
            packetLength += 2 + length;

            Ieee80211SupportedRatesElement supRat;
            supRat.numRates = getOneByte(buf + packetLength, 1);
            for (unsigned int i = 0; i < supRat.numRates; i++)
                supRat.rate[i] = (double)(getOneByte(buf + packetLength, 2 + i) & 0x7F)*0.5;
            body.setSupportedRates(supRat);

            ((Ieee80211ProbeResponseFrame*)pkt)->setBody(body);
            return pkt;
        }

        case 0xD0: // type = ST_ACTION
        {
            // Ieee80211ActionFrame
            return NULL;
        }

        default:
            throw cRuntimeError("Ieee80211Serializer: cannot serialize the frame");
    }
}

} // namespace serializer

} // namespace inet
