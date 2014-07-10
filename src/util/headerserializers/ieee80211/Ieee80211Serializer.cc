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

void Ieee80211Serializer::parse(const unsigned char *buf, unsigned int bufsize, Ieee80211Frame *pkt)
{
    struct ieee80211_frame *frame = (struct ieee80211_frame *) (buf);
}

