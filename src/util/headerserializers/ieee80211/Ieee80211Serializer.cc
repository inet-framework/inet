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

using namespace INETFw;

int Ieee80211Serializerr::serialize(const Ieee80211Frame *pkt, unsigned char *buf, unsigned int bufsize)
{
    if (NULL != dynamic_cast<Ieee80211ACKFrame *>(pkt))
    {
        struct ieee80211_frame_ack *frame = (struct ieee80211_frame_ack *) (buf);
    }

    else if (NULL != dynamic_cast<Ieee80211RTSFrame *>(pkt))
    {
        struct ieee80211_frame_rts *frame = (struct ieee80211_frame_rts *) (buf);
    }

    else if (NULL != dynamic_cast<Ieee80211CTSFrame *>(pkt))
    {
        struct ieee80211_frame_cts *frame = (struct ieee80211_frame_cts *) (buf);
    }

    else if (NULL != dynamic_cast<Ieee80211DataFrame *>(pkt))
    {
        struct ieee80211_frame_addr4 *frame = (struct ieee80211_frame_addr4 *) (buf);
    }

    /*else if (NULL != dynamic_cast<Ieee80211DataFrameWithSNAP *>(pkt))
    {
        struct ieee80211_frame *frame = (struct ieee80211_frame *) (buf);
    }*/

    else if (NULL != dynamic_cast<Ieee80211ManagementFrame *>(pkt))
    {
        struct ieee80211_frame *frame = (struct ieee80211_frame *) (buf);
    }

    else
        throw cRuntimeError("Ieee80211Serializer: cannot serialize protocol the frame");

    int packetLength = 0;
    return packetLength;
}

void Ieee80211Serializer::parse(const unsigned char *buf, unsigned int bufsize, Ieee80211Frame *pkt)
{
    struct ieee80211_frame *frame = (struct ieee80211_frame *) (buf);
}

