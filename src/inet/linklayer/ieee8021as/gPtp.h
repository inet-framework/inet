//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

#ifndef GPTP_H_
#define GPTP_H_


class gPtp_Sync;
class gPtp_FollowUp;
class gPtp_PdelayReq;
class gPtp_PdelayResp;
class gPtp_PdelayRespFollowUp;

/* Packet size is in byte */
enum gPtpPacketSize: int {
    SYNC_PACKET_SIZE = 44,
    FOLLOW_UP_PACKET_SIZE = 76,
    PDELAY_REQ_PACKET_SIZE = 54,
    PDELAY_RESP_PACKET_SIZE = 54,
    PDELAY_RESP_FOLLOW_UP_PACKET_SIZE = 54
};

/* Below is used to calculate packet transmission time */
enum gPtpHeader: int {
    MAC_HEADER = 22,
    CRC_CHECKSUM = 4
};

namespace gPtp
{
    gPtp_Sync* newSyncPacket();

    gPtp_FollowUp* newFollowUpPacket();

    gPtp_PdelayReq* newDelayReqPacket();

    gPtp_PdelayResp* newDelayRespPacket();

    gPtp_PdelayRespFollowUp* newDelayRespFollowUpPacket();
}

#endif /* GPTP_H_ */
