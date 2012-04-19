//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen, Andras Varga
// Copyright (C) 2010 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <platdep/sockets.h>

#include "headers/defs.h"

namespace INETFw // load headers into a namespace, to avoid conflicts with platform definitions of the same stuff
{
#include "headers/bsdint.h"
#include "headers/in.h"
#include "headers/in_systm.h"
#include "headers/ip.h"
#include "headers/sctp.h"
};

#include "SCTPSerializer.h"
#include "SCTPAssociation.h"
//#include "platdep/intxtypes.h"

#if !defined(_WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>  // htonl, ntohl, ...
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

#include <sys/types.h>


using namespace INETFw;



int32 SCTPSerializer::serialize(const SCTPMessage *msg, unsigned char *buf, uint32 bufsize)
{
    int32 size_init_chunk = sizeof(struct init_chunk);
    int32 size_sack_chunk = sizeof(struct sack_chunk);
    int32 size_heartbeat_chunk = sizeof(struct heartbeat_chunk);
    int32 size_heartbeat_ack_chunk = sizeof(struct heartbeat_ack_chunk);
    int32 size_chunk = sizeof(struct chunk);


    struct common_header *ch = (struct common_header*) (buf);
    uint32 writtenbytes = sizeof(struct common_header);

    // fill SCTP common header structure
    ch->source_port = htons(msg->getSrcPort());
    ch->destination_port = htons(msg->getDestPort());
    ch->verification_tag = htonl(msg->getTag());


    // SCTP chunks:
    int32 noChunks = msg->getChunksArraySize();
        for (int32 cc = 0; cc < noChunks; cc++)
        {
            const SCTPChunk *chunk = check_and_cast<SCTPChunk *>(((SCTPMessage *)msg)->getChunks(cc));
            unsigned char chunkType = chunk->getChunkType();
            switch (chunkType)
            {
                case DATA:
                {
                    //sctpEV3<<simulation.simTime()<<" SCTPAssociation:: Data sent \n";
                    SCTPDataChunk *dataChunk = check_and_cast<SCTPDataChunk *>(chunk);
                    struct data_chunk *dc = (struct data_chunk*) (buf + writtenbytes); // append data to buffer
                    unsigned char flags = 0;

                    // fill buffer with data from SCTP data chunk structure
                    dc->type = dataChunk->getChunkType();
                    if (dataChunk->getUBit())
                        flags |= UNORDERED_BIT;
                    if (dataChunk->getBBit())
                        flags |= BEGIN_BIT;
                    if (dataChunk->getEBit())
                        flags |= END_BIT;
                    dc->flags = flags;
                    dc->length = htons(dataChunk->getByteLength());
                    dc->tsn = htonl(dataChunk->getTsn());
                    dc->sid = htons(dataChunk->getSid());
                    dc->ssn = htons(dataChunk->getSsn());
                    dc->ppi = htonl(dataChunk->getPpid());
                    writtenbytes += SCTP_DATA_CHUNK_LENGTH;

                    SCTPSimpleMessage *smsg = check_and_cast<SCTPSimpleMessage *>(dataChunk->getEncapsulatedPacket());
                        // T.D. 09.02.2010: Only copy data when there is something to copy!
                        const uint32 datalen = smsg->getDataLen();
                        if ( smsg->getDataArraySize() >= datalen) {
                            for (uint32 i = 0; i < datalen; i++) {
                                dc->user_data[i] = smsg->getData(i);
                            }
                        }
                        writtenbytes += ADD_PADDING(datalen);
                    break;
                }
                case INIT:
                {
                    //sctpEV3<<"serialize INIT sizeKeyVector="<<sizeKeyVector<<"\n";
                    // source data from internal struct:
                    SCTPInitChunk *initChunk = check_and_cast<SCTPInitChunk *>(chunk);
                    //sctpEV3<<simulation.simTime()<<" SCTPAssociation:: Init sent \n";
                    // destination is send buffer:
                    struct init_chunk *ic = (struct init_chunk*) (buf + writtenbytes); // append data to buffer
                    //buflen += (initChunk->getBitLength() / 8);

                    // fill buffer with data from SCTP init chunk structure
                    ic->type = initChunk->getChunkType();
                    ic->flags = 0; // no flags available in this type of SCTPChunk
                    ic->initiate_tag = htonl(initChunk->getInitTag());
                    ic->a_rwnd = htonl(initChunk->getA_rwnd());
                    ic->mos = htons(initChunk->getNoOutStreams());
                    ic->mis = htons(initChunk->getNoInStreams());
                    ic->initial_tsn = htonl(initChunk->getInitTSN());
                    int32 parPtr = 0;
                    // Var.-Len. Parameters
                    struct supported_address_types_parameter* sup_addr = (struct supported_address_types_parameter*) (((unsigned char *)ic) + size_init_chunk + parPtr);
                    sup_addr->type = htons(INIT_SUPPORTED_ADDRESS);
                    sup_addr->length = htons(6);
                    sup_addr->address_type = htons(INIT_PARAM_IPV4);
                    parPtr += 8;
                    int32 numaddr = initChunk->getAddressesArraySize();
                    for (int32 i=0; i<numaddr; i++)
                    {
                            struct init_ipv4_address_parameter *ipv4addr = (struct init_ipv4_address_parameter*) (((unsigned char *)ic) + size_init_chunk + parPtr);
                            ipv4addr->type = htons(INIT_PARAM_IPV4);
                            ipv4addr->length = htons(8);
                            ipv4addr->address = htonl(initChunk->getAddresses(i).get4().getInt());
                            parPtr += sizeof(struct init_ipv4_address_parameter);
                    }
                    ic->length = htons(SCTP_INIT_CHUNK_LENGTH+parPtr);
                    writtenbytes += SCTP_INIT_CHUNK_LENGTH+parPtr;
                    break;
                }
                case INIT_ACK:
                {
                    //sctpEV3<<"serialize INIT_ACK sizeKeyVector="<<sizeKeyVector<<"\n";
                    SCTPInitAckChunk *initAckChunk = check_and_cast<SCTPInitAckChunk *>(chunk);
                    //sctpEV3<<simulation.simTime()<<" SCTPAssociation:: InitAck sent \n";
                    // destination is send buffer:
                    struct init_ack_chunk *iac = (struct init_ack_chunk*) (buf + writtenbytes); // append data to buffer
                    // fill buffer with data from SCTP init ack chunk structure
                    iac->type = initAckChunk->getChunkType();
//                  iac->flags = initAckChunk->getFlags(); // no flags available in this type of SCTPChunk
                    iac->initiate_tag = htonl(initAckChunk->getInitTag());
                    iac->a_rwnd = htonl(initAckChunk->getA_rwnd());
                    iac->mos = htons(initAckChunk->getNoOutStreams());
                    iac->mis = htons(initAckChunk->getNoInStreams());
                    iac->initial_tsn = htonl(initAckChunk->getInitTSN());
                    // Var.-Len. Parameters
                    int32 parPtr = 0;

                    struct supported_address_types_parameter* sup_addr = (struct supported_address_types_parameter*) (((unsigned char *)iac) + size_init_chunk + parPtr);
                    sup_addr->type = htons(INIT_SUPPORTED_ADDRESS);
                    sup_addr->length = htons(6);
                    sup_addr->address_type = htons(INIT_PARAM_IPV4);
                    parPtr += 8;


                    int32 numaddr = initAckChunk->getAddressesArraySize();
                    for (int32 i=0; i<numaddr; i++)
                    {
                        struct init_ipv4_address_parameter *ipv4addr = (struct init_ipv4_address_parameter*) (((unsigned char *)iac) + size_init_chunk + parPtr);
                        ipv4addr->type = htons(INIT_PARAM_IPV4);
                        ipv4addr->length = htons(8);
                        ipv4addr->address = htonl(initAckChunk->getAddresses(i).get4().getInt());
                        parPtr += 8;
                    }
                    uint32 uLen = initAckChunk->getUnrecognizedParametersArraySize();
                    if (uLen>0)
                    {
                        //sctpEV3<<"uLen="<<uLen<<"\n";
                        int32 k = 0;
                        uint32 pLen = 0;
                        while (uLen>0)
                        {
                            struct tlv* unknown = (struct tlv*) (((unsigned char *)iac) + size_init_chunk + parPtr);
                            unknown->type = htons(UNRECOGNIZED_PARAMETER);
                            pLen = initAckChunk->getUnrecognizedParameters(k+2)*16+initAckChunk->getUnrecognizedParameters(k+3);
                            unknown->length = htons(pLen+4);
                            //sctpEV3<<"unknown->length="<<pLen<<"\n";
                            for (uint32 i=0; i<ADD_PADDING(pLen); i++, k++)
                                unknown->value[i] = initAckChunk->getUnrecognizedParameters(k);
                            parPtr += ADD_PADDING(pLen+4);
                            uLen -= ADD_PADDING(pLen);
                        }
                    }

                    /*if(cookielen == 0)
                    {
                        cookielen = 4;
                        initAckChunk->setCookieArraySize(cookielen);
                        initAckChunk->setCookie(0, '1');
                        initAckChunk->setCookie(1, '3');
                        initAckChunk->setCookie(2, '3');
                        initAckChunk->setCookie(3, '7');
                        iac->length = htons(ntohs(iac->length) + 8);
                    }*/
                    int32 cookielen = initAckChunk->getCookieArraySize();
                    if (cookielen == 0)
                    {
                        SCTPCookie* stateCookie = check_and_cast<SCTPCookie*>(initAckChunk->getStateCookie());
                        struct init_cookie_parameter *cookie = (struct init_cookie_parameter*) (((unsigned char *)iac) + size_init_chunk + parPtr);
                        cookie->type = htons(INIT_PARAM_COOKIE);
                        cookie->length = htons(SCTP_COOKIE_LENGTH + 4);
                        cookie->creationTime = htonl((uint32)stateCookie->getCreationTime().dbl());
                        cookie->localTag = htonl(stateCookie->getLocalTag());
                        cookie->peerTag = htonl(stateCookie->getPeerTag());
                        for (int32 i=0; i<32; i++)
                        {
                            cookie->localTieTag[i] = stateCookie->getLocalTieTag(i);
                            cookie->peerTieTag[i] = stateCookie->getPeerTieTag(i);
                        }
                        parPtr += (SCTP_COOKIE_LENGTH + 4);
                    }
                    else
                    {
                        struct tlv *cookie = (struct tlv*) (((unsigned char *)iac) + size_init_chunk + parPtr);
                        cookie->type = htons(INIT_PARAM_COOKIE);
                        cookie->length = htons(cookielen+4);
                        for (int32 i=0; i<cookielen; i++)
                            cookie->value[i] = initAckChunk->getCookie(i);
                        parPtr += cookielen + 4;
                    }
                    iac->length = htons(SCTP_INIT_CHUNK_LENGTH+parPtr);
                    writtenbytes += SCTP_INIT_CHUNK_LENGTH+parPtr;
                    break;
                }
                case SACK:
                {
                    SCTPSackChunk *sackChunk = check_and_cast<SCTPSackChunk *>(chunk);

                    // destination is send buffer:
                    struct sack_chunk *sac = (struct sack_chunk*) (buf + writtenbytes); // append data to buffer
                    writtenbytes += (sackChunk->getBitLength() / 8);

                    // fill buffer with data from SCTP init ack chunk structure
                    sac->type = sackChunk->getChunkType();
//                  sac->flags = sackChunk->getFlags(); // no flags available in this type of SCTPChunk
                    sac->length = htons(sackChunk->getBitLength() / 8);
                    uint32 cumtsnack = sackChunk->getCumTsnAck();
                    sac->cum_tsn_ack = htonl(cumtsnack);
                    sac->a_rwnd = htonl(sackChunk->getA_rwnd());
                    sac->nr_of_gaps = htons(sackChunk->getNumGaps());
                    sac->nr_of_dups = htons(sackChunk->getNumDupTsns());

                    // GAPs and Dup. TSNs:
                    int32 numgaps = sackChunk->getNumGaps();
                    int32 numdups = sackChunk->getNumDupTsns();
                    for (int32 i=0; i<numgaps; i++)
                    {
                        struct sack_gap *gap = (struct sack_gap*) (((unsigned char *)sac) + size_sack_chunk + i*sizeof(struct sack_gap));
                        gap->start = htons(sackChunk->getGapStart(i) - cumtsnack);
                        gap->stop = htons(sackChunk->getGapStop(i) - cumtsnack);
                    }
                    for (int32 i=0; i<numdups; i++)
                    {
                        struct sack_duptsn *dup = (struct sack_duptsn*) (((unsigned char *)sac) + size_sack_chunk + numgaps*sizeof(struct sack_gap) + i*sizeof(sack_duptsn));
                        dup->tsn = htonl(sackChunk->getDupTsns(i));
                    }
                    break;
                }
                case HEARTBEAT:
                {
                    //sctpEV3<<simulation.simTime()<<"  SCTPAssociation:: Heartbeat sent \n";
                    SCTPHeartbeatChunk *heartbeatChunk = check_and_cast<SCTPHeartbeatChunk *>(chunk);

                    // destination is send buffer:
                    struct heartbeat_chunk *hbc = (struct heartbeat_chunk*) (buf + writtenbytes); // append data to buffer
                    writtenbytes += (heartbeatChunk->getBitLength() / 8);

                    // fill buffer with data from SCTP init ack chunk structure
                    hbc->type = heartbeatChunk->getChunkType();
//                  hbc->flags = heartbeatChunk->getFlags(); // no flags available in this type of SCTPChunk
                    hbc->length = htons(heartbeatChunk->getBitLength() / 8);

                    // deliver info:
                    struct heartbeat_info *hbi = (struct heartbeat_info*) (((unsigned char*)hbc) + size_heartbeat_chunk);
                    IPvXAddress addr = heartbeatChunk->getRemoteAddr();
                    simtime_t time = heartbeatChunk->getTimeField();
                    int32 infolen = sizeof(addr.get4().getInt()) + sizeof(uint32);
                    hbi->type = htons(1);   // mandatory
                    hbi->length = htons(infolen+4);
                    HBI_ADDR(hbi) = htonl(addr.get4().getInt());
                    HBI_TIME(hbi) = htonl((uint32)time.dbl());
                    break;
                }
                case HEARTBEAT_ACK:
                {
                    //sctpEV3<<simulation.simTime()<<" SCTPAssociation:: HeartbeatAck sent \n";
                    SCTPHeartbeatAckChunk *heartbeatAckChunk = check_and_cast<SCTPHeartbeatAckChunk *>(chunk);

                    // destination is send buffer:
                    struct heartbeat_ack_chunk *hbac = (struct heartbeat_ack_chunk*) (buf + writtenbytes); // append data to buffer
                    writtenbytes += (heartbeatAckChunk->getBitLength() / 8);

                    // fill buffer with data from SCTP init ack chunk structure
                    hbac->type = heartbeatAckChunk->getChunkType();
//                  hbac->flags = heartbeatAckChunk->getFlags(); // no flags available in this type of SCTPChunk
                    hbac->length = htons(heartbeatAckChunk->getBitLength() / 8);

                    // deliver info:
                    struct heartbeat_info *hbi = (struct heartbeat_info*) (((unsigned char*)hbac) + size_heartbeat_ack_chunk);
                    int32 infolen = heartbeatAckChunk->getInfoArraySize();
                    hbi->type = htons(1); //mandatory
                    if (infolen > 0)
                    {
                        hbi->length = htons(infolen+4);
                        for (int32 i=0; i<infolen; i++)
                        {
                            HBI_INFO(hbi)[i] = heartbeatAckChunk->getInfo(i);
                        }
                    }
                    else
                    {
                        IPvXAddress addr = heartbeatAckChunk->getRemoteAddr();
                        infolen = sizeof(addr.get4().getInt()) + sizeof(uint32);
                        hbi->type = htons(1);   // mandatory
                        hbi->length = htons(infolen+4);

                        simtime_t time = heartbeatAckChunk->getTimeField();
                        HBI_ADDR(hbi) = htonl(addr.get4().getInt());
                        HBI_TIME(hbi) = htonl((uint32)time.dbl());
                    }
                    break;
                }
                case ABORT:
                {
                    //sctpEV3<<simulation.simTime()<<" SCTPAssociation:: Abort sent \n";
                    SCTPAbortChunk *abortChunk = check_and_cast<SCTPAbortChunk *>(chunk);

                    // destination is send buffer:
                    struct abort_chunk *ac = (struct abort_chunk*) (buf + writtenbytes); // append data to buffer
                    writtenbytes += (abortChunk->getBitLength() / 8);

                    // fill buffer with data from SCTP init ack chunk structure
                    ac->type = abortChunk->getChunkType();
                    unsigned char flags = 0;
                    if (abortChunk->getT_Bit())
                        flags |= T_BIT;
                    ac->flags = flags;
                    ac->length = htons(abortChunk->getBitLength() / 8);
                    break;
                }
                case COOKIE_ECHO:
                {
                    //sctpEV3<<simulation.simTime()<<" SCTPAssociation:: CookieEcho sent \n";
                    SCTPCookieEchoChunk *cookieChunk = check_and_cast<SCTPCookieEchoChunk *>(chunk);

                    struct cookie_echo_chunk *cec = (struct cookie_echo_chunk*) (buf + writtenbytes);


                    cec->type = cookieChunk->getChunkType();
                    cec->length = htons(cookieChunk->getBitLength() / 8);
                    int32 cookielen = cookieChunk->getCookieArraySize();
                    if (cookielen>0)
                    {
                        for (int32 i=0; i<cookielen; i++)
                            cec->state_cookie[i] = cookieChunk->getCookie(i);
                    }
                    else
                    {
                        SCTPCookie* stateCookie = check_and_cast<SCTPCookie*>(cookieChunk->getStateCookie());
                        struct cookie_parameter *cookie = (struct cookie_parameter*) (buf + writtenbytes+4);
                        cookie->creationTime = htonl((uint32)stateCookie->getCreationTime().dbl());
                        cookie->localTag = htonl(stateCookie->getLocalTag());
                        cookie->peerTag = htonl(stateCookie->getPeerTag());
                        for (int32 i=0; i<32; i++)
                        {
                            cookie->localTieTag[i] = stateCookie->getLocalTieTag(i);
                            cookie->peerTieTag[i] = stateCookie->getPeerTieTag(i);
                        }
                    }
                    writtenbytes += (ADD_PADDING(cookieChunk->getBitLength() / 8));
                    //sctpEV3<<"buflen cookie_echo="<<buflen<<"\n";
                    uint32 uLen = cookieChunk->getUnrecognizedParametersArraySize();
                    if (uLen>0)
                    {
                        //sctpEV3<<"uLen="<<uLen<<"\n";
                        struct error_chunk* error = (struct error_chunk*)(buf + writtenbytes);
                        error->type = ERRORTYPE;
                        error->flags = 0;
                        int32 k = 0;
                        uint32 pLen = 0;
                        uint32 ecLen = SCTP_ERROR_CHUNK_LENGTH;
                        uint32 ecParPtr = 0;
                        while (uLen>0)
                        {
                            struct tlv* unknown = (struct tlv*) (((unsigned char *)error) + sizeof(struct error_chunk) + ecParPtr);
                            unknown->type = htons(UNRECOGNIZED_PARAMETER);
                            pLen = cookieChunk->getUnrecognizedParameters(k+2)*16+cookieChunk->getUnrecognizedParameters(k+3);
                            unknown->length = htons(pLen+4);
                            ecLen += pLen+4;
                            //sctpEV3<<"plength="<<pLen<<" ecLen="<<ecLen<<"\n";
                            for (uint32 i=0; i<ADD_PADDING(pLen); i++, k++)
                                unknown->value[i] = cookieChunk->getUnrecognizedParameters(k);
                            ecParPtr += ADD_PADDING(pLen+4);
                            //sctpEV3<<"ecParPtr="<<ecParPtr<<"\n";
                            uLen -= ADD_PADDING(pLen);
                        }
                        error->length = htons(ecLen);
                        writtenbytes += SCTP_ERROR_CHUNK_LENGTH+ecParPtr;
                    }

                    break;
                }
                case COOKIE_ACK:
                {
                    //sctpEV3<<simulation.simTime()<<" SCTPAssociation:: CookieAck sent \n";
                    SCTPCookieAckChunk *cookieAckChunk = check_and_cast<SCTPCookieAckChunk *>(chunk);

                    struct cookie_ack_chunk *cac = (struct cookie_ack_chunk*) (buf + writtenbytes);
                    writtenbytes += (cookieAckChunk->getBitLength() / 8);

                    cac->type = cookieAckChunk->getChunkType();
                    cac->length = htons(cookieAckChunk->getBitLength() / 8);

                    break;
                }
                case SHUTDOWN:
                {
                    //sctpEV3<<simulation.simTime()<<" SCTPAssociation:: ShutdownAck sent \n";
                    SCTPShutdownChunk *shutdownChunk = check_and_cast<SCTPShutdownChunk *>(chunk);

                    struct shutdown_chunk *sac = (struct shutdown_chunk*) (buf + writtenbytes);
                    writtenbytes += (shutdownChunk->getBitLength() / 8);

                    sac->type = shutdownChunk->getChunkType();
                    sac->cumulative_tsn_ack = htonl(shutdownChunk->getCumTsnAck());
                    sac->length = htons(shutdownChunk->getBitLength() / 8);

                    break;
                }
                case SHUTDOWN_ACK:
                {
                    //sctpEV3<<simulation.simTime()<<" SCTPAssociation:: ShutdownAck sent \n";
                    SCTPShutdownAckChunk *shutdownAckChunk = check_and_cast<SCTPShutdownAckChunk *>(chunk);

                    struct shutdown_ack_chunk *sac = (struct shutdown_ack_chunk*) (buf + writtenbytes);
                    writtenbytes += (shutdownAckChunk->getBitLength() / 8);

                    sac->type = shutdownAckChunk->getChunkType();
                    sac->length = htons(shutdownAckChunk->getBitLength() / 8);

                    break;
                }
                case SHUTDOWN_COMPLETE:
                {
                    //sctpEV3<<simulation.simTime()<<" SCTPAssociation:: ShutdownAck sent \n";
                    SCTPShutdownCompleteChunk *shutdownCompleteChunk = check_and_cast<SCTPShutdownCompleteChunk *>(chunk);

                    struct shutdown_complete_chunk *sac = (struct shutdown_complete_chunk*) (buf + writtenbytes);
                    writtenbytes += (shutdownCompleteChunk->getBitLength() / 8);

                    sac->type = shutdownCompleteChunk->getChunkType();
                    sac->length = htons(shutdownCompleteChunk->getBitLength() / 8);
                    unsigned char flags = 0;
                    if (shutdownCompleteChunk->getTBit())
                        flags |= T_BIT;
                    sac->flags = flags;
                    break;
                }
                case ERRORTYPE:
                {
                    SCTPErrorChunk* errorchunk = check_and_cast<SCTPErrorChunk*>(chunk);
                    struct error_chunk* error = (struct error_chunk*)(buf + writtenbytes);
                    error->type = errorchunk->getChunkType();
                    error->flags = 0;

                    if (errorchunk->getParametersArraySize()>0)
                    {
                        writtenbytes += size_chunk;
                    }
                    else
                        writtenbytes += ADD_PADDING(error->length);
                    break;
                }
                default:
                    printf("Serialize TODO: Implement for outgoing chunk type %d!\n", chunkType);
                    throw new cRuntimeError("TODO: unknown chunktype in outgoing packet on external interface! Implement it!");
                // break;

            }

            /*drop(chunk);
            delete chunk;*/

        }
    // finally, set the CRC32 checksum field in the SCTP common header

    /*sctpEV3<<"srcport="<<msg->getSrcPort() <<"destport="<<msg->getDestPort() <<"writtenbytes vor checksum="<<writtenbytes<<"\n";*/

    ch->checksum = checksum((unsigned char*)ch, writtenbytes);
    return writtenbytes;
}

uint32 SCTPSerializer::checksum(const uint8_t *buf, register uint32 len)
{
    uint32 h;
    unsigned char byte0, byte1, byte2, byte3;
    uint32 crc32c;
    register uint32 i;
    register uint32 res = (~0L);
    for (i = 0; i < len; i++)
      CRC32C(res, buf[i]);
    h = ~res;
    byte0 = h & 0xff;
    byte1 = (h>>8) & 0xff;
    byte2 = (h>>16) & 0xff;
    byte3 = (h>>24) & 0xff;
    crc32c = ((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | byte3);
    return htonl(crc32c);
}

void SCTPSerializer::parse(const uint8_t *buf, uint32 bufsize, SCTPMessage *dest)
{
    int32 size_common_header = sizeof(struct common_header);
    int32 size_init_chunk = sizeof(struct init_chunk);
    int32 size_init_ack_chunk = sizeof(struct init_ack_chunk);
    int32 size_data_chunk = sizeof(struct data_chunk);
    int32 size_sack_chunk = sizeof(struct sack_chunk);
    int32 size_heartbeat_chunk = sizeof(struct heartbeat_chunk);
    int32 size_heartbeat_ack_chunk = sizeof(struct heartbeat_ack_chunk);
    int32 size_abort_chunk = sizeof(struct abort_chunk);
    int32 size_cookie_echo_chunk = sizeof(struct cookie_echo_chunk);
    uint16 paramType;
    int32 parptr, chunklen, cLen, woPadding;
    struct common_header *common_header = (struct common_header*) (buf);
    int32 tempChecksum = common_header->checksum;
    common_header->checksum = 0;
    int32 chksum = checksum((unsigned char*)common_header, bufsize);
    common_header->checksum = tempChecksum;

    const unsigned char *chunks = (unsigned char*) (buf + size_common_header);
    sctpEV3<<"SCTPSerializer::parse SCTPMessage\n";
    if (tempChecksum == chksum)
        dest->setChecksumOk(true);
    else
        dest->setChecksumOk(false);
    sctpEV3<<"checksumOK="<<dest->getChecksumOk()<<"\n";
    dest->setSrcPort(ntohs(common_header->source_port));
    dest->setDestPort(ntohs(common_header->destination_port));
    dest->setTag(ntohl(common_header->verification_tag));
    dest->setBitLength(SCTP_COMMON_HEADER*8);
    // chunks
    uint32 chunkPtr = 0;

    // catch ALL chunks - when a chunk is taken, the chunkPtr is set to the next chunk
    while (chunkPtr < (bufsize - size_common_header))
    {
        const struct chunk * chunk = (struct chunk*)(chunks + chunkPtr);
        int32 chunkType = chunk->type;
sctpEV3<<"chunkType="<<chunkType<<"\n";
        woPadding = ntohs(chunk->length);
sctpEV3<<"chunk->length="<<ntohs(chunk->length)<<"\n";
        cLen = ADD_PADDING(woPadding);
        switch (chunkType)
        {
            case DATA:
            {
                ev<<"Data received\n";
                const struct data_chunk *dc = (struct data_chunk*) (chunks + chunkPtr);
                sctpEV3<<"cLen="<<cLen<<"\n";
                if (cLen == 0)
                    throw new cRuntimeError("Incoming SCTP packet contains data chunk with length==0");
                SCTPDataChunk *chunk = new SCTPDataChunk("DATA");
                chunk->setChunkType(chunkType);
                chunk->setUBit(dc->flags & UNORDERED_BIT);
                chunk->setBBit(dc->flags & BEGIN_BIT);
                chunk->setEBit(dc->flags & END_BIT);
                chunk->setTsn(ntohl(dc->tsn));
                chunk->setSid(ntohs(dc->sid));
                chunk->setSsn(ntohs(dc->ssn));
                chunk->setPpid(ntohl(dc->ppi));
                chunk->setBitLength(SCTP_DATA_CHUNK_LENGTH*8);
                sctpEV3<<"parse data: woPadding="<<woPadding<<" size_data_chunk="<<size_data_chunk<<"\n";
                if (woPadding > size_data_chunk)
                {
                    SCTPSimpleMessage* msg = new SCTPSimpleMessage("data");
                    int32 datalen = (woPadding - size_data_chunk);
                    msg->setBitLength(datalen*8);
                    msg->setDataLen(datalen);
                    msg->setDataArraySize(datalen);
                    for (int32 i=0; i<datalen; i++)
                        msg->setData(i, dc->user_data[i]);

                    chunk->encapsulate(msg);
                }
                sctpEV3<<"datachunkLength="<<chunk->getBitLength()<<"\n";
                dest->addChunk(chunk);
                break;
            }
            case INIT:
            {
                ev<<"parse INIT\n";
                const struct init_chunk *init_chunk = (struct init_chunk*) (chunks + chunkPtr); // (recvBuffer + size_ip + size_common_header);
                chunklen = SCTP_INIT_CHUNK_LENGTH;
                SCTPInitChunk *chunk = new SCTPInitChunk("INIT");
                chunk->setChunkType(chunkType);
                chunk->setName("INIT");
                chunk->setInitTag(ntohl(init_chunk->initiate_tag));
                chunk->setA_rwnd(ntohl(init_chunk->a_rwnd));
                chunk->setNoOutStreams(ntohs(init_chunk->mos));
                chunk->setNoInStreams(ntohs(init_chunk->mis));
                chunk->setInitTSN(ntohl(init_chunk->initial_tsn));
                chunk->setAddressesArraySize(0);
                chunk->setUnrecognizedParametersArraySize(0);
                //sctpEV3<<"INIT arrived from wire\n";
                if (cLen > size_init_chunk)
                {
                    int32 parcounter = 0, addrcounter = 0;
                    parptr = 0;
                    bool stopProcessing = false;
                    while (cLen > size_init_chunk+parptr && !stopProcessing)
                    {
                        sctpEV3<<"Process INIT parameters\n";
                        const struct tlv *parameter = (struct tlv*)(((unsigned char*)init_chunk) + size_init_chunk + parptr);
                        paramType = ntohs(parameter->type);
                        sctpEV3<<"search for param "<<paramType<<"\n";
                        switch (paramType)
                        {
                            case SUPPORTED_ADDRESS_TYPES:
                            {
                                break;
                            }
                            case INIT_PARAM_IPV4:
                            {
                                // we supppose an ipv4 address parameter
                                sctpEV3<<"IPv4\n";
                                const struct init_ipv4_address_parameter *v4addr;
                                v4addr = (struct init_ipv4_address_parameter*) (((unsigned char*)init_chunk) + size_init_chunk + parptr);
                                chunk->setAddressesArraySize(++addrcounter);
                                IPvXAddress localv4Addr(IPv4Address(ntohl(v4addr->address)));
                                chunk->setAddresses(addrcounter-1, localv4Addr);
                                chunklen += 8;
                                break;
                            }
                            case INIT_PARAM_IPV6:
                            {
                                sctpEV3<<"IPv6\n";
                                const struct init_ipv6_address_parameter *ipv6addr;
                                ipv6addr = (struct init_ipv6_address_parameter*) (((unsigned char*)init_chunk) + size_init_chunk + parptr);
                                IPv6Address ipv6Addr = IPv6Address(ipv6addr->address[0], ipv6addr->address[1],
                                                                    ipv6addr->address[2], ipv6addr->address[3]);
                                IPvXAddress localv6Addr(ipv6Addr);
                                sctpEV3<<"address"<<ipv6Addr<<"\n";
                                chunk->setAddressesArraySize(++addrcounter);
                                chunk->setAddresses(addrcounter-1, localv6Addr);
                                chunklen += 20;
                                break;
                            }
                            default:
                            {
                                sctpEV3 << "ExtInterface: Unknown SCTP INIT parameter type " << paramType<<"\n";
                                uint16 skip = (paramType & 0x8000) >> 15;
                                if (skip == 0)
                                    stopProcessing = true;
                                uint16 report = (paramType & 0x4000) >> 14;
                                if (report != 0)
                                {
                                    const struct tlv *unknown;
                                    unknown = (struct tlv*) (((unsigned char*)init_chunk) + size_init_chunk + parptr);
                                    uint32 unknownLen = chunk->getUnrecognizedParametersArraySize();
                                    chunk->setUnrecognizedParametersArraySize(unknownLen+ADD_PADDING(ntohs(unknown->length)));
                                    struct data_vector* dv = (struct data_vector*) (((unsigned char*)init_chunk) + size_init_chunk + parptr);

                                    for (uint32 i=unknownLen; i<unknownLen+ADD_PADDING(ntohs(unknown->length)); i++)
                                        chunk->setUnrecognizedParameters(i, dv->data[i-unknownLen]);
                                }
                                sctpEV3<<"stopProcessing="<<stopProcessing<<" report="<<report<<"\n";
                                break;
                            }
                        }
                        parptr += ADD_PADDING(ntohs(parameter->length));
                        parcounter++;
                    }


                }
                chunk->setBitLength(chunklen*8);
                dest->addChunk(chunk);
                //chunkPtr += cLen;
                break;
            }
            case INIT_ACK:
            {
                const struct init_ack_chunk *iac = (struct init_ack_chunk*) (chunks + chunkPtr);
                chunklen = SCTP_INIT_CHUNK_LENGTH;
                SCTPInitAckChunk *chunk = new SCTPInitAckChunk("INIT_ACK");
                chunk->setChunkType(chunkType);
                chunk->setInitTag(ntohl(iac->initiate_tag));
                chunk->setA_rwnd(ntohl(iac->a_rwnd));
                chunk->setNoOutStreams(ntohs(iac->mos));
                chunk->setNoInStreams(ntohs(iac->mis));
                chunk->setInitTSN(ntohl(iac->initial_tsn));
                chunk->setUnrecognizedParametersArraySize(0);
                if (cLen > size_init_ack_chunk)
                {
                    int32 parcounter = 0, addrcounter = 0;
                    parptr = 0;
                    bool stopProcessing = false;
                    //sctpEV3<<"cLen="<<cLen<<"\n";
                    while (cLen > size_init_ack_chunk+parptr && !stopProcessing)
                    {
                        const struct tlv *parameter = (struct tlv*)(((unsigned char*)iac) + size_init_ack_chunk + parptr);
                        paramType = ntohs(parameter->type);
                        //sctpEV3<<"ParamType = "<<paramType<<" parameterLength="<<ntohs(parameter->length)<<"\n";
                        switch (paramType)
                        {
                            case SUPPORTED_ADDRESS_TYPES:
                            {
                                break;
                            }
                            case INIT_PARAM_IPV4:
                            {
                                sctpEV3<<"parse IPv4\n";
                                const struct init_ipv4_address_parameter *v4addr;
                                v4addr = (struct init_ipv4_address_parameter*) (((unsigned char*)iac) + size_init_ack_chunk + parptr);
                                chunk->setAddressesArraySize(++addrcounter);
                                IPvXAddress localv4Addr(IPv4Address(ntohl(v4addr->address)));
                                chunk->setAddresses(addrcounter-1, localv4Addr);
                                chunklen += 8;
                                break;
                            }
                            case INIT_PARAM_IPV6:
                            {
                                sctpEV3<<"IPv6\n";
                                const struct init_ipv6_address_parameter *ipv6addr;
                                ipv6addr = (struct init_ipv6_address_parameter*) (((unsigned char*)iac) + size_init_chunk + parptr);
                                IPv6Address ipv6Addr = IPv6Address(ipv6addr->address[0], ipv6addr->address[1],
                                                                    ipv6addr->address[2], ipv6addr->address[3]);
                                sctpEV3<<"address"<<ipv6Addr<<"\n";
                                IPvXAddress localv6Addr(ipv6Addr);

                                chunk->setAddressesArraySize(++addrcounter);
                                chunk->setAddresses(addrcounter-1, localv6Addr);
                                chunklen += 20;
                                break;
                            }
                            case INIT_PARAM_COOKIE:
                            {
                                const struct tlv *cookie = (struct tlv*) (((unsigned char*)iac) + size_init_ack_chunk + parptr);
                                int32 cookieLen = ntohs(cookie->length) - 4;
                                // put cookie data into chunk (char array cookie)
                                chunk->setCookieArraySize(cookieLen);
                                for (int32 i=0; i<cookieLen; i++)
                                    chunk->setCookie(i, cookie->value[i]);
                                chunklen += cookieLen+4;
                                break;
                            }
                            default:
                            {
                                sctpEV3 << "ExtInterface: Unknown SCTP INIT-ACK parameter type " << paramType<<"\n";
                                uint16 skip = (paramType & 0x8000) >> 15;
                                if (skip == 0)
                                    stopProcessing = true;
                                uint16 report = (paramType & 0x4000) >> 14;
                                if (report != 0)
                                {
                                    const struct tlv *unknown;
                                    unknown = (struct tlv*) (((unsigned char*)iac) + size_init_ack_chunk + parptr);
                                    uint32 unknownLen = chunk->getUnrecognizedParametersArraySize();
                                    chunk->setUnrecognizedParametersArraySize(unknownLen+ADD_PADDING(ntohs(unknown->length)));
                                    struct data_vector* dv = (struct data_vector*) (((unsigned char*)iac) + size_init_ack_chunk + parptr);

                                    for (uint32 i=unknownLen; i<unknownLen+ADD_PADDING(ntohs(unknown->length)); i++)
                                        chunk->setUnrecognizedParameters(i, dv->data[i-unknownLen]);
                                }
                                sctpEV3<<"stopProcessing="<<stopProcessing<<"  report="<<report<<"\n";

                                break;
                            }
                        }
                        parptr += ADD_PADDING(ntohs(parameter->length));
                        //sctpEV3<<"parptr="<<parptr<<"\n";
                        parcounter++;
                    }
                }
                chunk->setBitLength(chunklen*8);
                dest->addChunk(chunk);
                break;
            }
            case SACK:
            {
                ev<<"SCTPMessage: SACK received\n";
                const struct sack_chunk *sac = (struct sack_chunk*) (chunks + chunkPtr);
                SCTPSackChunk *chunk = new SCTPSackChunk("SACK");
                chunk->setChunkType(chunkType);
                uint32 cumtsnack = ntohl(sac->cum_tsn_ack);
                chunk->setCumTsnAck(cumtsnack);
                chunk->setA_rwnd(ntohl(sac->a_rwnd));

                int32 ngaps = ntohs(sac->nr_of_gaps);
                int32 ndups = ntohs(sac->nr_of_dups);
                chunk->setNumGaps(ngaps);
                chunk->setNumDupTsns(ndups);

                chunk->setGapStartArraySize(ngaps);
                chunk->setGapStopArraySize(ngaps);
                chunk->setDupTsnsArraySize(ndups);

                for (int32 i=0; i<ngaps; i++)
                {
                    const struct sack_gap *gap = (struct sack_gap*) (((unsigned char*)sac) + size_sack_chunk + i*sizeof(sack_gap));
                    chunk->setGapStart(i, ntohs(gap->start) + cumtsnack);
                    chunk->setGapStop(i, ntohs(gap->stop) + cumtsnack);
                }
                for (int32 i=0; i<ndups; i++)
                {
                    const struct sack_duptsn *dup = (struct sack_duptsn*) (((unsigned char*)sac) + size_sack_chunk + ngaps*sizeof(sack_gap) + i*sizeof(sack_duptsn));
                    chunk->setDupTsns(i, ntohl(dup->tsn));
                }

                chunk->setBitLength(cLen*8);
                dest->addChunk(chunk);
                break;
            }
            case HEARTBEAT:
            {
                //sctpEV3<<"SCTPMessage: Heartbeat received\n";
                const struct heartbeat_chunk *hbc = (struct heartbeat_chunk*) (chunks + chunkPtr);
                SCTPHeartbeatChunk *chunk = new SCTPHeartbeatChunk("HEARTBEAT");
                chunk->setChunkType(chunkType);
                if (cLen > size_heartbeat_chunk)
                {
                    int32 parcounter = 0;
                    parptr = 0;
                    while (cLen > size_heartbeat_chunk+parptr)
                    {
                        // we supppose type 1 here
                        //sctpEV3<<"HB-chunk+parptr="<<size_heartbeat_chunk+parptr<<"\n";
                        const struct heartbeat_info *hbi = (struct heartbeat_info*) (((unsigned char*)hbc) + size_heartbeat_chunk + parptr);
                        if (ntohs(hbi->type) == 1) // sender specific hb info
                        {
                            //sctpEV3<<"HBInfo\n";
                            int32 infoLen = ntohs(hbi->length) - 4;
                            parptr += ADD_PADDING(infoLen) + 4;
                            parcounter++;
                            chunk->setInfoArraySize(infoLen);
                            for (int32 i=0; i<infoLen; i++)
                                chunk->setInfo(i, HBI_INFO(hbi)[i]);
                        }
                        else
                        {
                            parptr += ADD_PADDING(ntohs(hbi->length));  // set pointer forwards with count of bytes in length field of TLV
                            parcounter++;
                            continue;
                        }
                    }
                }
                chunk->setBitLength(cLen*8);
                dest->addChunk(chunk);
                break;
            }
            case HEARTBEAT_ACK:
            {
                ev<<"SCTPMessage: Heartbeat_Ack received\n";
                const struct heartbeat_ack_chunk *hbac = (struct heartbeat_ack_chunk*) (chunks + chunkPtr);
                SCTPHeartbeatAckChunk *chunk = new SCTPHeartbeatAckChunk("HEARTBEAT_ACK");
                chunk->setChunkType(chunkType);
                if (cLen>size_heartbeat_ack_chunk)
                {
                    int32 parcounter = 0;
                    parptr = 0;
                    while (cLen > size_heartbeat_ack_chunk+parptr)
                    {
                        // we supppose type 1 here, the same provided in heartbeat chunks
                        const struct heartbeat_info *hbi = (struct heartbeat_info*) (((unsigned char*)hbac) + size_heartbeat_ack_chunk + parptr);
                        if (ntohs(hbi->type) == 1) // sender specific hb info
                        {
                            parptr += sizeof(struct heartbeat_info);
                            parcounter++;
                            chunk->setRemoteAddr(IPvXAddress(IPv4Address(ntohl(HBI_ADDR(hbi)))));
                            chunk->setTimeField(ntohl((uint32)HBI_TIME(hbi)));
                        }
                        else
                        {
                            parptr += ntohs(hbi->length);   // set pointer forwards with count of bytes in length field of TLV
                            parcounter++;
                            continue;
                        }

                    }
                }
                chunk->setBitLength(cLen*8);
                dest->addChunk(chunk);
                break;
            }
            case ABORT:
            {
                ev<<"SCTPMessage: Abort received\n";
                const struct abort_chunk *ac = (struct abort_chunk*) (chunks + chunkPtr);
                cLen = ntohs(ac->length);
                SCTPAbortChunk *chunk = new SCTPAbortChunk("ABORT");
                chunk->setChunkType(chunkType);
                chunk->setT_Bit(ac->flags & T_BIT);
                if (cLen>size_abort_chunk)
                {
                    // TODO: handle attached error causes
                }
                chunk->setBitLength(cLen*8);
                dest->addChunk(chunk);
                break;
            }
            case COOKIE_ECHO:
            {
                SCTPCookieEchoChunk *chunk = new SCTPCookieEchoChunk("COOKIE_ECHO");
                chunk->setChunkType(chunkType);
                sctpEV3<<"Parse Cookie-Echo\n";
                if (cLen>size_cookie_echo_chunk)
                {
                    int32 cookieSize = woPadding-size_cookie_echo_chunk;
                    sctpEV3<<"cookieSize="<<cookieSize<<"\n";
                    const struct cookie_parameter *cookie = (struct cookie_parameter*)(chunks+chunkPtr+4);
                    SCTPCookie* stateCookie = new SCTPCookie();
                    stateCookie->setCreationTime(ntohl(cookie->creationTime));
                    stateCookie->setLocalTag(ntohl(cookie->localTag));
                    stateCookie->setPeerTag(ntohl(cookie->peerTag));
                    stateCookie->setLocalTieTagArraySize(32);
                    stateCookie->setPeerTieTagArraySize(32);
                    for (int32 i=0; i<32; i++)
                    {
                        stateCookie->setLocalTieTag(i, cookie->localTieTag[i]);
                        stateCookie->setPeerTieTag(i, cookie->peerTieTag[i]);
                    }
                    stateCookie->setBitLength(SCTP_COOKIE_LENGTH*8);
                    chunk->setStateCookie(stateCookie);
                }
                chunk->setBitLength(woPadding*8);
                dest->addChunk(chunk);
                break;
            }
            case COOKIE_ACK:
            {
                ev<<"SCTPMessage: Cookie_Ack received\n";
                SCTPCookieAckChunk *chunk = new SCTPCookieAckChunk("COOKIE_ACK");
                chunk->setChunkType(chunkType);
                chunk->setBitLength(cLen*8);
                dest->addChunk(chunk);
                break;
            }
            case SHUTDOWN:
            {
                ev<<"SCTPMessage: Shutdown received\n";
                const struct shutdown_chunk *sc = (struct shutdown_chunk*) (chunks + chunkPtr);
                SCTPShutdownChunk *chunk = new SCTPShutdownChunk("SHUTDOWN");
                chunk->setChunkType(chunkType);
                uint32 cumtsnack = ntohl(sc->cumulative_tsn_ack);
                chunk->setCumTsnAck(cumtsnack);
                chunk->setBitLength(cLen*8);
                dest->addChunk(chunk);
                break;
            }
            case SHUTDOWN_ACK:
            {
                ev<<"SCTPMessage: ShutdownAck received\n";
                SCTPShutdownAckChunk *chunk = new SCTPShutdownAckChunk("SHUTDOWN_ACK");
                chunk->setChunkType(chunkType);
                chunk->setBitLength(cLen*8);
                dest->addChunk(chunk);
                break;
            }
            case SHUTDOWN_COMPLETE:
            {
                ev<<"SCTPMessage: ShutdownComplete received\n";
                const struct shutdown_complete_chunk *scc = (struct shutdown_complete_chunk*) (chunks + chunkPtr);
                SCTPShutdownCompleteChunk *chunk = new SCTPShutdownCompleteChunk("SHUTDOWN_COMPLETE");
                chunk->setChunkType(chunkType);
                chunk->setTBit(scc->flags & T_BIT);
                chunk->setBitLength(cLen*8);
                dest->addChunk(chunk);
                break;
            }
            case ERRORTYPE:
            {
                const struct error_chunk *error;
                error = (struct error_chunk*) (chunks + chunkPtr);
                SCTPErrorChunk *errorchunk;
                errorchunk = new SCTPErrorChunk("ERROR");
                errorchunk->setChunkType(chunkType);
                errorchunk->setBitLength(SCTP_ERROR_CHUNK_LENGTH*8);
                parptr = 0;
                dest->addChunk(errorchunk);
                break;
            }
            default:
                //printf("TODO: Implement chunk type %d, found in chunk array on %d!\n", chunkType, ct);
                sctpEV3 << "Parser: Unknown SCTP chunk type " << chunkType;

                /*throw new cRuntimeError("TODO: unknown chunktype in incoming packet from external interface! Implement it!");*/

                break;
        }   // end of switch(chunkType)
        chunkPtr += cLen;
    }   // end of while()
}



