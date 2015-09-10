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

#include "inet/common/serializer/SerializerUtil.h"

#include "inet/common/serializer/headers/defs.h"

#include "inet/common/serializer/headers/bsdint.h"
#include "inet/common/serializer/headers/in_systm.h"
#include "inet/common/serializer/headers/in.h"
#include "inet/common/serializer/ipv4/headers/ip.h"
#include "inet/common/serializer/ipv4/IPv4Serializer.h"
#include "inet/common/serializer/sctp/headers/sctphdr.h"

#include "inet/common/serializer/sctp/SCTPSerializer.h"

#include "inet/transportlayer/contract/sctp/SCTPCommand_m.h"
#include "inet/transportlayer/sctp/SCTPAssociation.h"

#include "inet/networklayer/common/IPProtocolId_m.h"

#include "inet/networklayer/ipv4/IPv4Datagram.h"

#if !defined(_WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>    // htonl, ntohl, ...
#include <arpa/inet.h>
#include <sys/socket.h>
#endif // if !defined(_WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

#include <sys/types.h>

namespace inet {

using namespace sctp;

namespace serializer {

Register_Serializer(sctp::SCTPMessage, IP_PROT, IP_PROT_SCTP, SCTPSerializer);

unsigned char SCTPSerializer::keyVector[512];
unsigned int SCTPSerializer::sizeKeyVector = 0;
unsigned char SCTPSerializer::peerKeyVector[512];
unsigned int SCTPSerializer::sizePeerKeyVector = 0;
unsigned char SCTPSerializer::sharedKey[512];

void SCTPSerializer::serialize(const cPacket *pkt, Buffer &b, Context& context)
{
    int32 len = serialize(check_and_cast<const SCTPMessage *>(pkt), static_cast<unsigned char *>(b.accessNBytes(0)), b.getRemainingSize());
    b.accessNBytes(len);
}

cPacket* SCTPSerializer::deserialize(const Buffer &b, Context& context)
{
    SCTPMessage *dest = new SCTPMessage("parsed-sctp");
    parse(static_cast<const uint8 *>(b.accessNBytes(0)), b.getRemainingSize(), dest);
    b.accessNBytes(b.getRemainingSize());
    return dest;
}


int32 SCTPSerializer::serialize(const SCTPMessage *msg, unsigned char *buf, uint32 bufsize)
{
    int32 size_init_chunk = sizeof(struct init_chunk);
    int32 size_sack_chunk = sizeof(struct sack_chunk);
    int32 size_nr_sack_chunk = sizeof(struct nr_sack_chunk);
    int32 size_heartbeat_chunk = sizeof(struct heartbeat_chunk);
    int32 size_heartbeat_ack_chunk = sizeof(struct heartbeat_ack_chunk);
    // int32 size_chunk = sizeof(struct chunk);

    int authstart = 0;
    struct common_header *ch = (struct common_header *)(buf);
    uint32 writtenbytes = sizeof(struct common_header);

    // fill SCTP common header structure
    ch->source_port = htons(msg->getSrcPort());
    ch->destination_port = htons(msg->getDestPort());
    ch->verification_tag = htonl(msg->getTag());

    // SCTP chunks:
    int32 noChunks = msg->getChunksArraySize();
    for (int32 cc = 0; cc < noChunks; cc++) {
        SCTPChunk *chunk = const_cast<SCTPChunk *>(check_and_cast<const SCTPChunk *>(((SCTPMessage *)msg)->getChunks(cc)));
        unsigned char chunkType = chunk->getChunkType();
        switch (chunkType) {
            case DATA: {
                EV_INFO << simTime() << " SCTPAssociation:: Data sent \n";
                SCTPDataChunk *dataChunk = check_and_cast<SCTPDataChunk *>(chunk);
                struct data_chunk *dc = (struct data_chunk *)(buf + writtenbytes);    // append data to buffer
                unsigned char flags = 0;

                // fill buffer with data from SCTP data chunk structure
                dc->type = dataChunk->getChunkType();
                if (dataChunk->getUBit())
                    flags |= UNORDERED_BIT;
                if (dataChunk->getBBit())
                    flags |= BEGIN_BIT;
                if (dataChunk->getEBit())
                    flags |= END_BIT;
                if (dataChunk->getIBit())
                    flags |= I_BIT;
                dc->flags = flags;
                dc->length = htons(dataChunk->getByteLength());
                dc->tsn = htonl(dataChunk->getTsn());
                dc->sid = htons(dataChunk->getSid());
                dc->ssn = htons(dataChunk->getSsn());
                dc->ppi = htonl(dataChunk->getPpid());
                writtenbytes += SCTP_DATA_CHUNK_LENGTH;

                SCTPSimpleMessage *smsg = check_and_cast<SCTPSimpleMessage *>(dataChunk->getEncapsulatedPacket());
                const uint32 datalen = smsg->getDataLen();
                if (smsg->getDataArraySize() >= datalen) {
                    for (uint32 i = 0; i < datalen; i++) {
                        dc->user_data[i] = smsg->getData(i);
                    }
                }
                writtenbytes += ADD_PADDING(datalen);
                break;
            }

            case INIT: {
                EV_INFO << "serialize INIT sizeKeyVector=" << sizeKeyVector << "\n";
                // source data from internal struct:
                SCTPInitChunk *initChunk = check_and_cast<SCTPInitChunk *>(chunk);
                //sctpEV3<<simulation.simTime()<<" SCTPAssociation:: Init sent \n";
                // destination is send buffer:
                struct init_chunk *ic = (struct init_chunk *)(buf + writtenbytes);    // append data to buffer

                // fill buffer with data from SCTP init chunk structure
                ic->type = initChunk->getChunkType();
                ic->flags = 0;    // no flags available in this type of SCTPChunk
                ic->initiate_tag = htonl(initChunk->getInitTag());
                ic->a_rwnd = htonl(initChunk->getA_rwnd());
                ic->mos = htons(initChunk->getNoOutStreams());
                ic->mis = htons(initChunk->getNoInStreams());
                ic->initial_tsn = htonl(initChunk->getInitTSN());
                int32 parPtr = 0;
                // Var.-Len. Parameters
                if (initChunk->getIpv4Supported() || initChunk->getIpv6Supported()) {
                    struct supported_address_types_parameter *sup_addr = (struct supported_address_types_parameter *)(((unsigned char *)ic) + size_init_chunk + parPtr);
                    sup_addr->type = htons(INIT_SUPPORTED_ADDRESS);
                    sup_addr->length = htons(8);
                    if (initChunk->getIpv4Supported() && initChunk->getIpv6Supported()) {
                        sup_addr->address_type_1 = htons(INIT_PARAM_IPV4);
                        sup_addr->address_type_2 = htons(INIT_PARAM_IPV6);
                    } else if (initChunk->getIpv4Supported()) {
                        sup_addr->address_type_1 = htons(INIT_PARAM_IPV4);
                        sup_addr->address_type_2 = 0;
                    } else {
                        sup_addr->address_type_1 = htons(INIT_PARAM_IPV6);
                        sup_addr->address_type_2 = 0;
                    }
                    parPtr += 8;
                }
                if (initChunk->getForwardTsn() == true) {
                    struct forward_tsn_supported_parameter *forward = (struct forward_tsn_supported_parameter *)(((unsigned char *)ic) + size_init_chunk + parPtr);
                    forward->type = htons(FORWARD_TSN_SUPPORTED_PARAMETER);
                    forward->length = htons(4);
                    parPtr += 4;
                }
                int32 numaddr = initChunk->getAddressesArraySize();
                for (int32 i = 0; i < numaddr; i++) {
#ifdef WITH_IPv4
                    if (initChunk->getAddresses(i).getType() == L3Address::IPv4) {
                        struct init_ipv4_address_parameter *ipv4addr = (struct init_ipv4_address_parameter *)(((unsigned char *)ic) + size_init_chunk + parPtr);
                        ipv4addr->type = htons(INIT_PARAM_IPV4);
                        ipv4addr->length = htons(8);
                        ipv4addr->address = htonl(initChunk->getAddresses(i).toIPv4().getInt());
                        parPtr += sizeof(struct init_ipv4_address_parameter);
                    }
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
                    if (initChunk->getAddresses(i).getType() == L3Address::IPv6) {
                        struct init_ipv6_address_parameter *ipv6addr = (struct init_ipv6_address_parameter *)(((unsigned char *)ic) + size_init_chunk + parPtr);
                        ipv6addr->type = htons(INIT_PARAM_IPV6);
                        ipv6addr->length = htons(20);
                        for (int32 j = 0; j < 4; j++) {
                            ipv6addr->address[j] = htonl(initChunk->getAddresses(i).toIPv6().words()[j]);
                        }
                        parPtr += sizeof(struct init_ipv6_address_parameter);
                    }
#endif // ifdef WITH_IPv6
                }
                int chunkcount = initChunk->getSepChunksArraySize();
                if (chunkcount > 0) {
                    struct supported_extensions_parameter *supext = (struct supported_extensions_parameter *)(((unsigned char *)ic) + size_init_chunk + parPtr);
                    supext->type = htons(SUPPORTED_EXTENSIONS);
                    int chunkcount = initChunk->getSepChunksArraySize();
                    supext->length = htons(sizeof(struct supported_extensions_parameter) + chunkcount);
                    for (int i = 0; i < chunkcount; i++) {
                        supext->chunk_type[i] = initChunk->getSepChunks(i);
                    }
                    parPtr += ADD_PADDING(sizeof(struct supported_extensions_parameter) + chunkcount);
                }
                if (initChunk->getHmacTypesArraySize() > 0) {
                    struct random_parameter *random = (struct random_parameter *)(((unsigned char *)ic) + size_init_chunk + parPtr);
                    random->type = htons(RANDOM);
                    unsigned char *vector = (unsigned char *)malloc(64);
                    struct random_parameter *rp = (struct random_parameter *)((unsigned char *)vector);
                    rp->type = htons(RANDOM);
                    int randomsize = initChunk->getRandomArraySize();
                    for (int i = 0; i < randomsize; i++) {
                        random->random[i] = (initChunk->getRandom(i));
                        rp->random[i] = (initChunk->getRandom(i));
                    }
                    parPtr += ADD_PADDING(sizeof(struct random_parameter) + randomsize);
                    random->length = htons(sizeof(struct random_parameter) + randomsize);
                    rp->length = htons(sizeof(struct random_parameter) + randomsize);
                    sizeKeyVector = sizeof(struct random_parameter) + randomsize;
                    struct tlv *chunks = (struct tlv *)(((unsigned char *)ic) + size_init_chunk + parPtr);
                    struct tlv *cp = (struct tlv *)(((unsigned char *)vector) + sizeKeyVector);

                    chunks->type = htons(CHUNKS);
                    cp->type = htons(CHUNKS);
                    int chunksize = initChunk->getChunkTypesArraySize();
                    EV_DETAIL << "chunksize=" << chunksize << "\n";
                    for (int i = 0; i < chunksize; i++) {
                        chunks->value[i] = (initChunk->getChunkTypes(i));
                        EV_DETAIL << "chunkType=" << initChunk->getChunkTypes(i) << "\n";
                        cp->value[i] = (initChunk->getChunkTypes(i));
                    }
                    chunks->length = htons(sizeof(struct tlv) + chunksize);
                    cp->length = htons(sizeof(struct tlv) + chunksize);
                    sizeKeyVector += sizeof(struct tlv) + chunksize;
                    parPtr += ADD_PADDING(sizeof(struct tlv) + chunksize);
                    struct hmac_algo *hmac = (struct hmac_algo *)(((unsigned char *)ic) + size_init_chunk + parPtr);
                    struct hmac_algo *hp = (struct hmac_algo *)(((unsigned char *)vector) + sizeKeyVector);
                    hmac->type = htons(HMAC_ALGO);
                    hp->type = htons(HMAC_ALGO);
                    hmac->length = htons(4 + 2 * initChunk->getHmacTypesArraySize());
                    hp->length = htons(4 + 2 * initChunk->getHmacTypesArraySize());
                    sizeKeyVector += (4 + 2 * initChunk->getHmacTypesArraySize());
                    for (unsigned int i = 0; i < initChunk->getHmacTypesArraySize(); i++) {
                        hmac->ident[i] = htons(initChunk->getHmacTypes(i));
                        hp->ident[i] = htons(initChunk->getHmacTypes(i));
                    }
                    parPtr += ADD_PADDING(4 + 2 * initChunk->getHmacTypesArraySize());

                    for (unsigned int k = 0; k < sizeKeyVector; k++) {
                        keyVector[k] = vector[k];
                    }
                    free(vector);
                }
                ic->length = htons(SCTP_INIT_CHUNK_LENGTH + parPtr);
                writtenbytes += SCTP_INIT_CHUNK_LENGTH + parPtr;
                break;
            }

            case INIT_ACK: {
                EV_INFO << "serialize INIT_ACK sizeKeyVector=" << sizeKeyVector << "\n";
                SCTPInitAckChunk *initAckChunk = check_and_cast<SCTPInitAckChunk *>(chunk);
                //sctpEV3<<simulation.simTime()<<" SCTPAssociation:: InitAck sent \n";
                // destination is send buffer:
                struct init_ack_chunk *iac = (struct init_ack_chunk *)(buf + writtenbytes);    // append data to buffer
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
                if (initAckChunk->getIpv4Supported() || initAckChunk->getIpv6Supported()) {
                    struct supported_address_types_parameter *sup_addr = (struct supported_address_types_parameter *)(((unsigned char *)iac) + size_init_chunk + parPtr);
                    sup_addr->type = htons(INIT_SUPPORTED_ADDRESS);
                    sup_addr->length = htons(8);
                    if (initAckChunk->getIpv4Supported() && initAckChunk->getIpv6Supported()) {
                        sup_addr->address_type_1 = htons(INIT_PARAM_IPV4);
                        sup_addr->address_type_2 = htons(INIT_PARAM_IPV6);
                    } else if (initAckChunk->getIpv4Supported()) {
                        sup_addr->address_type_1 = htons(INIT_PARAM_IPV4);
                        sup_addr->address_type_2 = 0;
                    } else {
                        sup_addr->address_type_1 = htons(INIT_PARAM_IPV6);
                        sup_addr->address_type_2 = 0;
                    }
                    parPtr += 8;
                }
                if (initAckChunk->getForwardTsn() == true) {
                    struct forward_tsn_supported_parameter *forward = (struct forward_tsn_supported_parameter *)(((unsigned char *)iac) + size_init_chunk + parPtr);
                    forward->type = htons(FORWARD_TSN_SUPPORTED_PARAMETER);
                    forward->length = htons(4);
                    parPtr += 4;
                }

                int32 numaddr = initAckChunk->getAddressesArraySize();
                for (int32 i = 0; i < numaddr; i++) {
#ifdef WITH_IPv4
                    if (initAckChunk->getAddresses(i).getType() == L3Address::IPv4) {
                        struct init_ipv4_address_parameter *ipv4addr = (struct init_ipv4_address_parameter *)(((unsigned char *)iac) + size_init_chunk + parPtr);
                        ipv4addr->type = htons(INIT_PARAM_IPV4);
                        ipv4addr->length = htons(8);
                        ipv4addr->address = htonl(initAckChunk->getAddresses(i).toIPv4().getInt());
                        parPtr += sizeof(struct init_ipv4_address_parameter);
                    }
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
                    if (initAckChunk->getAddresses(i).getType() == L3Address::IPv6) {
                        struct init_ipv6_address_parameter *ipv6addr = (struct init_ipv6_address_parameter *)(((unsigned char *)iac) + size_init_chunk + parPtr);
                        ipv6addr->type = htons(INIT_PARAM_IPV6);
                        ipv6addr->length = htons(20);
                        for (int j = 0; j < 4; j++) {
                            ipv6addr->address[j] = htonl(initAckChunk->getAddresses(i).toIPv6().words()[j]);
                        }
                        parPtr += sizeof(struct init_ipv6_address_parameter);
                    }
#endif // ifdef WITH_IPv6
                }
                int chunkcount = initAckChunk->getSepChunksArraySize();
                if (chunkcount > 0) {
                    struct supported_extensions_parameter *supext = (struct supported_extensions_parameter *)(((unsigned char *)iac) + size_init_chunk + parPtr);
                    supext->type = htons(SUPPORTED_EXTENSIONS);
                    int chunkcount = initAckChunk->getSepChunksArraySize();
                    supext->length = htons(sizeof(struct supported_extensions_parameter) + chunkcount);
                    for (int i = 0; i < chunkcount; i++) {
                        supext->chunk_type[i] = initAckChunk->getSepChunks(i);
                    }
                    parPtr += ADD_PADDING(sizeof(struct supported_extensions_parameter) + chunkcount);
                }
                uint32 uLen = initAckChunk->getUnrecognizedParametersArraySize();
                if (uLen > 0) {
                    //sctpEV3<<"uLen="<<uLen<<"\n";
                    int32 k = 0;
                    uint32 pLen = 0;
                    while (uLen > 0) {
                        struct tlv *unknown = (struct tlv *)(((unsigned char *)iac) + size_init_chunk + parPtr);
                        unknown->type = htons(UNRECOGNIZED_PARAMETER);
                        pLen = initAckChunk->getUnrecognizedParameters(k + 2) * 16 + initAckChunk->getUnrecognizedParameters(k + 3);
                        unknown->length = htons(pLen + 4);
                        //sctpEV3<<"unknown->length="<<pLen<<"\n";
                        for (uint32 i = 0; i < ADD_PADDING(pLen); i++, k++)
                            unknown->value[i] = initAckChunk->getUnrecognizedParameters(k);
                        parPtr += ADD_PADDING(pLen + 4);
                        uLen -= ADD_PADDING(pLen);
                    }
                }
                if (initAckChunk->getHmacTypesArraySize() > 0) {
                    unsigned int sizeVector;
                    struct random_parameter *random = (struct random_parameter *)(((unsigned char *)iac) + size_init_chunk + parPtr);
                    random->type = htons(RANDOM);
                    int randomsize = initAckChunk->getRandomArraySize();
                    unsigned char *vector = (unsigned char *)malloc(64);
                    struct random_parameter *rp = (struct random_parameter *)((unsigned char *)vector);
                    rp->type = htons(RANDOM);
                    for (int i = 0; i < randomsize; i++) {
                        random->random[i] = (initAckChunk->getRandom(i));
                        rp->random[i] = (initAckChunk->getRandom(i));
                    }
                    parPtr += ADD_PADDING(sizeof(struct random_parameter) + randomsize);
                    random->length = htons(sizeof(struct random_parameter) + randomsize);
                    rp->length = htons(sizeof(struct random_parameter) + randomsize);
                    sizeVector = ntohs(rp->length);
                    struct tlv *chunks = (struct tlv *)(((unsigned char *)iac) + size_init_chunk + parPtr);
                    struct tlv *cp = (struct tlv *)(((unsigned char *)vector) + 36);
                    chunks->type = htons(CHUNKS);
                    cp->type = htons(CHUNKS);
                    int chunksize = initAckChunk->getChunkTypesArraySize();
                    for (int i = 0; i < chunksize; i++) {
                        chunks->value[i] = (initAckChunk->getChunkTypes(i));
                        cp->value[i] = (initAckChunk->getChunkTypes(i));
                    }
                    chunks->length = htons(sizeof(struct tlv) + chunksize);
                    cp->length = htons(sizeof(struct tlv) + chunksize);
                    sizeVector += sizeof(struct tlv) + chunksize;
                    parPtr += ADD_PADDING(sizeof(struct tlv) + chunksize);
                    struct hmac_algo *hmac = (struct hmac_algo *)(((unsigned char *)iac) + size_init_chunk + parPtr);
                    struct hmac_algo *hp = (struct hmac_algo *)(((unsigned char *)(vector)) + 36 + sizeof(struct tlv) + chunksize);
                    hmac->type = htons(HMAC_ALGO);
                    hp->type = htons(HMAC_ALGO);
                    hmac->length = htons(4 + 2 * initAckChunk->getHmacTypesArraySize());
                    hp->length = htons(4 + 2 * initAckChunk->getHmacTypesArraySize());
                    sizeVector += (4 + 2 * initAckChunk->getHmacTypesArraySize());
                    for (unsigned int i = 0; i < initAckChunk->getHmacTypesArraySize(); i++) {
                        hmac->ident[i] = htons(initAckChunk->getHmacTypes(i));
                        hp->ident[i] = htons(initAckChunk->getHmacTypes(i));
                    }
                    parPtr += ADD_PADDING(4 + 2 * initAckChunk->getHmacTypesArraySize());
                    for (unsigned int k = 0; k < min(sizeVector, 64); k++) {
                        if (sizeKeyVector != 0)
                            peerKeyVector[k] = vector[k];
                        else
                            keyVector[k] = vector[k];
                    }

                    if (sizeKeyVector != 0)
                        sizePeerKeyVector = sizeVector;
                    else
                        sizeKeyVector = sizeVector;

                    calculateSharedKey();
                    free(vector);
                }
                int32 cookielen = initAckChunk->getCookieArraySize();
                if (cookielen == 0) {
                    SCTPCookie *stateCookie = check_and_cast<SCTPCookie *>(initAckChunk->getStateCookie());
                    struct init_cookie_parameter *cookie = (struct init_cookie_parameter *)(((unsigned char *)iac) + size_init_chunk + parPtr);
                    cookie->type = htons(INIT_PARAM_COOKIE);
                    cookie->length = htons(SCTP_COOKIE_LENGTH + 4);
                    cookie->creationTime = htonl((uint32)stateCookie->getCreationTime().dbl());
                    cookie->localTag = htonl(stateCookie->getLocalTag());
                    cookie->peerTag = htonl(stateCookie->getPeerTag());
                    for (int32 i = 0; i < 32; i++) {
                        cookie->localTieTag[i] = stateCookie->getLocalTieTag(i);
                        cookie->peerTieTag[i] = stateCookie->getPeerTieTag(i);
                    }
                    parPtr += (SCTP_COOKIE_LENGTH + 4);
                } else {
                    struct tlv *cookie = (struct tlv *)(((unsigned char *)iac) + size_init_chunk + parPtr);
                    cookie->type = htons(INIT_PARAM_COOKIE);
                    cookie->length = htons(cookielen + 4);
                    for (int32 i = 0; i < cookielen; i++)
                        cookie->value[i] = initAckChunk->getCookie(i);
                    parPtr += cookielen + 4;
                }
                iac->length = htons(SCTP_INIT_CHUNK_LENGTH + parPtr);
                writtenbytes += SCTP_INIT_CHUNK_LENGTH + parPtr;
                break;
            }

            case SACK: {
                SCTPSackChunk *sackChunk = check_and_cast<SCTPSackChunk *>(chunk);

                // destination is send buffer:
                struct sack_chunk *sac = (struct sack_chunk *)(buf + writtenbytes);    // append data to buffer
                writtenbytes += (sackChunk->getByteLength());

                // fill buffer with data from SCTP init ack chunk structure
                sac->type = sackChunk->getChunkType();
                sac->length = htons(sackChunk->getByteLength());
                uint32 cumtsnack = sackChunk->getCumTsnAck();
                sac->cum_tsn_ack = htonl(cumtsnack);
                sac->a_rwnd = htonl(sackChunk->getA_rwnd());
                sac->nr_of_gaps = htons(sackChunk->getNumGaps());
                sac->nr_of_dups = htons(sackChunk->getNumDupTsns());

                // GAPs and Dup. TSNs:
                int16 numgaps = sackChunk->getNumGaps();
                int16 numdups = sackChunk->getNumDupTsns();
                for (int16 i = 0; i < numgaps; i++) {
                    struct sack_gap *gap = (struct sack_gap *)(((unsigned char *)sac) + size_sack_chunk + i * sizeof(struct sack_gap));
                    gap->start = htons(sackChunk->getGapStart(i) - cumtsnack);
                    gap->stop = htons(sackChunk->getGapStop(i) - cumtsnack);
                }
                for (int16 i = 0; i < numdups; i++) {
                    struct sack_duptsn *dup = (struct sack_duptsn *)(((unsigned char *)sac) + size_sack_chunk + numgaps * sizeof(struct sack_gap) + i * sizeof(struct sack_duptsn));
                    dup->tsn = htonl(sackChunk->getDupTsns(i));
                }
                break;
            }

            case NR_SACK: {
                SCTPSackChunk *sackChunk = check_and_cast<SCTPSackChunk *>(chunk);

                // destination is send buffer:
                struct nr_sack_chunk *sac = (struct nr_sack_chunk *)(buf + writtenbytes);    // append data to buffer
                writtenbytes += (sackChunk->getByteLength());

                // fill buffer with data from SCTP init ack chunk structure
                sac->type = sackChunk->getChunkType();
                sac->length = htons(sackChunk->getByteLength());
                uint32 cumtsnack = sackChunk->getCumTsnAck();
                sac->cum_tsn_ack = htonl(cumtsnack);
                sac->a_rwnd = htonl(sackChunk->getA_rwnd());
                sac->nr_of_gaps = htons(sackChunk->getNumGaps());
                sac->nr_of_dups = htons(sackChunk->getNumDupTsns());

                // GAPs and Dup. TSNs:
                int16 numgaps = sackChunk->getNumGaps();
                int16 numdups = sackChunk->getNumDupTsns();
                int16 numnrgaps = 0;
                for (int16 i = 0; i < numgaps; i++) {
                    struct sack_gap *gap = (struct sack_gap *)(((unsigned char *)sac) + size_nr_sack_chunk + i * sizeof(struct sack_gap));
                    gap->start = htons(sackChunk->getGapStart(i) - cumtsnack);
                    gap->stop = htons(sackChunk->getGapStop(i) - cumtsnack);
                }
                sac->nr_of_nr_gaps = htons(sackChunk->getNumNrGaps());
                sac->reserved = htons(0);
                numnrgaps = sackChunk->getNumNrGaps();
                for (int16 i = 0; i < numnrgaps; i++) {
                    struct sack_gap *gap = (struct sack_gap *)(((unsigned char *)sac) + size_nr_sack_chunk + (numgaps + i) * sizeof(struct sack_gap));
                    gap->start = htons(sackChunk->getNrGapStart(i) - cumtsnack);
                    gap->stop = htons(sackChunk->getNrGapStop(i) - cumtsnack);
                }
                for (int16 i = 0; i < numdups; i++) {
                    struct sack_duptsn *dup = (struct sack_duptsn *)(((unsigned char *)sac) + size_nr_sack_chunk + (numgaps + numnrgaps) * sizeof(struct sack_gap) + i * sizeof(sack_duptsn));
                    dup->tsn = htonl(sackChunk->getDupTsns(i));
                }
                break;
            }

            case HEARTBEAT :
                {
                    EV_INFO << simTime() << "  SCTPAssociation:: Heartbeat sent \n";
                    SCTPHeartbeatChunk *heartbeatChunk = check_and_cast<SCTPHeartbeatChunk *>(chunk);

                    // destination is send buffer:
                    struct heartbeat_chunk *hbc = (struct heartbeat_chunk *)(buf + writtenbytes);    // append data to buffer

                    // fill buffer with data from SCTP init ack chunk structure
                    hbc->type = heartbeatChunk->getChunkType();

                    // deliver info:
                    struct heartbeat_info *hbi = (struct heartbeat_info *)(((unsigned char *)hbc) + size_heartbeat_chunk);
                    L3Address addr = heartbeatChunk->getRemoteAddr();
                    simtime_t time = heartbeatChunk->getTimeField();
                    int32 infolen = 0;
#ifdef WITH_IPv4
                    if (addr.getType() == L3Address::IPv4) {
                        infolen = sizeof(addr.toIPv4().getInt()) + sizeof(uint32);
                        hbi->type = htons(1);    // mandatory
                        hbi->length = htons(infolen + 4);
                        struct init_ipv4_address_parameter *ipv4addr = (struct init_ipv4_address_parameter *)(((unsigned char *)hbc) + 8);
                        ipv4addr->type = htons(INIT_PARAM_IPV4);
                        ipv4addr->length = htons(8);
                        ipv4addr->address = htonl(addr.toIPv4().getInt());
                        HBI_ADDR(hbi).v4addr = *ipv4addr;
                    }
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
                    if (addr.getType() == L3Address::IPv6) {
                        infolen = 20 + sizeof(uint32);
                        hbi->type = htons(1);    // mandatory
                        hbi->length = htons(infolen + 4);
                        struct init_ipv6_address_parameter *ipv6addr = (struct init_ipv6_address_parameter *)(((unsigned char *)hbc) + 8);
                        ipv6addr->type = htons(INIT_PARAM_IPV6);
                        ipv6addr->length = htons(20);
                        for (int32 j = 0; j < 4; j++) {
                            ipv6addr->address[j] = htonl(addr.toIPv6().words()[j]);
                        }
                        HBI_ADDR(hbi).v6addr = *ipv6addr;
                    }
#endif // ifdef WITH_IPv6
                    ASSERT(infolen != 0);
                    HBI_TIME(hbi) = htonl((uint32)time.dbl());
                    hbc->length = htons(sizeof(struct heartbeat_chunk) + infolen + 4);
                    writtenbytes += sizeof(struct heartbeat_chunk) + infolen + 4;
                    break;
                }

            case HEARTBEAT_ACK :
                {
                    EV_INFO << simTime() << " SCTPAssociation:: HeartbeatAck sent \n";
                    SCTPHeartbeatAckChunk *heartbeatAckChunk = check_and_cast<SCTPHeartbeatAckChunk *>(chunk);

                    // destination is send buffer:
                    struct heartbeat_ack_chunk *hbac = (struct heartbeat_ack_chunk *)(buf + writtenbytes);    // append data to buffer

                    // fill buffer with data from SCTP init ack chunk structure
                    hbac->type = heartbeatAckChunk->getChunkType();

                    // deliver info:
                    struct heartbeat_info *hbi = (struct heartbeat_info *)(((unsigned char *)hbac) + size_heartbeat_ack_chunk);
                    int32 infolen = heartbeatAckChunk->getInfoArraySize();
                    hbi->type = htons(1);    //mandatory
                    if (infolen > 0) {
                        hbi->length = htons(infolen + 4);
                        for (int32 i = 0; i < infolen; i++) {
                            HBI_INFO(hbi)[i] = heartbeatAckChunk->getInfo(i);
                        }
                    }
                    else {
                        L3Address addr = heartbeatAckChunk->getRemoteAddr();
                        simtime_t time = heartbeatAckChunk->getTimeField();

#ifdef WITH_IPv4
                        if (addr.getType() == L3Address::IPv4) {
                            infolen = sizeof(addr.toIPv4().getInt()) + sizeof(uint32);
                            hbi->type = htons(1);    // mandatory
                            hbi->length = htons(infolen + 4);
                            struct init_ipv4_address_parameter *ipv4addr = (struct init_ipv4_address_parameter *)(((unsigned char *)hbac) + 8);
                            ipv4addr->type = htons(INIT_PARAM_IPV4);
                            ipv4addr->length = htons(8);
                            ipv4addr->address = htonl(addr.toIPv4().getInt());
                            HBI_ADDR(hbi).v4addr = *ipv4addr;
                        }
#endif // ifdef WITH_IPv4
#ifdef WITH_IPv6
                        if (addr.getType() == L3Address::IPv6) {
                            infolen = 20 + sizeof(uint32);
                            hbi->type = htons(1);    // mandatory
                            hbi->length = htons(infolen + 4);
                            struct init_ipv6_address_parameter *ipv6addr = (struct init_ipv6_address_parameter *)(((unsigned char *)hbac) + 8);
                            ipv6addr->type = htons(INIT_PARAM_IPV6);
                            ipv6addr->length = htons(20);
                            for (int32 j = 0; j < 4; j++) {
                                ipv6addr->address[j] = htonl(addr.toIPv6().words()[j]);
                            }
                            HBI_ADDR(hbi).v6addr = *ipv6addr;
                        }
#endif // ifdef WITH_IPv6
                        HBI_TIME(hbi) = htonl((uint32)time.dbl());
                    }
                    hbac->length = htons(sizeof(struct heartbeat_ack_chunk) + infolen + 4);
                    writtenbytes += sizeof(struct heartbeat_ack_chunk) + infolen + 4;

                    break;
                }

            case ABORT: {
                EV_INFO << simTime() << " SCTPAssociation:: Abort sent \n";
                SCTPAbortChunk *abortChunk = check_and_cast<SCTPAbortChunk *>(chunk);

                // destination is send buffer:
                struct abort_chunk *ac = (struct abort_chunk *)(buf + writtenbytes);    // append data to buffer
                writtenbytes += (abortChunk->getByteLength());

                // fill buffer with data from SCTP init ack chunk structure
                ac->type = abortChunk->getChunkType();
                unsigned char flags = 0;
                if (abortChunk->getT_Bit())
                    flags |= T_BIT;
                ac->flags = flags;
                ac->length = htons(abortChunk->getByteLength());
                break;
            }

            case COOKIE_ECHO: {
                EV_INFO << simTime() << " SCTPAssociation:: CookieEcho sent \n";
                SCTPCookieEchoChunk *cookieChunk = check_and_cast<SCTPCookieEchoChunk *>(chunk);

                struct cookie_echo_chunk *cec = (struct cookie_echo_chunk *)(buf + writtenbytes);

                cec->type = cookieChunk->getChunkType();
                cec->length = htons(cookieChunk->getByteLength());
                int32 cookielen = cookieChunk->getCookieArraySize();
                if (cookielen > 0) {
                    for (int32 i = 0; i < cookielen; i++)
                        cec->state_cookie[i] = cookieChunk->getCookie(i);
                }
                else {
                    SCTPCookie *stateCookie = check_and_cast<SCTPCookie *>(cookieChunk->getStateCookie());
                    struct cookie_parameter *cookie = (struct cookie_parameter *)(buf + writtenbytes + 4);
                    cookie->creationTime = htonl((uint32)stateCookie->getCreationTime().dbl());
                    cookie->localTag = htonl(stateCookie->getLocalTag());
                    cookie->peerTag = htonl(stateCookie->getPeerTag());
                    for (int32 i = 0; i < 32; i++) {
                        cookie->localTieTag[i] = stateCookie->getLocalTieTag(i);
                        cookie->peerTieTag[i] = stateCookie->getPeerTieTag(i);
                    }
                }
                writtenbytes += (ADD_PADDING(cookieChunk->getByteLength()));
                //sctpEV3<<"buflen cookie_echo="<<buflen<<"\n";
                uint32 uLen = cookieChunk->getUnrecognizedParametersArraySize();
                if (uLen > 0) {
                    //sctpEV3<<"uLen="<<uLen<<"\n";
                    struct error_chunk *error = (struct error_chunk *)(buf + writtenbytes);
                    error->type = ERRORTYPE;
                    error->flags = 0;
                    int32 k = 0;
                    uint32 pLen = 0;
                    uint32 ecLen = SCTP_ERROR_CHUNK_LENGTH;
                    uint32 ecParPtr = 0;
                    while (uLen > 0) {
                        struct tlv *unknown = (struct tlv *)(((unsigned char *)error) + sizeof(struct error_chunk) + ecParPtr);
                        unknown->type = htons(UNRECOGNIZED_PARAMETER);
                        pLen = cookieChunk->getUnrecognizedParameters(k + 2) * 16 + cookieChunk->getUnrecognizedParameters(k + 3);
                        unknown->length = htons(pLen + 4);
                        ecLen += pLen + 4;
                        //sctpEV3<<"plength="<<pLen<<" ecLen="<<ecLen<<"\n";
                        for (uint32 i = 0; i < ADD_PADDING(pLen); i++, k++)
                            unknown->value[i] = cookieChunk->getUnrecognizedParameters(k);
                        ecParPtr += ADD_PADDING(pLen + 4);
                        //sctpEV3<<"ecParPtr="<<ecParPtr<<"\n";
                        uLen -= ADD_PADDING(pLen);
                    }
                    error->length = htons(ecLen);
                    writtenbytes += SCTP_ERROR_CHUNK_LENGTH + ecParPtr;
                }

                break;
            }

            case COOKIE_ACK: {
                EV_INFO << simTime() << " SCTPAssociation:: CookieAck sent \n";
                SCTPCookieAckChunk *cookieAckChunk = check_and_cast<SCTPCookieAckChunk *>(chunk);

                struct cookie_ack_chunk *cac = (struct cookie_ack_chunk *)(buf + writtenbytes);
                writtenbytes += (cookieAckChunk->getByteLength());

                cac->type = cookieAckChunk->getChunkType();
                cac->length = htons(cookieAckChunk->getByteLength());

                break;
            }

            case SHUTDOWN: {
                EV_INFO << simTime() << " SCTPAssociation:: Shutdown sent \n";
                SCTPShutdownChunk *shutdownChunk = check_and_cast<SCTPShutdownChunk *>(chunk);

                struct shutdown_chunk *sac = (struct shutdown_chunk *)(buf + writtenbytes);
                writtenbytes += (shutdownChunk->getByteLength());

                sac->type = shutdownChunk->getChunkType();
                sac->cumulative_tsn_ack = htonl(shutdownChunk->getCumTsnAck());
                sac->length = htons(shutdownChunk->getByteLength());

                break;
            }

            case SHUTDOWN_ACK: {
                EV_INFO << simTime() << " SCTPAssociation:: ShutdownAck sent \n";
                SCTPShutdownAckChunk *shutdownAckChunk = check_and_cast<SCTPShutdownAckChunk *>(chunk);

                struct shutdown_ack_chunk *sac = (struct shutdown_ack_chunk *)(buf + writtenbytes);
                writtenbytes += (shutdownAckChunk->getByteLength());

                sac->type = shutdownAckChunk->getChunkType();
                sac->length = htons(shutdownAckChunk->getByteLength());

                break;
            }

            case SHUTDOWN_COMPLETE: {
                EV_INFO << simTime() << " SCTPAssociation:: ShutdownComplete sent \n";
                SCTPShutdownCompleteChunk *shutdownCompleteChunk = check_and_cast<SCTPShutdownCompleteChunk *>(chunk);

                struct shutdown_complete_chunk *sac = (struct shutdown_complete_chunk *)(buf + writtenbytes);
                writtenbytes += (shutdownCompleteChunk->getByteLength());

                sac->type = shutdownCompleteChunk->getChunkType();
                sac->length = htons(shutdownCompleteChunk->getByteLength());
                unsigned char flags = 0;
                if (shutdownCompleteChunk->getTBit())
                    flags |= T_BIT;
                sac->flags = flags;
                break;
            }

            case AUTH: {
                SCTPAuthenticationChunk *authChunk = check_and_cast<SCTPAuthenticationChunk *>(chunk);
                struct auth_chunk *auth = (struct auth_chunk *)(buf + writtenbytes);
                authstart = writtenbytes;
                writtenbytes += SCTP_AUTH_CHUNK_LENGTH + SHA_LENGTH;
                auth->type = authChunk->getChunkType();
                auth->flags = 0;
                auth->length = htons(SCTP_AUTH_CHUNK_LENGTH + SHA_LENGTH);
                auth->shared_key = htons(authChunk->getSharedKey());
                auth->hmac_identifier = htons(authChunk->getHMacIdentifier());
                for (int i = 0; i < SHA_LENGTH; i++)
                    auth->hmac[i] = 0;
                break;
            }

            case FORWARD_TSN: {
                SCTPForwardTsnChunk *forward = check_and_cast<SCTPForwardTsnChunk *>(chunk);
                struct forward_tsn_chunk *forw = (struct forward_tsn_chunk *)(buf + writtenbytes);
                writtenbytes += (forward->getByteLength());
                forw->type = forward->getChunkType();
                forw->length = htons(forward->getByteLength());
                forw->cum_tsn = htonl(forward->getNewCumTsn());
                int streamPtr = 0;
                for (unsigned int i = 0; i < forward->getSidArraySize(); i++) {
                    struct forward_tsn_streams *str = (struct forward_tsn_streams *)(((unsigned char *)forw) + sizeof(struct forward_tsn_chunk) + streamPtr);
                    str->sid = htons(forward->getSid(i));
                    str->ssn = htons(forward->getSsn(i));
                    streamPtr += 4;
                }
                break;
            }

            case ASCONF: {
                SCTPAsconfChunk *asconfChunk = check_and_cast<SCTPAsconfChunk *>(chunk);
                struct asconf_chunk *asconf = (struct asconf_chunk *)(buf + writtenbytes);
                writtenbytes += (asconfChunk->getByteLength());
                asconf->type = asconfChunk->getChunkType();
                asconf->length = htons(asconfChunk->getByteLength());
                asconf->serial = htonl(asconfChunk->getSerialNumber());
                int parPtr = 0;
                struct init_ipv4_address_parameter *ipv4addr = (struct init_ipv4_address_parameter *)(((unsigned char *)asconf) + sizeof(struct asconf_chunk) + parPtr);
                ipv4addr->type = htons(INIT_PARAM_IPV4);
                ipv4addr->length = htons(8);
                ipv4addr->address = htonl(asconfChunk->getAddressParam().toIPv4().getInt());
                parPtr += 8;
                for (unsigned int i = 0; i < asconfChunk->getAsconfParamsArraySize(); i++) {
                    SCTPParameter *parameter = check_and_cast<SCTPParameter *>(asconfChunk->getAsconfParams(i));
                    switch (parameter->getParameterType()) {
                        case ADD_IP_ADDRESS: {
                            SCTPAddIPParameter *addip = check_and_cast<SCTPAddIPParameter *>(parameter);
                            struct add_ip_parameter *ip = (struct add_ip_parameter *)(((unsigned char *)asconf) + sizeof(struct asconf_chunk) + parPtr);
                            parPtr += 8;
                            ip->type = htons(ADD_IP_ADDRESS);
                            ip->correlation_id = htonl(addip->getRequestCorrelationId());
                            struct init_ipv4_address_parameter *ipv4addr = (struct init_ipv4_address_parameter *)(((unsigned char *)asconf) + sizeof(struct asconf_chunk) + parPtr);
                            ipv4addr->type = htons(INIT_PARAM_IPV4);
                            ipv4addr->length = htons(8);
                            ipv4addr->address = htonl(addip->getAddressParam().toIPv4().getInt());
                            parPtr += 8;
                            ip->length = htons(addip->getByteLength());
                            break;
                        }

                        case DELETE_IP_ADDRESS: {
                            SCTPDeleteIPParameter *deleteip = check_and_cast<SCTPDeleteIPParameter *>(parameter);
                            struct add_ip_parameter *ip = (struct add_ip_parameter *)(((unsigned char *)asconf) + sizeof(struct asconf_chunk) + parPtr);
                            parPtr += 8;
                            ip->type = htons(DELETE_IP_ADDRESS);
                            ip->correlation_id = htonl(deleteip->getRequestCorrelationId());
                            struct init_ipv4_address_parameter *ipv4addr = (struct init_ipv4_address_parameter *)(((unsigned char *)asconf) + sizeof(struct asconf_chunk) + parPtr);
                            ipv4addr->type = htons(INIT_PARAM_IPV4);
                            ipv4addr->length = htons(8);
                            ipv4addr->address = htonl(deleteip->getAddressParam().toIPv4().getInt());
                            parPtr += 8;
                            ip->length = htons(deleteip->getByteLength());
                            break;
                        }

                        case SET_PRIMARY_ADDRESS: {
                            SCTPSetPrimaryIPParameter *setip = check_and_cast<SCTPSetPrimaryIPParameter *>(parameter);
                            struct add_ip_parameter *ip = (struct add_ip_parameter *)(((unsigned char *)asconf) + sizeof(struct asconf_chunk) + parPtr);
                            parPtr += 8;
                            ip->type = htons(SET_PRIMARY_ADDRESS);
                            ip->correlation_id = htonl(setip->getRequestCorrelationId());
                            struct init_ipv4_address_parameter *ipv4addr = (struct init_ipv4_address_parameter *)(((unsigned char *)asconf) + sizeof(struct asconf_chunk) + parPtr);
                            ipv4addr->type = htons(INIT_PARAM_IPV4);
                            ipv4addr->length = htons(8);
                            ipv4addr->address = htonl(setip->getAddressParam().toIPv4().getInt());
                            parPtr += 8;
                            ip->length = htons(setip->getByteLength());
                            break;
                        }
                    }
                }
                break;
            }

            case ASCONF_ACK: {
                SCTPAsconfAckChunk *asconfAckChunk = check_and_cast<SCTPAsconfAckChunk *>(chunk);
                struct asconf_ack_chunk *asconfack = (struct asconf_ack_chunk *)(buf + writtenbytes);
                writtenbytes += SCTP_ADD_IP_CHUNK_LENGTH;
                asconfack->type = asconfAckChunk->getChunkType();
                asconfack->length = htons(asconfAckChunk->getByteLength());
                asconfack->serial = htonl(asconfAckChunk->getSerialNumber());
                int parPtr = 0;
                for (unsigned int i = 0; i < asconfAckChunk->getAsconfResponseArraySize(); i++) {
                    SCTPParameter *parameter = check_and_cast<SCTPParameter *>(asconfAckChunk->getAsconfResponse(i));
                    switch (parameter->getParameterType()) {
                        case ERROR_CAUSE_INDICATION: {
                            SCTPErrorCauseParameter *error = check_and_cast<SCTPErrorCauseParameter *>(parameter);
                            struct add_ip_parameter *addip = (struct add_ip_parameter *)(((unsigned char *)asconfack) + sizeof(struct asconf_ack_chunk) + parPtr);
                            addip->type = htons(error->getParameterType());
                            addip->length = htons(error->getByteLength());
                            addip->correlation_id = htonl(error->getResponseCorrelationId());
                            parPtr += 8;
                            struct error_cause *errorc = (struct error_cause *)(((unsigned char *)asconfack) + sizeof(struct asconf_ack_chunk) + parPtr);
                            errorc->cause_code = htons(error->getErrorCauseType());
                            errorc->length = htons(error->getByteLength() - 8);
                            parPtr += 4;
                            if (check_and_cast<SCTPParameter *>(error->getEncapsulatedPacket()) != nullptr) {
                                SCTPParameter *encParameter = check_and_cast<SCTPParameter *>(error->getEncapsulatedPacket());
                                switch (encParameter->getParameterType()) {
                                    case ADD_IP_ADDRESS: {
                                        SCTPAddIPParameter *addip = check_and_cast<SCTPAddIPParameter *>(encParameter);
                                        struct add_ip_parameter *ip = (struct add_ip_parameter *)(((unsigned char *)errorc) + sizeof(struct error_cause));
                                        parPtr += 8;
                                        ip->type = htons(ADD_IP_ADDRESS);
                                        ip->correlation_id = htonl(addip->getRequestCorrelationId());
                                        struct init_ipv4_address_parameter *ipv4addr = (struct init_ipv4_address_parameter *)(((unsigned char *)errorc) + sizeof(struct error_cause) + 8);
                                        ipv4addr->length = htons(8);
                                        ipv4addr->address = htonl(addip->getAddressParam().toIPv4().getInt());
                                        parPtr += 8;
                                        ip->length = htons(addip->getByteLength());
                                        break;
                                    }

                                    case DELETE_IP_ADDRESS: {
                                        SCTPDeleteIPParameter *deleteip = check_and_cast<SCTPDeleteIPParameter *>(encParameter);
                                        struct add_ip_parameter *ip = (struct add_ip_parameter *)(((unsigned char *)errorc) + sizeof(struct error_cause));
                                        parPtr += 8;
                                        ip->type = htons(DELETE_IP_ADDRESS);
                                        ip->correlation_id = htonl(deleteip->getRequestCorrelationId());
                                        struct init_ipv4_address_parameter *ipv4addr = (struct init_ipv4_address_parameter *)(((unsigned char *)errorc) + sizeof(struct error_cause) + 8);
                                        ipv4addr->type = htons(INIT_PARAM_IPV4);
                                        ipv4addr->length = htons(8);
                                        ipv4addr->address = htonl(deleteip->getAddressParam().toIPv4().getInt());
                                        parPtr += 8;
                                        ip->length = htons(deleteip->getByteLength());
                                        break;
                                    }

                                    case SET_PRIMARY_ADDRESS: {
                                        SCTPSetPrimaryIPParameter *setip = check_and_cast<SCTPSetPrimaryIPParameter *>(encParameter);
                                        struct add_ip_parameter *ip = (struct add_ip_parameter *)(((unsigned char *)errorc) + sizeof(struct error_cause));
                                        parPtr += 8;
                                        ip->type = htons(SET_PRIMARY_ADDRESS);
                                        ip->correlation_id = htonl(setip->getRequestCorrelationId());
                                        struct init_ipv4_address_parameter *ipv4addr = (struct init_ipv4_address_parameter *)(((unsigned char *)errorc) + sizeof(struct error_cause) + 8);
                                        ipv4addr->type = htons(INIT_PARAM_IPV4);
                                        ipv4addr->length = htons(8);
                                        ipv4addr->address = htonl(setip->getAddressParam().toIPv4().getInt());
                                        parPtr += 8;
                                        ip->length = htons(setip->getByteLength());
                                        break;
                                    }
                                }
                            }
                            break;
                        }

                        case SUCCESS_INDICATION: {
                            SCTPSuccessIndication *success = check_and_cast<SCTPSuccessIndication *>(parameter);
                            struct add_ip_parameter *addip = (struct add_ip_parameter *)(((unsigned char *)asconfack) + sizeof(struct asconf_ack_chunk) + parPtr);
                            addip->type = htons(success->getParameterType());
                            addip->length = htons(8);
                            addip->correlation_id = htonl(success->getResponseCorrelationId());
                            parPtr += 8;
                            break;
                        }
                    }
                }
                writtenbytes += parPtr;
                break;
            }

            case ERRORTYPE: {
                SCTPErrorChunk *errorchunk = check_and_cast<SCTPErrorChunk *>(chunk);
                struct error_chunk *error = (struct error_chunk *)(buf + writtenbytes);
                error->type = errorchunk->getChunkType();
                uint16 flags = 0;
                if (errorchunk->getMBit())
                    flags |= NAT_M_FLAG;
                if (errorchunk->getTBit())
                    flags |= NAT_T_FLAG;
                error->flags = flags;
                error->length = htons(errorchunk->getByteLength());

                if (errorchunk->getParametersArraySize() > 0) {
                    SCTPParameter *parameter = check_and_cast<SCTPParameter *>(errorchunk->getParameters(0));
                    switch (parameter->getParameterType()) {
                        case MISSING_NAT_ENTRY: {
                            SCTPSimpleErrorCauseParameter *ecp = check_and_cast<SCTPSimpleErrorCauseParameter *>(parameter);
                            struct error_cause *errorc = (struct error_cause *)(((unsigned char *)error) + sizeof(struct error_chunk));
                            errorc->cause_code = htons(ecp->getParameterType());
                            if (check_and_cast<IPv4Datagram *>(ecp->getEncapsulatedPacket()) != nullptr) {
                                Buffer b((unsigned char *)error + sizeof(struct error_chunk) + 4, ecp->getByteLength() - 4);
                                Context c;
                                IPv4Serializer().serializePacket(ecp->getEncapsulatedPacket(), b, c);
                            }
                            errorc->length = htons(ecp->getByteLength());
                        }
                    }
                    writtenbytes += errorchunk->getByteLength();
                }
                else
                    writtenbytes += ADD_PADDING(error->length);
                break;
            }

            case STREAM_RESET: {
                SCTPStreamResetChunk *streamReset = check_and_cast<SCTPStreamResetChunk *>(chunk);
                struct stream_reset_chunk *stream = (struct stream_reset_chunk *)(buf + writtenbytes);
                writtenbytes += (streamReset->getByteLength());
                stream->type = streamReset->getChunkType();
                stream->length = htons(streamReset->getByteLength());
                int parPtr = 0;
                for (unsigned int i = 0; i < streamReset->getParametersArraySize(); i++) {
                    SCTPParameter *parameter = check_and_cast<SCTPParameter *>(streamReset->getParameters(i));
                    switch (parameter->getParameterType()) {
                        case OUTGOING_RESET_REQUEST_PARAMETER: {
                            SCTPOutgoingSSNResetRequestParameter *outparam = check_and_cast<SCTPOutgoingSSNResetRequestParameter *>(parameter);
                            struct outgoing_reset_request_parameter *out = (outgoing_reset_request_parameter *)(((unsigned char *)stream) + sizeof(struct stream_reset_chunk) + parPtr);
                            out->type = htons(outparam->getParameterType());
                            out->srReqSn = htonl(outparam->getSrReqSn());
                            out->srResSn = htonl(outparam->getSrResSn());
                            out->lastTsn = htonl(outparam->getLastTsn());
                            parPtr += sizeof(struct outgoing_reset_request_parameter);
                            if (outparam->getStreamNumbersArraySize() > 0) {
                                for (unsigned int j = 0; j < outparam->getStreamNumbersArraySize(); j++) {
                                    out->streamNumbers[j * 2] = htons(outparam->getStreamNumbers(j));
                                }
                                parPtr += ADD_PADDING(outparam->getStreamNumbersArraySize() * 2);
                            }
                            out->length = htons(sizeof(struct outgoing_reset_request_parameter) + outparam->getStreamNumbersArraySize() * 2);
                            break;
                        }

                        case INCOMING_RESET_REQUEST_PARAMETER: {
                            SCTPIncomingSSNResetRequestParameter *inparam = check_and_cast<SCTPIncomingSSNResetRequestParameter *>(parameter);
                            struct incoming_reset_request_parameter *in = (incoming_reset_request_parameter *)(((unsigned char *)stream) + sizeof(struct stream_reset_chunk) + parPtr);
                            in->type = htons(inparam->getParameterType());
                            in->srReqSn = htonl(inparam->getSrReqSn());
                            parPtr += sizeof(struct incoming_reset_request_parameter);
                            if (inparam->getStreamNumbersArraySize() > 0) {
                                for (unsigned int j = 0; j < inparam->getStreamNumbersArraySize(); j++) {
                                    in->streamNumbers[j * 2] = htons(inparam->getStreamNumbers(j));
                                }
                                parPtr += ADD_PADDING(inparam->getStreamNumbersArraySize() * 2);
                            }
                            in->length = htons(sizeof(struct incoming_reset_request_parameter) + inparam->getStreamNumbersArraySize() * 2);
                            break;
                        }

                        case SSN_TSN_RESET_REQUEST_PARAMETER: {
                            SCTPSSNTSNResetRequestParameter *ssnparam = check_and_cast<SCTPSSNTSNResetRequestParameter *>(parameter);
                            struct ssn_tsn_reset_request_parameter *ssn = (struct ssn_tsn_reset_request_parameter *)(((unsigned char *)stream) + sizeof(struct stream_reset_chunk) + parPtr);
                            ssn->type = htons(ssnparam->getParameterType());
                            ssn->length = htons(4);
                            ssn->srReqSn = htonl(ssnparam->getSrReqSn());
                            parPtr += sizeof(struct ssn_tsn_reset_request_parameter);
                            break;
                        }

                        case STREAM_RESET_RESPONSE_PARAMETER: {
                            SCTPStreamResetResponseParameter *response = check_and_cast<SCTPStreamResetResponseParameter *>(parameter);
                            struct stream_reset_response_parameter *resp = (struct stream_reset_response_parameter *)(((unsigned char *)stream) + sizeof(struct stream_reset_chunk) + parPtr);
                            resp->type = htons(response->getParameterType());
                            resp->srResSn = htonl(response->getSrResSn());
                            resp->result = htonl(response->getResult());
                            resp->length = htons(12);
                            parPtr += 12;
                            if (response->getSendersNextTsn() != 0) {
                                resp->sendersNextTsn = htonl(response->getSendersNextTsn());
                                resp->receiversNextTsn = htonl(response->getReceiversNextTsn());
                                resp->length = htons(20);
                                parPtr += 8;
                            }
                            break;
                        }
                    }
                }
                break;
            }

            case PKTDROP: {
                SCTPPacketDropChunk *packetdrop = check_and_cast<SCTPPacketDropChunk *>(chunk);
                struct pktdrop_chunk *drop = (struct pktdrop_chunk *)(buf + writtenbytes);
                unsigned char flags = 0;
                if (packetdrop->getCFlag())
                    flags |= C_FLAG;
                if (packetdrop->getTFlag())
                    flags |= T_FLAG;
                if (packetdrop->getBFlag())
                    flags |= B_FLAG;
                if (packetdrop->getMFlag())
                    flags |= M_FLAG;
                drop->flags = flags;
                drop->type = packetdrop->getChunkType();
                drop->max_rwnd = htonl(packetdrop->getMaxRwnd());
                drop->queued_data = htonl(packetdrop->getQueuedData());
                drop->trunc_length = htons(packetdrop->getTruncLength());
                drop->reserved = 0;
                SCTPMessage *msg = check_and_cast<SCTPMessage *>(packetdrop->getEncapsulatedPacket());
                int msglen = msg->getByteLength();
                drop->length = htons(SCTP_PKTDROP_CHUNK_LENGTH + msglen);
                //int len = serialize(msg, drop->dropped_data, msglen);
                writtenbytes += (packetdrop->getByteLength());
                break;
            }

            default:
                printf("Serialize TODO: Implement for outgoing chunk type %d!\n", chunkType);
                throw new cRuntimeError("TODO: unknown chunktype in outgoing packet on external interface! Implement it!");
        }
    }
    // calculate the HMAC if required
    uint8 result[SHA_LENGTH];
    if (authstart != 0) {
        struct data_vector *ac = (struct data_vector *)(buf + authstart);
        EV_DETAIL << "sizeKeyVector=" << sizeKeyVector << ", sizePeerKeyVector=" << sizePeerKeyVector << "\n";
        hmacSha1((uint8 *)ac->data, writtenbytes - authstart, sharedKey, sizeKeyVector + sizePeerKeyVector, result);
        struct auth_chunk *auth = (struct auth_chunk *)(buf + authstart);
        for (int32 k = 0; k < SHA_LENGTH; k++)
            auth->hmac[k] = result[k];
    }
    // finally, set the CRC32 checksum field in the SCTP common header
    ch->checksum = checksum((unsigned char *)ch, writtenbytes);

    // check the serialized packet length
    if (writtenbytes != msg->getByteLength()) {
        throw cRuntimeError("SCTP Serializer error: writtenbytes (%lu) != msgLength(%lu) in message (%s)%s",
                writtenbytes, (unsigned long)msg->getByteLength(), msg->getClassName(), msg->getFullName());
    }

    return writtenbytes;
}

void SCTPSerializer::hmacSha1(const uint8 *buf, uint32 buflen, const uint8 *key, uint32 keylen, uint8 *digest)
{
    /* XXX needs to be implemented */
    for (uint16 i = 0; i < SHA_LENGTH; i++) {
        digest[i] = 0;
    }
}

uint32 SCTPSerializer::checksum(const uint8_t *buf, register uint32 len)
{
    uint32 h;
    unsigned char byte0, byte1, byte2, byte3;
    uint32 crc32c;
    uint32 i;
    uint32 res = (~0L);
    for (i = 0; i < len; i++)
        CRC32C(res, buf[i]);
    h = ~res;
    byte0 = h & 0xff;
    byte1 = (h >> 8) & 0xff;
    byte2 = (h >> 16) & 0xff;
    byte3 = (h >> 24) & 0xff;
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
    int size_error_chunk = sizeof(struct error_chunk);
    int size_forward_tsn_chunk = sizeof(struct forward_tsn_chunk);
    int size_asconf_chunk = sizeof(struct asconf_chunk);
    int size_addip_parameter = sizeof(struct add_ip_parameter);
    int size_asconf_ack_chunk = sizeof(struct asconf_ack_chunk);
    int size_auth_chunk = sizeof(struct auth_chunk);
    int size_stream_reset_chunk = sizeof(struct stream_reset_chunk);
    uint16 paramType;
    int32 parptr, chunklen, cLen, woPadding;
    struct common_header *common_header = (struct common_header *)(buf);
    int32 tempChecksum = common_header->checksum;
    common_header->checksum = 0;
    int32 chksum = checksum((unsigned char *)common_header, bufsize);
    common_header->checksum = tempChecksum;

    const unsigned char *chunks = (unsigned char *)(buf + size_common_header);
    EV_TRACE << "SCTPSerializer::parse SCTPMessage\n";
    if (tempChecksum == chksum)
        dest->setChecksumOk(true);
    else
        dest->setChecksumOk(false);
    EV_DETAIL << "checksumOK=" << dest->getChecksumOk() << "\n";
    dest->setSrcPort(ntohs(common_header->source_port));
    dest->setDestPort(ntohs(common_header->destination_port));
    dest->setTag(ntohl(common_header->verification_tag));
    dest->setBitLength(SCTP_COMMON_HEADER * 8);
    // chunks
    uint32 chunkPtr = 0;

    // catch ALL chunks - when a chunk is taken, the chunkPtr is set to the next chunk
    while (chunkPtr < (bufsize - size_common_header)) {
        const struct chunk *chunk = (struct chunk *)(chunks + chunkPtr);
        int32 chunkType = chunk->type;
        woPadding = ntohs(chunk->length);
        cLen = ADD_PADDING(woPadding);
        switch (chunkType) {
            case DATA: {
                EV_INFO << "Data received\n";
                const struct data_chunk *dc = (struct data_chunk *)(chunks + chunkPtr);
                EV_DETAIL << "cLen=" << cLen << "\n";
                if (cLen == 0)
                    throw new cRuntimeError("Incoming SCTP packet contains data chunk with length==0");
                SCTPDataChunk *chunk = new SCTPDataChunk("DATA");
                chunk->setChunkType(chunkType);
                chunk->setUBit(dc->flags & UNORDERED_BIT);
                chunk->setBBit(dc->flags & BEGIN_BIT);
                chunk->setEBit(dc->flags & END_BIT);
                chunk->setIBit(dc->flags & I_BIT);
                chunk->setTsn(ntohl(dc->tsn));
                chunk->setSid(ntohs(dc->sid));
                chunk->setSsn(ntohs(dc->ssn));
                chunk->setPpid(ntohl(dc->ppi));
                chunk->setByteLength(SCTP_DATA_CHUNK_LENGTH);
                EV_DETAIL << "parse data: woPadding=" << woPadding << " size_data_chunk=" << size_data_chunk << "\n";
                if (woPadding > size_data_chunk) {
                    SCTPSimpleMessage *msg = new SCTPSimpleMessage("data");
                    int32 datalen = (woPadding - size_data_chunk);
                    msg->setBitLength(datalen * 8);
                    msg->setDataLen(datalen);
                    msg->setDataArraySize(datalen);
                    for (int32 i = 0; i < datalen; i++)
                        msg->setData(i, dc->user_data[i]);

                    chunk->encapsulate(msg);
                }
                EV_DETAIL << "datachunkLength=" << chunk->getByteLength() << "\n";
                dest->addChunk(chunk);
                break;
            }

            case INIT: {
                EV << "parse INIT\n";
                const struct init_chunk *init_chunk = (struct init_chunk *)(chunks + chunkPtr);    // (recvBuffer + size_ip + size_common_header);
                struct tlv *cp;
                struct random_parameter *rp;
                struct hmac_algo *hp;
                unsigned int rplen = 0, hplen = 0, cplen = 0;
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
                if (cLen > size_init_chunk) {
                    int32 parcounter = 0, addrcounter = 0;
                    parptr = 0;
                    int chkcounter = 0;
                    bool stopProcessing = false;
                    while (cLen > size_init_chunk + parptr && !stopProcessing) {
                        EV_INFO << "Process INIT parameters\n";
                        const struct tlv *parameter = (struct tlv *)(((unsigned char *)init_chunk) + size_init_chunk + parptr);
                        paramType = ntohs(parameter->type);
                        EV_INFO << "search for param " << paramType << "\n";
                        switch (paramType) {
                            case SUPPORTED_ADDRESS_TYPES: {
                                const struct supported_address_types_parameter *sup_addr = (struct supported_address_types_parameter *)(((unsigned char *)init_chunk) + size_init_chunk + parptr);
                                if (sup_addr->address_type_1 == ntohs(INIT_PARAM_IPV4) || sup_addr->address_type_2 == ntohs(INIT_PARAM_IPV4)) {
                                    chunk->setIpv4Supported(true);
                                } else {
                                    chunk->setIpv4Supported(false);
                                }
                                if (sup_addr->address_type_1 == ntohs(INIT_PARAM_IPV6) || sup_addr->address_type_2 == ntohs(INIT_PARAM_IPV6)) {
                                    chunk->setIpv6Supported(true);
                                } else {
                                    chunk->setIpv6Supported(false);
                                }
                                chunklen += 8;
                                break;
                            }

                            case INIT_PARAM_IPV4: {
                                // we supppose an ipv4 address parameter
                                EV_INFO << "IPv4\n";
                                const struct init_ipv4_address_parameter *v4addr;
                                v4addr = (struct init_ipv4_address_parameter *)(((unsigned char *)init_chunk) + size_init_chunk + parptr);
                                chunk->setAddressesArraySize(++addrcounter);
                                L3Address localv4Addr(IPv4Address(ntohl(v4addr->address)));
                                chunk->setAddresses(addrcounter - 1, localv4Addr);
                                chunklen += 8;
                                break;
                            }

                            case INIT_PARAM_IPV6: {
                                EV_INFO << "IPv6\n";
                                const struct init_ipv6_address_parameter *ipv6addr;
                                ipv6addr = (struct init_ipv6_address_parameter *)(((unsigned char *)init_chunk) + size_init_chunk + parptr);
                                IPv6Address ipv6Addr = IPv6Address(ipv6addr->address[0], ipv6addr->address[1],
                                            ipv6addr->address[2], ipv6addr->address[3]);
                                L3Address localv6Addr(ipv6Addr);
                                EV_INFO << "address" << ipv6Addr << "\n";
                                chunk->setAddressesArraySize(++addrcounter);
                                chunk->setAddresses(addrcounter - 1, localv6Addr);
                                chunklen += 20;
                                break;
                            }

                            case SUPPORTED_EXTENSIONS: {
                                EV_INFO << "Supported extensions\n";
                                const struct supported_extensions_parameter *supext;
                                supext = (struct supported_extensions_parameter *)(((unsigned char *)init_chunk) + size_init_chunk + parptr);
                                unsigned short chunkTypes;
                                chunklen += 4;
                                int len = 4;
                                EV_INFO << "supext->len=" << ntohs(supext->length) << "\n";
                                while (ntohs(supext->length) > len) {
                                    chunkTypes = (int)*(chunks + chunkPtr + size_init_chunk + parptr + 4 + chkcounter);
                                    chunk->setSepChunksArraySize(++chkcounter);
                                    EV_INFO << "Extension " << chunkTypes << " added\n";
                                    chunk->setSepChunks(chkcounter - 1, chunkTypes);
                                    chunklen++;
                                    len++;
                                }
                                break;
                            }

                            case FORWARD_TSN_SUPPORTED_PARAMETER: {
                                EV_INFO << "Forward TSN\n";
                                int size = chunk->getChunkTypesArraySize();
                                chunk->setChunkTypesArraySize(size + 1);
                                chunk->setChunkTypes(size, FORWARD_TSN_SUPPORTED_PARAMETER);
                                chunklen++;
                                break;
                            }

                            case RANDOM: {
                                EV_INFO << "random parameter received\n";
                                const struct random_parameter *rand;
                                rand = (struct random_parameter *)(((unsigned char *)init_chunk) + size_init_chunk + parptr);
                                unsigned char *rv = (unsigned char *)malloc(64);
                                rp = (struct random_parameter *)((unsigned char *)rv);
                                rp->type = rand->type;
                                rplen = ntohs(rand->length);
                                rp->length = rand->length;
                                int rlen = ntohs(rand->length) - 4;
                                chunk->setRandomArraySize(rlen);
                                for (int i = 0; i < rlen; i++) {
                                    chunk->setRandom(i, (unsigned char)(rand->random[i]));
                                    rp->random[i] = (unsigned char)(rand->random[i]);
                                }
                                chunklen += parameter->length / 8;
                                break;
                            }

                            case HMAC_ALGO: {
                                EV_INFO << "hmac_algo parameter received\n";
                                const struct hmac_algo *hmac;
                                hmac = (struct hmac_algo *)(((unsigned char *)init_chunk) + size_init_chunk + parptr);
                                int num = (ntohs(hmac->length) - 4) / 2;
                                chunk->setHmacTypesArraySize(num);
                                unsigned char *hv = (unsigned char *)malloc(64);
                                hp = (struct hmac_algo *)((unsigned char *)hv);
                                hp->type = hmac->type;
                                hplen = ntohs(hmac->length);
                                hp->length = hmac->length;
                                for (int i = 0; i < num; i++) {
                                    chunk->setHmacTypes(i, ntohs(hmac->ident[i]));
                                    hp->ident[i] = hmac->ident[i];
                                }
                                chunklen += 4 + 2 * num;
                                break;
                            }

                            case CHUNKS: {
                                EV_INFO << "chunks parameter received\n";
                                const struct tlv *chunks;
                                chunks = (struct tlv *)(((unsigned char *)init_chunk) + size_init_chunk + parptr);
                                unsigned char *cv = (unsigned char *)malloc(64);
                                cp = (struct tlv *)((unsigned char *)cv);
                                cp->type = chunks->type;
                                cplen = ntohs(chunks->length);
                                cp->length = chunks->length;
                                int num = cplen - 4;
                                chunk->setChunkTypesArraySize(num);
                                for (int i = 0; i < num; i++) {
                                    chunk->setChunkTypes(i, (chunks->value[i]));
                                    cp->value[i] = chunks->value[i];
                                }
                                chunklen += parameter->length / 8;
                                break;
                            }

                            default: {
                                EV_INFO << "ExtInterface: Unknown SCTP INIT parameter type " << paramType << "\n";
                                uint16 skip = (paramType & 0x8000) >> 15;
                                if (skip == 0)
                                    stopProcessing = true;
                                uint16 report = (paramType & 0x4000) >> 14;
                                if (report != 0) {
                                    const struct tlv *unknown;
                                    unknown = (struct tlv *)(((unsigned char *)init_chunk) + size_init_chunk + parptr);
                                    uint32 unknownLen = chunk->getUnrecognizedParametersArraySize();
                                    chunk->setUnrecognizedParametersArraySize(unknownLen + ADD_PADDING(ntohs(unknown->length)));
                                    struct data_vector *dv = (struct data_vector *)(((unsigned char *)init_chunk) + size_init_chunk + parptr);

                                    for (uint32 i = unknownLen; i < unknownLen + ADD_PADDING(ntohs(unknown->length)); i++)
                                        chunk->setUnrecognizedParameters(i, dv->data[i - unknownLen]);
                                }
                                EV_INFO << "stopProcessing=" << stopProcessing << " report=" << report << "\n";
                                break;
                            }
                        }
                        parptr += ADD_PADDING(ntohs(parameter->length));
                        parcounter++;
                    }
                }
                if (chunk->getHmacTypesArraySize() != 0) {
                    unsigned char *vector = (unsigned char *)malloc(64);
                    sizePeerKeyVector = rplen;
                    memcpy(vector, rp, rplen);
                    for (unsigned int k = 0; k < sizePeerKeyVector; k++) {
                        peerKeyVector[k] = vector[k];
                    }
                    memcpy(vector, cp, cplen);
                    for (unsigned int k = 0; k < cplen; k++) {
                        peerKeyVector[sizePeerKeyVector + k] = vector[k];
                    }
                    sizePeerKeyVector += cplen;
                    memcpy(vector, hp, hplen);
                    for (unsigned int k = 0; k < hplen; k++) {
                        peerKeyVector[sizePeerKeyVector + k] = vector[k];
                    }
                    sizePeerKeyVector += hplen;
                    free(vector);
                }
                chunk->setBitLength(chunklen * 8);
                dest->addChunk(chunk);
                //chunkPtr += cLen;
                break;
            }

            case INIT_ACK: {
                const struct init_ack_chunk *iac = (struct init_ack_chunk *)(chunks + chunkPtr);
                struct tlv *cp = nullptr;
                struct random_parameter *rp = nullptr;
                struct hmac_algo *hp = nullptr;
                unsigned int rplen = 0, hplen = 0, cplen = 0;
                chunklen = SCTP_INIT_CHUNK_LENGTH;
                SCTPInitAckChunk *chunk = new SCTPInitAckChunk("INIT_ACK");
                chunk->setChunkType(chunkType);
                chunk->setInitTag(ntohl(iac->initiate_tag));
                chunk->setA_rwnd(ntohl(iac->a_rwnd));
                chunk->setNoOutStreams(ntohs(iac->mos));
                chunk->setNoInStreams(ntohs(iac->mis));
                chunk->setInitTSN(ntohl(iac->initial_tsn));
                chunk->setUnrecognizedParametersArraySize(0);
                if (cLen > size_init_ack_chunk) {
                    int32 parcounter = 0, addrcounter = 0;
                    parptr = 0;
                    int chkcounter = 0;
                    bool stopProcessing = false;
                    //sctpEV3<<"cLen="<<cLen<<"\n";
                    while (cLen > size_init_ack_chunk + parptr && !stopProcessing) {
                        const struct tlv *parameter = (struct tlv *)(((unsigned char *)iac) + size_init_ack_chunk + parptr);
                        paramType = ntohs(parameter->type);
                        //sctpEV3<<"ParamType = "<<paramType<<" parameterLength="<<ntohs(parameter->length)<<"\n";
                        switch (paramType) {
                            case SUPPORTED_ADDRESS_TYPES: {
                                const struct supported_address_types_parameter *sup_addr = (struct supported_address_types_parameter *)(((unsigned char *)iac) + size_init_ack_chunk + parptr);
                                if (sup_addr->address_type_1 == ntohs(INIT_PARAM_IPV4) || sup_addr->address_type_2 == ntohs(INIT_PARAM_IPV4)) {
                                    chunk->setIpv4Supported(true);
                                } else {
                                    chunk->setIpv4Supported(false);
                                }
                                if (sup_addr->address_type_1 == ntohs(INIT_PARAM_IPV6) || sup_addr->address_type_2 == ntohs(INIT_PARAM_IPV6)) {
                                    chunk->setIpv6Supported(true);
                                } else {
                                    chunk->setIpv6Supported(false);
                                }
                                chunklen += 8;
                                break;
                            }

                            case INIT_PARAM_IPV4: {
                                EV_INFO << "parse IPv4\n";
                                const struct init_ipv4_address_parameter *v4addr;
                                v4addr = (struct init_ipv4_address_parameter *)(((unsigned char *)iac) + size_init_ack_chunk + parptr);
                                chunk->setAddressesArraySize(++addrcounter);
                                L3Address localv4Addr(IPv4Address(ntohl(v4addr->address)));
                                chunk->setAddresses(addrcounter - 1, localv4Addr);
                                chunklen += 8;
                                break;
                            }

                            case INIT_PARAM_IPV6: {
                                EV_INFO << "IPv6\n";
                                const struct init_ipv6_address_parameter *ipv6addr;
                                ipv6addr = (struct init_ipv6_address_parameter *)(((unsigned char *)iac) + size_init_chunk + parptr);
                                IPv6Address ipv6Addr = IPv6Address(ipv6addr->address[0], ipv6addr->address[1],
                                            ipv6addr->address[2], ipv6addr->address[3]);
                                EV_INFO << "address" << ipv6Addr << "\n";
                                L3Address localv6Addr(ipv6Addr);

                                chunk->setAddressesArraySize(++addrcounter);
                                chunk->setAddresses(addrcounter - 1, localv6Addr);
                                chunklen += 20;
                                break;
                            }

                            case RANDOM: {
                                const struct random_parameter *rand;
                                rand = (struct random_parameter *)(((unsigned char *)iac) + size_init_ack_chunk + parptr);
                                int rlen = ntohs(rand->length) - 4;
                                chunk->setRandomArraySize(rlen);
                                rp = (struct random_parameter *)((unsigned char *)malloc(64));
                                rp->type = rand->type;
                                rplen = ntohs(rand->length);
                                rp->length = rand->length;
                                for (int i = 0; i < rlen; i++) {
                                    chunk->setRandom(i, (unsigned char)(rand->random[i]));
                                    rp->random[i] = (unsigned char)(rand->random[i]);
                                }

                                chunklen += ntohs(parameter->length) / 8;
                                break;
                            }

                            case HMAC_ALGO: {
                                const struct hmac_algo *hmac;
                                hmac = (struct hmac_algo *)(((unsigned char *)iac) + size_init_ack_chunk + parptr);
                                int num = (ntohs(hmac->length) - 4) / 2;
                                chunk->setHmacTypesArraySize(num);
                                hp = (struct hmac_algo *)((unsigned char *)malloc(64));
                                hp->type = hmac->type;
                                hplen = ntohs(hmac->length);
                                hp->length = hmac->length;
                                for (int i = 0; i < num; i++) {
                                    chunk->setHmacTypes(i, ntohs(hmac->ident[i]));
                                    hp->ident[i] = hmac->ident[i];
                                }
                                chunklen += 4 + 2 * num;
                                break;
                            }

                            case CHUNKS: {
                                const struct tlv *chunks;
                                chunks = (struct tlv *)(((unsigned char *)iac) + size_init_ack_chunk + parptr);
                                int num = ntohs(chunks->length) - 4;
                                chunk->setChunkTypesArraySize(num);
                                cp = (struct tlv *)((unsigned char *)malloc(64));
                                cp->type = chunks->type;
                                cplen = ntohs(chunks->length);
                                cp->length = chunks->length;
                                for (int i = 0; i < num; i++) {
                                    chunk->setChunkTypes(i, chunks->value[i]);
                                    cp->value[i] = chunks->value[i];
                                }
                                chunklen += ntohs(parameter->length) / 8;
                                break;
                            }

                            case INIT_PARAM_COOKIE: {
                                const struct tlv *cookie = (struct tlv *)(((unsigned char *)iac) + size_init_ack_chunk + parptr);
                                int32 cookieLen = ntohs(cookie->length) - 4;
                                // put cookie data into chunk (char array cookie)
                                chunk->setCookieArraySize(cookieLen);
                                for (int32 i = 0; i < cookieLen; i++)
                                    chunk->setCookie(i, cookie->value[i]);
                                chunklen += cookieLen + 4;
                                break;
                            }

                            case SUPPORTED_EXTENSIONS: {
                                const struct supported_extensions_parameter *supext;
                                supext = (struct supported_extensions_parameter *)(((unsigned char *)iac) + size_init_ack_chunk + parptr);
                                unsigned short chunkTypes;
                                chunklen += 4;
                                int len = 4;
                                while (ntohs(supext->length) > len) {
                                    chunkTypes = (int)*(chunks + chunkPtr + size_init_ack_chunk + parptr + 4 + chkcounter);
                                    chunk->setSepChunksArraySize(++chkcounter);
                                    chunk->setSepChunks(chkcounter - 1, chunkTypes);
                                    chunklen++;
                                    len++;
                                }
                                break;
                            }

                            case FORWARD_TSN_SUPPORTED_PARAMETER: {
                                int size = chunk->getChunkTypesArraySize();
                                chunk->setChunkTypesArraySize(size + 1);
                                chunk->setChunkTypes(size, FORWARD_TSN_SUPPORTED_PARAMETER);
                                chunklen++;
                                break;
                            }

                            default: {
                                EV_INFO << "ExtInterface: Unknown SCTP INIT-ACK parameter type " << paramType << "\n";
                                uint16 skip = (paramType & 0x8000) >> 15;
                                if (skip == 0)
                                    stopProcessing = true;
                                uint16 report = (paramType & 0x4000) >> 14;
                                if (report != 0) {
                                    const struct tlv *unknown;
                                    unknown = (struct tlv *)(((unsigned char *)iac) + size_init_ack_chunk + parptr);
                                    uint32 unknownLen = chunk->getUnrecognizedParametersArraySize();
                                    chunk->setUnrecognizedParametersArraySize(unknownLen + ADD_PADDING(ntohs(unknown->length)));
                                    struct data_vector *dv = (struct data_vector *)(((unsigned char *)iac) + size_init_ack_chunk + parptr);

                                    for (uint32 i = unknownLen; i < unknownLen + ADD_PADDING(ntohs(unknown->length)); i++)
                                        chunk->setUnrecognizedParameters(i, dv->data[i - unknownLen]);
                                }
                                EV_INFO << "stopProcessing=" << stopProcessing << "  report=" << report << "\n";

                                break;
                            }
                        }
                        parptr += ADD_PADDING(ntohs(parameter->length));
                        //sctpEV3<<"parptr="<<parptr<<"\n";
                        parcounter++;
                    }
                }
                if (chunk->getHmacTypesArraySize() != 0) {
                    unsigned char vector[64];
                    if (rplen > 64) {
                        EV_ERROR << "Random parameter too long. It will be truncated.\n";
                        rplen = 64;
                    }
                    sizePeerKeyVector = rplen;
                    memcpy(vector, rp, rplen);
                    for (unsigned int k = 0; k < sizePeerKeyVector; k++) {
                        peerKeyVector[k] = vector[k];
                    }
                    free(rp);
                    if (cplen > 64) {
                        EV_ERROR << "Chunks parameter too long. It will be truncated.\n";
                        cplen = 64;
                    }
                    memcpy(vector, cp, cplen);
                    for (unsigned int k = 0; k < cplen; k++) {
                        peerKeyVector[sizePeerKeyVector + k] = vector[k];
                    }
                    free(cp);
                    sizePeerKeyVector += cplen;
                    if (hplen > 64) {
                        EV_ERROR << "HMac parameter too long. It will be truncated.\n";
                        hplen = 64;
                    }
                    memcpy(vector, hp, hplen);
                    for (unsigned int k = 0; k < hplen; k++) {
                        peerKeyVector[sizePeerKeyVector + k] = vector[k];
                    }
                    free(hp);
                    sizePeerKeyVector += hplen;
                    calculateSharedKey();
                }
                chunk->setBitLength(chunklen * 8);
                dest->addChunk(chunk);
                break;
            }

            case SACK: {
                EV << "SCTPMessage: SACK received\n";
                const struct sack_chunk *sac = (struct sack_chunk *)(chunks + chunkPtr);
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

                for (int32 i = 0; i < ngaps; i++) {
                    const struct sack_gap *gap = (struct sack_gap *)(((unsigned char *)sac) + size_sack_chunk + i * sizeof(sack_gap));
                    chunk->setGapStart(i, ntohs(gap->start) + cumtsnack);
                    chunk->setGapStop(i, ntohs(gap->stop) + cumtsnack);
                }
                for (int32 i = 0; i < ndups; i++) {
                    const struct sack_duptsn *dup = (struct sack_duptsn *)(((unsigned char *)sac) + size_sack_chunk + ngaps * sizeof(sack_gap) + i * sizeof(sack_duptsn));
                    chunk->setDupTsns(i, ntohl(dup->tsn));
                }

                chunk->setBitLength(cLen * 8);
                dest->addChunk(chunk);
                break;
            }

            case HEARTBEAT: {
                //sctpEV3<<"SCTPMessage: Heartbeat received\n";
                const struct heartbeat_chunk *hbc = (struct heartbeat_chunk *)(chunks + chunkPtr);
                SCTPHeartbeatChunk *chunk = new SCTPHeartbeatChunk("HEARTBEAT");
                chunk->setChunkType(chunkType);
                if (cLen > size_heartbeat_chunk) {
                    int32 parcounter = 0;
                    parptr = 0;
                    while (cLen > size_heartbeat_chunk + parptr) {
                        // we supppose type 1 here
                        //sctpEV3<<"HB-chunk+parptr="<<size_heartbeat_chunk+parptr<<"\n";
                        const struct heartbeat_info *hbi = (struct heartbeat_info *)(((unsigned char *)hbc) + size_heartbeat_chunk + parptr);
                        if (ntohs(hbi->type) == 1) {    // sender specific hb info
                            //sctpEV3<<"HBInfo\n";
                            int32 infoLen = ntohs(hbi->length) - 4;
                            parptr += ADD_PADDING(infoLen) + 4;
                            parcounter++;
                            chunk->setInfoArraySize(infoLen);
                            for (int32 i = 0; i < infoLen; i++)
                                chunk->setInfo(i, HBI_INFO(hbi)[i]);
                        }
                        else {
                            parptr += ADD_PADDING(ntohs(hbi->length));    // set pointer forwards with count of bytes in length field of TLV
                            parcounter++;
                            continue;
                        }
                    }
                }
                chunk->setBitLength(cLen * 8);
                dest->addChunk(chunk);
                break;
            }

            case HEARTBEAT_ACK: {
                EV << "SCTPMessage: Heartbeat_Ack received\n";
                const struct heartbeat_ack_chunk *hbac = (struct heartbeat_ack_chunk *)(chunks + chunkPtr);
                SCTPHeartbeatAckChunk *chunk = new SCTPHeartbeatAckChunk("HEARTBEAT_ACK");
                chunk->setChunkType(chunkType);
                if (cLen > size_heartbeat_ack_chunk) {
                    int32 parcounter = 0;
                    parptr = 0;
                    while (cLen > size_heartbeat_ack_chunk + parptr) {
                        // we supppose type 1 here, the same provided in heartbeat chunks
                        const struct heartbeat_info *hbi = (struct heartbeat_info *)(((unsigned char *)hbac) + size_heartbeat_ack_chunk + parptr);
                        if (ntohs(hbi->type) == 1) {    // sender specific hb info
                            uint16 ilen = ntohs(hbi->length);
                            ASSERT(ilen >= 4 && ilen == cLen - size_heartbeat_ack_chunk);
                            uint16 infoLen = ilen - 4;
                            parptr += ADD_PADDING(infoLen) + 4;
                            parcounter++;
                            chunk->setRemoteAddr(L3Address(IPv4Address(ntohl(HBI_ADDR(hbi).v4addr.address))));
                            chunk->setTimeField(ntohl((uint32)HBI_TIME(hbi)));
                            chunk->setInfoArraySize(infoLen);
                            for (int32 i = 0; i < infoLen; i++)
                                chunk->setInfo(i, HBI_INFO(hbi)[i]);
                        }
                        else {
                            parptr += ntohs(hbi->length);    // set pointer forwards with count of bytes in length field of TLV
                            parcounter++;
                            continue;
                        }
                    }
                }
                chunk->setBitLength(cLen * 8);
                dest->addChunk(chunk);
                break;
            }

            case ABORT: {
                EV << "SCTPMessage: Abort received\n";
                const struct abort_chunk *ac = (struct abort_chunk *)(chunks + chunkPtr);
                cLen = ntohs(ac->length);
                SCTPAbortChunk *chunk = new SCTPAbortChunk("ABORT");
                chunk->setChunkType(chunkType);
                chunk->setT_Bit(ac->flags & T_BIT);
                if (cLen > size_abort_chunk) {
                    // TODO: handle attached error causes
                }
                chunk->setBitLength(cLen * 8);
                dest->addChunk(chunk);
                break;
            }

            case COOKIE_ECHO: {
                SCTPCookieEchoChunk *chunk = new SCTPCookieEchoChunk("COOKIE_ECHO");
                chunk->setChunkType(chunkType);
                EV_INFO << "Parse Cookie-Echo\n";
                if (cLen > size_cookie_echo_chunk) {
                    int32 cookieSize = woPadding - size_cookie_echo_chunk;
                    EV_DETAIL << "cookieSize=" << cookieSize << "\n";
                    const struct cookie_parameter *cookie = (struct cookie_parameter *)(chunks + chunkPtr + 4);
                    SCTPCookie *stateCookie = new SCTPCookie();
                    stateCookie->setCreationTime(ntohl(cookie->creationTime));
                    stateCookie->setLocalTag(ntohl(cookie->localTag));
                    stateCookie->setPeerTag(ntohl(cookie->peerTag));
                    stateCookie->setLocalTieTagArraySize(32);
                    stateCookie->setPeerTieTagArraySize(32);
                    for (int32 i = 0; i < 32; i++) {
                        stateCookie->setLocalTieTag(i, cookie->localTieTag[i]);
                        stateCookie->setPeerTieTag(i, cookie->peerTieTag[i]);
                    }
                    stateCookie->setBitLength(SCTP_COOKIE_LENGTH * 8);
                    chunk->setStateCookie(stateCookie);
                }
                chunk->setBitLength(woPadding * 8);
                dest->addChunk(chunk);
                break;
            }

            case COOKIE_ACK: {
                EV << "SCTPMessage: Cookie_Ack received\n";
                SCTPCookieAckChunk *chunk = new SCTPCookieAckChunk("COOKIE_ACK");
                chunk->setChunkType(chunkType);
                chunk->setBitLength(cLen * 8);
                dest->addChunk(chunk);
                break;
            }

            case SHUTDOWN: {
                EV << "SCTPMessage: Shutdown received\n";
                const struct shutdown_chunk *sc = (struct shutdown_chunk *)(chunks + chunkPtr);
                SCTPShutdownChunk *chunk = new SCTPShutdownChunk("SHUTDOWN");
                chunk->setChunkType(chunkType);
                uint32 cumtsnack = ntohl(sc->cumulative_tsn_ack);
                chunk->setCumTsnAck(cumtsnack);
                chunk->setBitLength(cLen * 8);
                dest->addChunk(chunk);
                break;
            }

            case SHUTDOWN_ACK: {
                EV << "SCTPMessage: ShutdownAck received\n";
                SCTPShutdownAckChunk *chunk = new SCTPShutdownAckChunk("SHUTDOWN_ACK");
                chunk->setChunkType(chunkType);
                chunk->setBitLength(cLen * 8);
                dest->addChunk(chunk);
                break;
            }

            case SHUTDOWN_COMPLETE: {
                EV << "SCTPMessage: ShutdownComplete received\n";
                const struct shutdown_complete_chunk *scc = (struct shutdown_complete_chunk *)(chunks + chunkPtr);
                SCTPShutdownCompleteChunk *chunk = new SCTPShutdownCompleteChunk("SHUTDOWN_COMPLETE");
                chunk->setChunkType(chunkType);
                chunk->setTBit(scc->flags & T_BIT);
                chunk->setBitLength(cLen * 8);
                dest->addChunk(chunk);
                break;
            }

            case ERRORTYPE: {
                const struct error_chunk *error;
                error = (struct error_chunk *)(chunks + chunkPtr);
                SCTPErrorChunk *errorchunk;
                errorchunk = new SCTPErrorChunk("ERROR");
                errorchunk->setChunkType(chunkType);
                errorchunk->setBitLength(SCTP_ERROR_CHUNK_LENGTH * 8);
                parptr = 0;
                const struct error_cause *err = (struct error_cause *)(((unsigned char *)error) + size_error_chunk + parptr);
                if (err->cause_code == UNSUPPORTED_HMAC) {
                    SCTPSimpleErrorCauseParameter *errParam;
                    errParam = new SCTPSimpleErrorCauseParameter();
                    errParam->setParameterType(err->cause_code);
                    errParam->setByteLength(err->length);
                    errorchunk->addParameters(errParam);
                }
                dest->addChunk(errorchunk);
                break;
            }

            case FORWARD_TSN: {
                const struct forward_tsn_chunk *forward_tsn_chunk;
                forward_tsn_chunk = (struct forward_tsn_chunk *)(chunks + chunkPtr);
                SCTPForwardTsnChunk *chunk;
                chunk = new SCTPForwardTsnChunk("FORWARD_TSN");
                chunk->setChunkType(chunkType);
                chunk->setName("FORWARD_TSN");
                chunk->setNewCumTsn(ntohl(forward_tsn_chunk->cum_tsn));
                int streamNumber = 0, streamptr = 0;
                while (cLen > size_forward_tsn_chunk + streamptr) {
                    const struct forward_tsn_streams *forward = (struct forward_tsn_streams *)(((unsigned char *)forward_tsn_chunk) + size_forward_tsn_chunk + streamptr);
                    chunk->setSidArraySize(++streamNumber);
                    chunk->setSid(streamNumber - 1, ntohs(forward->sid));
                    chunk->setSsnArraySize(streamNumber);
                    chunk->setSsn(streamNumber - 1, ntohs(forward->ssn));
                    streamptr += sizeof(struct forward_tsn_streams);
                }
                chunk->setByteLength(cLen);
                dest->addChunk(chunk);
                break;
            }

            case AUTH: {
                int hmacSize;
                struct auth_chunk *ac = (struct auth_chunk *)(chunks + chunkPtr);
                SCTPAuthenticationChunk *chunk = new SCTPAuthenticationChunk("AUTH");
                chunk->setChunkType(chunkType);
                chunk->setSharedKey(ntohs(ac->shared_key));
                chunk->setHMacIdentifier(ntohs(ac->hmac_identifier));
                if (cLen > size_auth_chunk) {
                    hmacSize = woPadding - size_auth_chunk;
                    chunk->setHMACArraySize(hmacSize);
                    for (int i = 0; i < hmacSize; i++) {
                        chunk->setHMAC(i, ac->hmac[i]);
                        ac->hmac[i] = 0;
                    }
                }

                unsigned char result[SHA_LENGTH];
                unsigned int flen;
                flen = bufsize - (size_common_header + chunkPtr);

                const struct data_vector *sc = (struct data_vector *)(chunks + chunkPtr);
                hmacSha1((uint8 *)sc->data, flen, sharedKey, sizeKeyVector + sizePeerKeyVector, result);

                chunk->setHMacOk(true);
                for (unsigned int j = 0; j < SHA_LENGTH; j++) {
                    if (result[j] != chunk->getHMAC(j)) {
                        EV_DETAIL << "hmac falsch\n";
                        chunk->setHMacOk(false);
                        break;
                    }
                }
                chunk->setByteLength(woPadding);
                dest->addChunk(chunk);
                break;
            }

            case ASCONF: {
                const struct asconf_chunk *asconf_chunk = (struct asconf_chunk *)(chunks + chunkPtr);    // (recvBuffer + size_ip + size_common_header);
                int paramLength = 0;
                SCTPAsconfChunk *chunk = new SCTPAsconfChunk("ASCONF");
                chunk->setChunkType(chunkType);
                chunk->setName("ASCONF");
                chunk->setSerialNumber(ntohl(asconf_chunk->serial));
                if (cLen > size_asconf_chunk) {
                    int parcounter = 0;
                    parptr = 0;
                    // we supppose an ipv4 address parameter
                    const struct init_ipv4_address_parameter *ipv4addr = (struct init_ipv4_address_parameter *)(((unsigned char *)asconf_chunk) + size_asconf_chunk + parptr);
                    int parlen = ADD_PADDING(ntohs(ipv4addr->length));
                    parptr += parlen;
                    // set pointer forwards with count of bytes in length field of TLV
                    parcounter++;
                    if (ntohs(ipv4addr->type) != INIT_PARAM_IPV4) {
                        if (parlen == 0)
                            throw new cRuntimeError("ParamLen == 0.");
                        continue;
                    }
                    else {
                        L3Address localAddr(IPv4Address(ntohl(ipv4addr->address)));
                        chunk->setAddressParam(localAddr);
                    }
                    while (cLen > size_asconf_chunk + parptr) {
                        const struct add_ip_parameter *ipparam = (struct add_ip_parameter *)(((unsigned char *)asconf_chunk) + size_asconf_chunk + parptr);
                        paramType = ntohs(ipparam->type);
                        paramLength = ntohs(ipparam->length);
                        switch (paramType) {
                            case ADD_IP_ADDRESS: {
                                EV_INFO << "parse ADD_IP_ADDRESS\n";
                                SCTPAddIPParameter *addip;
                                addip = new SCTPAddIPParameter("ADD_IP");
                                addip->setParameterType(ntohs(ipparam->type));
                                addip->setRequestCorrelationId(ntohl(ipparam->correlation_id));
                                const struct init_ipv4_address_parameter *v4addr1;
                                v4addr1 = (struct init_ipv4_address_parameter *)(((unsigned char *)asconf_chunk) + size_asconf_chunk + parptr + size_addip_parameter);
                                L3Address localAddr(IPv4Address(ntohl(v4addr1->address)));
                                addip->setAddressParam(localAddr);
                                chunk->addAsconfParam(addip);
                                break;
                            }

                            case DELETE_IP_ADDRESS: {
                                EV_INFO << "parse DELETE_IP_ADDRESS\n";
                                SCTPDeleteIPParameter *deleteip;
                                deleteip = new SCTPDeleteIPParameter("DELETE_IP");
                                deleteip->setParameterType(ntohs(ipparam->type));
                                deleteip->setRequestCorrelationId(ntohl(ipparam->correlation_id));
                                const struct init_ipv4_address_parameter *v4addr2;
                                v4addr2 = (struct init_ipv4_address_parameter *)(((unsigned char *)asconf_chunk) + size_asconf_chunk + parptr + size_addip_parameter);
                                L3Address localAddr(IPv4Address(ntohl(v4addr2->address)));
                                deleteip->setAddressParam(localAddr);
                                chunk->addAsconfParam(deleteip);
                                break;
                            }

                            case SET_PRIMARY_ADDRESS: {
                                EV_INFO << "parse SET_PRIMARY_ADDRESS\n";
                                SCTPSetPrimaryIPParameter *priip;
                                priip = new SCTPSetPrimaryIPParameter("SET_PRI_IP");
                                priip->setParameterType(ntohs(ipparam->type));
                                priip->setRequestCorrelationId(ntohl(ipparam->correlation_id));
                                const struct init_ipv4_address_parameter *v4addr3;
                                v4addr3 = (struct init_ipv4_address_parameter *)(((unsigned char *)asconf_chunk) + size_asconf_chunk + parptr + size_addip_parameter);
                                L3Address localAddr(IPv4Address(ntohl(v4addr3->address)));
                                priip->setAddressParam(localAddr);
                                chunk->addAsconfParam(priip);
                                break;
                            }

                            default:
                                printf("TODO: Implement parameter type %d!\n", paramType);
                                EV << "ExtInterface: Unknown SCTP parameter type " << paramType;
                                /*throw new cRuntimeError("TODO: unknown parametertype in incoming packet from external interface! Implement it!");*/
                                break;
                        }
                        parptr += ADD_PADDING(paramLength);
                        parcounter++;
                    }
                }
                chunk->setByteLength(cLen);
                dest->addChunk(chunk);
                break;
            }

            case ASCONF_ACK: {
                const struct asconf_ack_chunk *asconf_ack_chunk = (struct asconf_ack_chunk *)(chunks + chunkPtr);    // (recvBuffer + size_ip + size_common_header);
                int paramLength = 0;
                SCTPAsconfAckChunk *chunk = new SCTPAsconfAckChunk("ASCONF_ACK");
                chunk->setChunkType(chunkType);
                chunk->setName("ASCONF_ACK");
                chunk->setSerialNumber(ntohl(asconf_ack_chunk->serial));
                if (cLen > size_asconf_ack_chunk) {
                    int parcounter = 0;
                    parptr = 0;

                    while (cLen > size_asconf_ack_chunk + parptr) {
                        const struct add_ip_parameter *ipparam = (struct add_ip_parameter *)(((unsigned char *)asconf_ack_chunk) + size_asconf_ack_chunk + parptr);
                        paramType = ntohs(ipparam->type);
                        paramLength = ntohs(ipparam->length);
                        switch (paramType) {
                            case ERROR_CAUSE_INDICATION: {
                                SCTPErrorCauseParameter *errorip;
                                errorip = new SCTPErrorCauseParameter("ERROR_CAUSE");
                                errorip->setParameterType(ntohs(ipparam->type));
                                errorip->setResponseCorrelationId(ntohl(ipparam->correlation_id));
                                const struct error_cause *errorcause;
                                errorcause = (struct error_cause *)(((unsigned char *)asconf_ack_chunk) + size_asconf_ack_chunk + parptr + size_addip_parameter);
                                errorip->setErrorCauseType(htons(errorcause->cause_code));
                                chunk->addAsconfResponse(errorip);
                                break;
                            }

                            case SUCCESS_INDICATION: {
                                SCTPSuccessIndication *success;
                                success = new SCTPSuccessIndication("SUCCESS");
                                success->setParameterType(ntohs(ipparam->type));
                                success->setResponseCorrelationId(ntohl(ipparam->correlation_id));
                                chunk->addAsconfResponse(success);
                                break;
                            }

                            default:
                                printf("TODO: Implement parameter type %d!\n", paramType);
                                EV << "ExtInterface: Unknown SCTP parameter type " << paramType;
                                break;
                        }
                        parptr += ADD_PADDING(paramLength);
                        parcounter++;
                    }
                }
                chunk->setByteLength(cLen);
                dest->addChunk(chunk);
                break;
            }

            case STREAM_RESET: {
                const struct stream_reset_chunk *stream_reset_chunk;
                stream_reset_chunk = (struct stream_reset_chunk *)(chunks + chunkPtr);
                SCTPStreamResetChunk *chunk;
                chunk = new SCTPStreamResetChunk("STREAM_RESET");
                chunk->setChunkType(chunkType);
                chunk->setName("STREAM_RESET");
                chunk->setByteLength(SCTP_STREAM_RESET_CHUNK_LENGTH);
                chunklen = SCTP_STREAM_RESET_CHUNK_LENGTH;
                int len;
                if ((unsigned int)cLen > sizeof(struct stream_reset_chunk)) {
                    parptr = 0;
                    int parcounter = 0;
                    int snnumbers, sncounter;
                    while (cLen > size_stream_reset_chunk + parptr) {
                        const struct tlv *parameter = (struct tlv *)(((unsigned char *)stream_reset_chunk) + size_stream_reset_chunk + parptr);
                        paramType = ntohs(parameter->type);
                        int paramLength = ntohs(parameter->length);
                        switch (paramType) {
                            case OUTGOING_RESET_REQUEST_PARAMETER: {
                                const struct outgoing_reset_request_parameter *outrr;
                                outrr = (struct outgoing_reset_request_parameter *)(((unsigned char *)stream_reset_chunk) + size_stream_reset_chunk + parptr);
                                SCTPOutgoingSSNResetRequestParameter *outstrrst;
                                outstrrst = new SCTPOutgoingSSNResetRequestParameter("OUT_STR_RST");
                                outstrrst->setParameterType(OUTGOING_RESET_REQUEST_PARAMETER);
                                outstrrst->setSrReqSn(ntohl(outrr->srReqSn));    //Stream Reset Request Sequence Number
                                outstrrst->setSrResSn(ntohl(outrr->srResSn));    //Stream Reset Response Sequence Number
                                outstrrst->setLastTsn(ntohl(outrr->lastTsn));    //Senders last assigned TSN
                                chunklen += SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH;
                                len = SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH;
                                sncounter = 0;
                                while (ntohs(outrr->length) > len) {
                                    snnumbers = (int)*(chunks + chunkPtr + size_stream_reset_chunk + parptr + SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH + sncounter * 2);
                                    outstrrst->setStreamNumbersArraySize(++sncounter);
                                    outstrrst->setStreamNumbers(sncounter - 1, snnumbers);
                                    chunklen += 2;
                                    len += 2;
                                }
                                chunk->addParameter(outstrrst);
                                break;
                            }

                            case INCOMING_RESET_REQUEST_PARAMETER: {
                                const struct incoming_reset_request_parameter *inrr;
                                inrr = (struct incoming_reset_request_parameter *)(((unsigned char *)stream_reset_chunk) + size_stream_reset_chunk + parptr);
                                SCTPIncomingSSNResetRequestParameter *instrrst;
                                instrrst = new SCTPIncomingSSNResetRequestParameter("IN_STR_RST");
                                instrrst->setParameterType(INCOMING_RESET_REQUEST_PARAMETER);
                                instrrst->setSrReqSn(ntohl(inrr->srReqSn));    //Stream Reset Request Sequence Number
                                chunklen += SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH;
                                len = SCTP_INCOMING_RESET_REQUEST_PARAMETER_LENGTH;
                                sncounter = 0;
                                while (ntohs(inrr->length) > len) {
                                    snnumbers = (int)*(chunks + chunkPtr + size_stream_reset_chunk + parptr + SCTP_OUTGOING_RESET_REQUEST_PARAMETER_LENGTH + sncounter * 2);
                                    instrrst->setStreamNumbersArraySize(++sncounter);
                                    instrrst->setStreamNumbers(sncounter - 1, snnumbers);
                                    chunklen += 2;
                                    len += 2;
                                }
                                chunk->addParameter(instrrst);
                                break;
                            }

                            case SSN_TSN_RESET_REQUEST_PARAMETER: {
                                const struct ssn_tsn_reset_request_parameter *ssnrr;
                                ssnrr = (struct ssn_tsn_reset_request_parameter *)(((unsigned char *)stream_reset_chunk) + size_stream_reset_chunk + parptr);
                                SCTPSSNTSNResetRequestParameter *ssnstrrst;
                                ssnstrrst = new SCTPSSNTSNResetRequestParameter("SSN_STR_RST");
                                ssnstrrst->setParameterType(SSN_TSN_RESET_REQUEST_PARAMETER);
                                ssnstrrst->setSrReqSn(ntohl(ssnrr->srReqSn));
                                chunklen += SCTP_SSN_TSN_RESET_REQUEST_PARAMETER_LENGTH;
                                chunk->addParameter(ssnstrrst);
                                break;
                            }

                            case STREAM_RESET_RESPONSE_PARAMETER: {
                                const struct stream_reset_response_parameter *resp;
                                resp = (struct stream_reset_response_parameter *)(((unsigned char *)stream_reset_chunk) + size_stream_reset_chunk + parptr);
                                SCTPStreamResetResponseParameter *strrst;
                                strrst = new SCTPStreamResetResponseParameter("STR_RST_RESPONSE");
                                strrst->setParameterType(STREAM_RESET_RESPONSE_PARAMETER);
                                strrst->setSrResSn(ntohl(resp->srResSn));
                                strrst->setResult(ntohl(resp->result));
                                int pLen = SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH;
                                if (cLen > size_stream_reset_chunk + parptr + SCTP_STREAM_RESET_RESPONSE_PARAMETER_LENGTH) {
                                    strrst->setSendersNextTsn(ntohl(resp->sendersNextTsn));
                                    strrst->setReceiversNextTsn(ntohl(resp->receiversNextTsn));
                                    pLen += 8;
                                }
                                strrst->setByteLength(pLen);
                                chunk->addParameter(strrst);
                                break;
                            }
                        }
                        parptr += ADD_PADDING(paramLength);
                        parcounter++;
                    }
                }
                chunk->setByteLength(cLen);
                dest->addChunk(chunk);
                break;
            }

            case PKTDROP: {
                const struct pktdrop_chunk *drop;
                drop = (struct pktdrop_chunk *)(chunks + chunkPtr);
                SCTPPacketDropChunk *dropChunk;
                dropChunk = new SCTPPacketDropChunk("PKTDROP");
                dropChunk->setChunkType(PKTDROP);
                dropChunk->setCFlag(drop->flags & C_FLAG);
                dropChunk->setTFlag(drop->flags & T_FLAG);
                dropChunk->setBFlag(drop->flags & B_FLAG);
                dropChunk->setMFlag(drop->flags & M_FLAG);
                dropChunk->setMaxRwnd(ntohl(drop->max_rwnd));
                dropChunk->setQueuedData(ntohl(drop->queued_data));
                dropChunk->setTruncLength(ntohs(drop->trunc_length));
                EV_INFO << "SCTPSerializer::pktdrop: parse SCTPMessage\n";
                SCTPMessage *msg;
                msg = new SCTPMessage();
                parse((unsigned char *)chunks + chunkPtr + 16, bufsize - size_common_header - chunkPtr - 16, msg);
                break;
            }

            default:
                //printf("TODO: Implement chunk type %d, found in chunk array on %d!\n", chunkType, ct);
                EV_ERROR << "Parser: Unknown SCTP chunk type " << chunkType;
                break;
        }    // end of switch(chunkType)
        chunkPtr += cLen;
    }    // end of while()
}

bool SCTPSerializer::compareRandom()
{
    unsigned int i, size;
    if (sizeKeyVector != sizePeerKeyVector) {
        if (sizePeerKeyVector > sizeKeyVector) {
            return false;
        }
        else {
            return true;
        }
    }
    else
        size = sizeKeyVector;
    for (i = 0; i < size; i++) {
        if (keyVector[i] < peerKeyVector[i])
            return false;
        if (keyVector[i] > peerKeyVector[i])
            return true;
    }
    return true;
}

void SCTPSerializer::calculateSharedKey()
{
    unsigned int i;
    bool peerFirst = false;

    peerFirst = compareRandom();

    if (peerFirst == false) {
        for (i = 0; i < sizeKeyVector; i++)
            sharedKey[i] = keyVector[i];
        for (i = 0; i < sizePeerKeyVector; i++)
            sharedKey[i + sizeKeyVector] = peerKeyVector[i];
    }
    else {
        for (i = 0; i < sizePeerKeyVector; i++)
            sharedKey[i] = peerKeyVector[i];
        for (i = 0; i < sizeKeyVector; i++)
            sharedKey[i + sizePeerKeyVector] = keyVector[i];
    }
}

} // namespace sctp

} // namespace inet

