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

#include <Ieee80211Serializer.h>
#include <Ieee80211Frame_m.h>
namespace INETFw // load headers into a namespace, to avoid conflicts with platform definitions of the same stuff
{
#include "headers/bsdint.h"
#include "headers/in.h"
#include "headers/in_systm.h"
#include "headers/ethernet.h"
#include "headers/ieee80211.h"
};

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
//#include <netinet/in.h>  // htonl, ntohl, ...
#endif

#ifdef WITH_IPv4
#include "IPv4Serializer.h"
#endif

#ifdef WITH_IPv6
#include "IPv6Serializer.h"
#endif

#include "ARPSerializer.h"

using namespace INETFw;


int Ieee80211Serializer::serialize(Ieee80211Frame *pkt, unsigned char *buf, unsigned int bufsize)
{
    if (NULL != dynamic_cast<Ieee80211ACKFrame *>(pkt))
    {
        Ieee80211ACKFrame *ackFrame = dynamic_cast<Ieee80211ACKFrame *>(pkt);
        struct ieee80211_frame_ack *frame = (struct ieee80211_frame_ack *) (buf);
        frame->i_fc[0] = 0xD4;
        frame->i_fc[1] = 0;
        frame->i_dur = (int)(ackFrame->getDuration().dbl()*1000);
        ackFrame->getReceiverAddress().getAddressBytes(frame->i_ra);

        return 4 + IEEE80211_ADDR_LEN;
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
        return 4+ 2*IEEE80211_ADDR_LEN;
    }

    else if (NULL != dynamic_cast<Ieee80211CTSFrame *>(pkt))
    {
        Ieee80211CTSFrame *ctsFrame = dynamic_cast<Ieee80211CTSFrame *>(pkt);
        struct ieee80211_frame_cts *frame = (struct ieee80211_frame_cts *) (buf);
        frame->i_fc[0] = 0xC4;
        frame->i_fc[1] = 0;
        frame->i_dur = (int)(ctsFrame->getDuration().dbl()*1000);
        ctsFrame->getReceiverAddress().getAddressBytes(frame->i_ra);
        return 4 + IEEE80211_ADDR_LEN;
    }

    else if (NULL != dynamic_cast<Ieee80211DataFrameWithSNAP *>(pkt))
    {
        Ieee80211DataFrameWithSNAP *dataFrame = dynamic_cast<Ieee80211DataFrameWithSNAP *>(pkt);
        unsigned int packetLength;
        struct ieee80211_frame_addr4 *frame = (struct ieee80211_frame_addr4 *) (buf);
        frame->i_fc[0] = 0x8;
        frame->i_fc[1] = 0; // TODO: Order, Protected Frame, MoreData, Power Mgt
        frame->i_fc[1] |= dataFrame->getRetry();
        frame->i_fc[1] <<= 1;
        frame->i_fc[1] |= dataFrame->getMoreFragments();
        frame->i_fc[1] <<= 1;
        frame->i_fc[1] |= dataFrame->getFromDS();
        frame->i_fc[1] <<= 1;
        frame->i_fc[1] |= dataFrame->getToDS();
        frame->i_dur = (int)(dataFrame->getDuration().dbl()*1000);
        dataFrame->getReceiverAddress().getAddressBytes(frame->i_addr1);
        dataFrame->getTransmitterAddress().getAddressBytes(frame->i_addr2);
        dataFrame->getAddress3().getAddressBytes(frame->i_addr3);
        dataFrame->getAddress4().getAddressBytes(frame->i_addr4);
        frame->i_seq = dataFrame->getFragmentNumber();
        frame->i_seq <<= 12;
        frame->i_seq |= (dataFrame->getSequenceNumber() & 0xFFF);
        //If there's no address 3 then we overwrite it
        if (dataFrame->getFromDS() && dataFrame->getToDS())
            packetLength = 6 + 4*IEEE80211_ADDR_LEN;
        else
            packetLength = 6 + 3*IEEE80211_ADDR_LEN;

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
        return packetLength;
    }

    else
        throw cRuntimeError("Ieee80211Serializer: cannot serialize the frame");
}

void Ieee80211Serializer::parse(const unsigned char *buf, unsigned int bufsize, cPacket **pkt)
{
    uint8_t *type = (uint8_t *) (buf);
    switch(*type)
    {
        case 0xD4:
        {
            struct ieee80211_frame_ack *frame = (struct ieee80211_frame_ack *) (buf);
            *pkt = new Ieee80211ACKFrame;
            Ieee80211ACKFrame *ackFrame = (Ieee80211ACKFrame*)*pkt;
            ackFrame->setType(ST_ACK);
            ackFrame->setToDS(false);
            ackFrame->setFromDS(false);
            ackFrame->setRetry(false);
            ackFrame->setMoreFragments(false);
            ackFrame->setDuration(SimTime((double)frame->i_dur/1000.0));
            MACAddress temp;
            temp.setAddressBytes(frame->i_ra);
            ackFrame->setReceiverAddress(temp);
            return;
        }
        case 0xB4:
        {
            struct ieee80211_frame_rts *frame = (struct ieee80211_frame_rts *) (buf);
            *pkt = new Ieee80211RTSFrame;
            Ieee80211RTSFrame *rtsFrame = (Ieee80211RTSFrame*)*pkt;
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
            return;
        }
        case 0xC4:
        {
            struct ieee80211_frame_cts *frame = (struct ieee80211_frame_cts *) (buf);
            *pkt = new Ieee80211CTSFrame;
            Ieee80211CTSFrame *ctsFrame = (Ieee80211CTSFrame*)*pkt;
            ctsFrame->setType(ST_CTS);
            ctsFrame->setToDS(false);
            ctsFrame->setFromDS(false);
            ctsFrame->setRetry(false);
            ctsFrame->setMoreFragments(false);
            ctsFrame->setDuration(SimTime((double)frame->i_dur/1000.0));
            MACAddress temp;
            temp.setAddressBytes(frame->i_ra);
            ctsFrame->setReceiverAddress(temp);
            return;
        }
        case 0x8:
        {
            struct ieee80211_frame_addr4 *frame = (struct ieee80211_frame_addr4 *) (buf);
            *pkt = new Ieee80211DataFrameWithSNAP;
            Ieee80211DataFrameWithSNAP *dataFrame = (Ieee80211DataFrameWithSNAP*)*pkt;
            dataFrame->setType(ST_DATA);
            dataFrame->setToDS(frame->i_fc[1]&0x1);
            dataFrame->setFromDS(frame->i_fc[1]&0x2);
            dataFrame->setRetry(frame->i_fc[1]&0x4);
            dataFrame->setMoreFragments(frame->i_fc[1]&0x8);
            dataFrame->setDuration(SimTime((double)frame->i_dur/1000.0));
            MACAddress temp;
            temp.setAddressBytes(frame->i_addr1);
            dataFrame->setReceiverAddress(temp);
            temp.setAddressBytes(frame->i_addr2);
            dataFrame->setTransmitterAddress(temp);
            temp.setAddressBytes(frame->i_addr3);
            dataFrame->setAddress3(temp);
            if (dataFrame->getFromDS() && dataFrame->getToDS())
            {
                temp.setAddressBytes(frame->i_addr4);
                dataFrame->setAddress4(temp);
            }
            dataFrame->setSequenceNumber(frame->i_seq & 0xFFF);
            frame->i_seq >>= 12;
            dataFrame->setFragmentNumber(frame->i_seq);

            unsigned int packetLength;
            if (dataFrame->getFromDS() && dataFrame->getToDS())
                packetLength = 6 + 4*IEEE80211_ADDR_LEN;
            else
                packetLength = 6 + 3*IEEE80211_ADDR_LEN;

            struct snap_header *snap_hdr = (struct snap_header *) (buf + packetLength);
            snap_hdr->snap >>= 24;
            dataFrame->setEtherType(ntohs(snap_hdr->snap));

            packetLength += 8;
            EV_DEBUG << "packetLength: " << packetLength << endl;
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
            return;
        }
        default:
            throw cRuntimeError("Ieee80211Serializer: cannot serialize the frame");
    }
}

