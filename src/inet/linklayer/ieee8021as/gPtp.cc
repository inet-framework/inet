//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

#include "gPtp.h"
#include "gPtpPacket_m.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h"
#include "inet/transportlayer/contract/udp/UDPControlInfo_m.h"
#include <iostream>

using namespace std;

gPtp_Sync* gPtp::newSyncPacket()
{
    gPtp_Sync* p = new gPtp_Sync("Sync", inet::IEEE802CTRL_DATA);
    p->setName("Sync");

    /* Sync message length 44 byte */
    p->setByteLength(SYNC_PACKET_SIZE);

    return p;
}

gPtp_FollowUp* gPtp::newFollowUpPacket()
{
    gPtp_FollowUp* p = new gPtp_FollowUp("Follow_Up", inet::IEEE802CTRL_DATA);
    p->setName("Follow_Up");

    /* Follow_Up message length 76 byte */
    p->setByteLength(FOLLOW_UP_PACKET_SIZE);

    return p;
}

gPtp_PdelayReq* gPtp::newDelayReqPacket()
{
    gPtp_PdelayReq* p = new gPtp_PdelayReq("Pdelay_Req", inet::IEEE802CTRL_DATA);
    p->setName("Pdelay_Req");

    /* Pdelay_Req message length 54 byte */
    p->setByteLength(PDELAY_REQ_PACKET_SIZE);

    return p;
}

gPtp_PdelayResp* gPtp::newDelayRespPacket()
{
    gPtp_PdelayResp* p = new gPtp_PdelayResp("Pdelay_Resp", inet::IEEE802CTRL_DATA);
    p->setName("Pdelay_Resp");

    /* Pdelay_Resp message length 54 byte */
    p->setByteLength(PDELAY_RESP_PACKET_SIZE);

    return p;
}

gPtp_PdelayRespFollowUp* gPtp::newDelayRespFollowUpPacket()
{
    gPtp_PdelayRespFollowUp* p = new gPtp_PdelayRespFollowUp("Pdelay_Resp_Follow_Up", inet::IEEE802CTRL_DATA);
    p->setName("Pdelay_Resp_Follow_Up");

    /* Pdelay_Resp_Follow_Up message length 54 byte */
    p->setByteLength(PDELAY_RESP_FOLLOW_UP_PACKET_SIZE);

    return p;
}




