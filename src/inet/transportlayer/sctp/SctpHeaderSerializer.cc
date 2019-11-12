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

#include "inet/common/Endian.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/networklayer/common/IpProtocolId_m.h"
#include "inet/networklayer/ipv4/headers/ip.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4HeaderSerializer.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/transportlayer/sctp/SctpChecksum.h"
#include "inet/transportlayer/sctp/SctpHeaderSerializer.h"
#include "inet/transportlayer/sctp/headers/sctphdr.h"


#if !defined(_WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>    // htonl, ntohl, ...
#include <arpa/inet.h>
#include <sys/socket.h>
#endif // if !defined(_WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

#include <sys/types.h>
#define MAXBUFLEN 1<<16

namespace inet {

namespace sctp {

Register_Serializer(SctpHeader, SctpHeaderSerializer);

namespace {

uint8_t paddingLength(uint64_t len) {
    return (4 - (len & 3)) & 3;
}

void serializeDataChunk(MemoryOutputStream& stream, const SctpDataChunk* dataChunk) {
    // RFC 4960, 3.3.1., RFC 7053, 3.
    stream.writeByte(dataChunk->getSctpChunkType());
    stream.writeNBitsOfUint64Be(0, 4);
    stream.writeBit(dataChunk->getIBit());
    stream.writeBit(dataChunk->getUBit());
    stream.writeBit(dataChunk->getBBit());
    stream.writeBit(dataChunk->getEBit());
    stream.writeUint16Be(dataChunk->getByteLength());
    stream.writeUint32Be(dataChunk->getTsn());
    stream.writeUint16Be(dataChunk->getSid());
    stream.writeUint16Be(dataChunk->getSsn());
    stream.writeUint32Be(dataChunk->getPpid());
    SctpSimpleMessage *smsg = check_and_cast<SctpSimpleMessage *>(dataChunk->getEncapsulatedPacket());
    const uint32_t datalen = smsg->getDataLen();
    ASSERT(datalen == dataChunk->getByteLength() - 16);
    if (smsg->getDataArraySize() >= datalen) {
        for (uint32_t i = 0; i < datalen; ++i) {
            stream.writeByte(smsg->getData(i));
        }
    }
}

void deserializeDataChunk(MemoryInputStream& stream, SctpDataChunk* dataChunk) {
    B startPos = stream.getRemainingLength();
    dataChunk->setSctpChunkType(DATA);
    stream.readNBitsToUint64Be(4);
    dataChunk->setIBit(stream.readBit());
    dataChunk->setUBit(stream.readBit());
    dataChunk->setBBit(stream.readBit());
    dataChunk->setEBit(stream.readBit());
    uint16_t length = stream.readUint16Be();
    dataChunk->setLength(length);
    dataChunk->setTsn(stream.readUint32Be());
    dataChunk->setSid(stream.readUint16Be());
    dataChunk->setSsn(stream.readUint16Be());
    dataChunk->setPpid(stream.readUint32Be());
    const uint32_t datalen = length - (B(startPos - stream.getRemainingLength()).get() + 1);    // +1 because the type is read before the initialization of the startPos variable
    if (datalen > 0) {
        SctpSimpleMessage *smsg = new SctpSimpleMessage("data");
        smsg->setBitLength(datalen * 8);
        smsg->setDataLen(datalen);
        smsg->setDataArraySize(datalen);
        for (uint32_t i = 0; i < datalen; ++i) {
            smsg->setData(i, stream.readByte());
        }
        dataChunk->encapsulate(smsg);
    }
    dataChunk->setByteLength(length);
}

void serializeInitChunk(MemoryOutputStream& stream, const SctpInitChunk* initChunk) {
    // RFC 4960, 3.3.2.
    stream.writeByte(initChunk->getSctpChunkType());
    stream.writeByte(0);
    stream.writeUint16Be(initChunk->getByteLength());
    stream.writeUint32Be(initChunk->getInitTag());
    stream.writeUint32Be(initChunk->getA_rwnd());
    stream.writeUint16Be(initChunk->getNoOutStreams());
    stream.writeUint16Be(initChunk->getNoInStreams());
    stream.writeUint32Be(initChunk->getInitTsn());
    // Supported Address Types Parameter (RFC 4960, 3.3.2.1.)
    if (initChunk->getIpv4Supported() || initChunk->getIpv6Supported()) {
        stream.writeUint16Be(INIT_SUPPORTED_ADDRESS);
        stream.writeUint16Be(initChunk->getIpv4Supported() && initChunk->getIpv6Supported() ? 8 : 6);
        if (initChunk->getIpv4Supported() && initChunk->getIpv6Supported()) {
            stream.writeUint16Be(INIT_PARAM_IPV4);
            stream.writeUint16Be(INIT_PARAM_IPV6);
        }
        else if (initChunk->getIpv4Supported())
            stream.writeUint16Be(INIT_PARAM_IPV4);
        else
            stream.writeUint16Be(INIT_PARAM_IPV6);
    }
    // Forward-TSN-Supported Parameter (RFC 3758, 3.1.)
    if (initChunk->getForwardTsn() == true) {
        stream.writeUint16Be(FORWARD_TSN_SUPPORTED_PARAMETER);
        stream.writeUint16Be(4);
    }
    // IPv4 Address Parameter & IPv6 Address Parameter (RFC 4960, 3.3.2.1.)
    for (size_t i = 0; i < initChunk->getAddressesArraySize(); ++i) {
        if (initChunk->getAddresses(i).getType() == L3Address::IPv4) {
            stream.writeUint16Be(INIT_PARAM_IPV4);
            stream.writeUint16Be(8);
            stream.writeIpv4Address(initChunk->getAddresses(i).toIpv4());
        }
        else if (initChunk->getAddresses(i).getType() == L3Address::IPv6) {
            stream.writeUint16Be(INIT_PARAM_IPV6);
            stream.writeUint16Be(20);
            stream.writeIpv6Address(initChunk->getAddresses(i).toIpv6());
        }
    }
    // Supported Extensions Parameter (RFC 5061, 4.2.7.)
    uint64_t chunkCount = initChunk->getSepChunksArraySize();
    if (chunkCount > 0) {
        stream.writeUint16Be(SUPPORTED_EXTENSIONS);
        stream.writeUint16Be(4 + chunkCount);
        for (uint64_t i = 0; i < chunkCount; ++i) {
            stream.writeByte(initChunk->getSepChunks(i));
        }
    }
    // Random Parameter (RFC 4895, 3.1.)
    uint64_t randomCount = initChunk->getRandomArraySize();
    if (randomCount > 0) {
        stream.writeUint16Be(RANDOM);
        stream.writeUint16Be(4 + randomCount);
        for (uint64_t i = 0; i < randomCount; ++i) {
            stream.writeByte(initChunk->getRandom(i));
        }
    }
    // Chunk List Parameter (RFC 4895, 3.2.)
    uint64_t chunkTypeCount = initChunk->getSctpChunkTypesArraySize();
    if (chunkTypeCount > 0) {
        stream.writeUint16Be(CHUNKS);
        stream.writeUint16Be(4 + chunkTypeCount);
        for (uint64_t i = 0; i < chunkTypeCount; ++i) {
            stream.writeByte(initChunk->getSctpChunkTypes(i));
        }
    }
    // Requested HMAC Algorithm Parameter (RFC 4895, 3.3.)
    uint64_t hmacCount = initChunk->getHmacTypesArraySize();
    if (hmacCount > 0) {
        stream.writeUint16Be(HMAC_ALGO);
        stream.writeUint16Be(4 + 2 * hmacCount);
        for (uint64_t i = 0; i < hmacCount; ++i) {
            stream.writeUint16Be(initChunk->getHmacTypes(i));
        }
    }
}

void deserializeInitChunk(MemoryInputStream& stream, SctpInitChunk *initChunk) {
    initChunk->setSctpChunkType(INIT);
    stream.readByte();
    initChunk->setByteLength(stream.readUint16Be());
    initChunk->setInitTag(stream.readUint32Be());
    initChunk->setA_rwnd(stream.readUint32Be());
    initChunk->setNoOutStreams(stream.readUint16Be());
    initChunk->setNoInStreams(stream.readUint16Be());
    initChunk->setInitTsn(stream.readUint32Be());
    uint64_t readBytes = 20;
    while (readBytes < uint64_t(initChunk->getByteLength())) {
        uint16_t chunkType = stream.readUint16Be();
        uint16_t length = stream.readUint16Be();
        readBytes += length;
        switch (chunkType) {
            case INIT_SUPPORTED_ADDRESS: {
                for (uint8_t i = 0; i < length - 4; i += 2) {
                    uint16_t tmp = stream.readUint16Be();
                    if (tmp == INIT_PARAM_IPV4)
                        initChunk->setIpv4Supported(true);
                    else if (tmp == INIT_PARAM_IPV6)
                        initChunk->setIpv6Supported(true);
                }
                break;
            }
            case FORWARD_TSN_SUPPORTED_PARAMETER: {
                initChunk->setSepChunksArraySize(length - 4);
                for (uint64_t i = 0; i < uint64_t(length - 4); ++i) {
                    initChunk->setSepChunks(i, stream.readByte());
                }
                break;
            }
            case INIT_PARAM_IPV4: {
                initChunk->setAddressesArraySize(initChunk->getAddressesArraySize() + 1);
                Ipv4Address addr = stream.readIpv4Address();
                initChunk->setAddresses(initChunk->getAddressesArraySize() - 1, addr);
                break;
            }
            case INIT_PARAM_IPV6: {
                initChunk->setAddressesArraySize(initChunk->getAddressesArraySize() + 1);
                initChunk->setAddresses(initChunk->getAddressesArraySize() - 1, stream.readIpv6Address());
                break;
            }
            case SUPPORTED_EXTENSIONS: {
                initChunk->setSepChunksArraySize(length - 4);
                for (uint16_t i = 0; i < uint16_t(length - 4); ++i) {
                    initChunk->setSepChunks(i, stream.readByte());
                }
                break;
            }
            case RANDOM: {
                initChunk->setRandomArraySize(length - 4);
                for (uint16_t i = 0; i < uint16_t(length - 4); ++i) {
                    initChunk->setRandom(i, stream.readByte());
                }
                break;
            }
            case CHUNKS: {
                initChunk->setSctpChunkTypesArraySize(length - 4);
                for (uint16_t i = 0; i < uint16_t(length - 4); ++i) {
                    initChunk->setSctpChunkTypes(i, stream.readByte());
                }
                break;
            }
            case HMAC_ALGO: {
                initChunk->setHmacTypesArraySize((length - 4) / 2);
                for (uint16_t i = 0; i < uint16_t((length - 4) / 2); ++i) {
                    initChunk->setHmacTypes(i, stream.readUint16Be());
                }
                break;
            }
            default: {
                break;
            }
        }
    }
}

void serializeInitAckChunk(MemoryOutputStream& stream, const SctpInitAckChunk* initAckChunk) {
    stream.writeByte(initAckChunk->getSctpChunkType());
    stream.writeByte(0);
    stream.writeUint16Be(initAckChunk->getByteLength());
    stream.writeUint32Be(initAckChunk->getInitTag());
    stream.writeUint32Be(initAckChunk->getA_rwnd());
    stream.writeUint16Be(initAckChunk->getNoOutStreams());
    stream.writeUint16Be(initAckChunk->getNoInStreams());
    stream.writeUint32Be(initAckChunk->getInitTsn());
    // Supported Address Types Parameter
    if (initAckChunk->getIpv4Supported() || initAckChunk->getIpv6Supported()) {
        stream.writeUint16Be(INIT_SUPPORTED_ADDRESS);
        stream.writeUint16Be(initAckChunk->getIpv4Supported() && initAckChunk->getIpv6Supported() ? 8 : 6);
        if (initAckChunk->getIpv4Supported() && initAckChunk->getIpv6Supported()) {
            stream.writeUint16Be(INIT_PARAM_IPV4);
            stream.writeUint16Be(INIT_PARAM_IPV6);
        }
        else if (initAckChunk->getIpv4Supported())
            stream.writeUint16Be(INIT_PARAM_IPV4);
        else
            stream.writeUint16Be(INIT_PARAM_IPV6);
    }
    // Forward-TSN-Supported Parameter
    if (initAckChunk->getForwardTsn() == true) {
        stream.writeUint16Be(FORWARD_TSN_SUPPORTED_PARAMETER);
        stream.writeUint16Be(4);
    }
    // IPv4 Address Parameter & IPv6 Address Parameter
    int32_t numaddr = initAckChunk->getAddressesArraySize();
    for (int32_t i = 0; i < numaddr; i++) {
        if (initAckChunk->getAddresses(i).getType() == L3Address::IPv4) {
            stream.writeUint16Be(INIT_PARAM_IPV4);
            stream.writeUint16Be(8);
            stream.writeIpv4Address(initAckChunk->getAddresses(i).toIpv4());
        }
        else if (initAckChunk->getAddresses(i).getType() == L3Address::IPv6) {
            stream.writeUint16Be(INIT_PARAM_IPV6);
            stream.writeUint16Be(20);
            stream.writeIpv6Address(initAckChunk->getAddresses(i).toIpv6());
        }
    }
    // Supported Extensions Parameter
    uint64_t chunkCount = initAckChunk->getSepChunksArraySize();
    if (chunkCount > 0) {
        stream.writeUint16Be(SUPPORTED_EXTENSIONS);
        stream.writeUint16Be(4 + chunkCount);
        for (uint64_t i = 0; i < chunkCount; ++i) {
            stream.writeByte(initAckChunk->getSepChunks(i));
        }
    }
    // Unrecognized Parameters
    uint64_t unrecognizedCount = initAckChunk->getUnrecognizedParametersArraySize();
    if (unrecognizedCount > 0) {
        stream.writeUint16Be(UNRECOGNIZED_PARAMETER);
        stream.writeUint16Be(4 + unrecognizedCount);
        for (uint64_t i = 0; i < unrecognizedCount; ++i) {
            stream.writeByte(initAckChunk->getUnrecognizedParameters(i));
        }
    }
    // Random Parameter
    uint64_t randomCount = initAckChunk->getRandomArraySize();
    if (randomCount > 0) {
        stream.writeUint16Be(RANDOM);
        stream.writeUint16Be(4 + randomCount);
        for (uint64_t i = 0; i < randomCount; ++i) {
            stream.writeByte(initAckChunk->getRandom(i));
        }
    }
    // Chunk List Parameter
    uint64_t chunkTypeCount = initAckChunk->getSctpChunkTypesArraySize();
    if (chunkTypeCount > 0) {
        stream.writeUint16Be(CHUNKS);
        stream.writeUint16Be(4 + chunkTypeCount);
        for (uint64_t i = 0; i < chunkTypeCount; ++i) {
            stream.writeByte(initAckChunk->getSctpChunkTypes(i));
        }
    }
    // Requested HMAC Algorithm Parameter
    uint64_t hmacCount = initAckChunk->getHmacTypesArraySize();
    if (hmacCount > 0) {
        stream.writeUint16Be(HMAC_ALGO);
        stream.writeUint16Be(4 + 2 * hmacCount);
        for (uint64_t i = 0; i < hmacCount; ++i) {
            stream.writeUint16Be(initAckChunk->getHmacTypes(i));
        }
    }
    // State Cookie Parameter: FIXME
    if (initAckChunk->getStateCookie() != nullptr) {
        stream.writeUint16Be(7);
        stream.writeUint16Be(initAckChunk->getStateCookie()->getLength());
        stream.writeByteRepeatedly('0', initAckChunk->getStateCookie()->getLength() - 4);
    }
}

void deserializeInitAckChunk(MemoryInputStream& stream, SctpInitAckChunk *initAckChunk) {
    initAckChunk->setSctpChunkType(INIT_ACK);
    stream.readByte();
    initAckChunk->setByteLength(stream.readUint16Be());
    initAckChunk->setInitTag(stream.readUint32Be());
    initAckChunk->setA_rwnd(stream.readUint32Be());
    initAckChunk->setNoOutStreams(stream.readUint16Be());
    initAckChunk->setNoInStreams(stream.readUint16Be());
    initAckChunk->setInitTsn(stream.readUint32Be());
    uint64_t readBytes = 20;
    while (readBytes < uint64_t(initAckChunk->getByteLength())) {
        uint16_t chunkType = stream.readUint16Be();
        uint16_t length = stream.readUint16Be();
        readBytes += length;
        switch (chunkType) {
            case INIT_SUPPORTED_ADDRESS: {
                for (uint8_t i = 0; i < length - 4; i += 2) {
                    uint16_t tmp = stream.readUint16Be();
                    if (tmp == INIT_PARAM_IPV4)
                        initAckChunk->setIpv4Supported(true);
                    else if (tmp == INIT_PARAM_IPV6)
                        initAckChunk->setIpv6Supported(true);
                }
                break;
            }
            case FORWARD_TSN_SUPPORTED_PARAMETER: {
                initAckChunk->setSepChunksArraySize(length - 4);
                for (uint64_t i = 0; i < uint64_t(length - 4); ++i) {
                    initAckChunk->setSepChunks(i, stream.readByte());
                }
                break;
            }
            case INIT_PARAM_IPV4: {
                initAckChunk->setAddressesArraySize(initAckChunk->getAddressesArraySize() + 1);
                initAckChunk->setAddresses(initAckChunk->getAddressesArraySize() - 1, stream.readIpv4Address());
                break;
            }
            case INIT_PARAM_IPV6: {
                initAckChunk->setAddressesArraySize(initAckChunk->getAddressesArraySize() + 1);
                initAckChunk->setAddresses(initAckChunk->getAddressesArraySize() - 1, stream.readIpv6Address());
                break;
            }
            case UNRECOGNIZED_PARAMETER: {
                initAckChunk->setUnrecognizedParametersArraySize(length - 4);
                for (uint64_t i = 0; i < uint64_t(length - 4); ++i) {
                    initAckChunk->setUnrecognizedParameters(i, stream.readByte());
                }
                break;
            }
            case SUPPORTED_EXTENSIONS: {
                initAckChunk->setSepChunksArraySize(length - 4);
                for (uint64_t i = 0; i < uint64_t(length - 4); ++i) {
                    initAckChunk->setSepChunks(i, stream.readByte());
                }
                break;
            }
            case RANDOM: {
                initAckChunk->setRandomArraySize(length - 4);
                for (uint64_t i = 0; i < uint64_t(length - 4); ++i) {
                    initAckChunk->setRandom(i, stream.readByte());
                }
                break;
            }
            case CHUNKS: {
                initAckChunk->setSctpChunkTypesArraySize(length - 4);
                for (uint64_t i = 0; i < uint64_t(length - 4); ++i) {
                    initAckChunk->setSctpChunkTypes(i, stream.readByte());
                }
                break;
            }
            case HMAC_ALGO: {
                initAckChunk->setHmacTypesArraySize((length - 4) / 2);
                for (uint64_t i = 0; i < uint64_t((length - 4) / 2); ++i) {
                    initAckChunk->setHmacTypes(i, stream.readUint16Be());
                }
                break;
            }
            // State Cookie Parameter: FIXME
            case 7: {
                SctpCookie *stateCookie = new SctpCookie();
                stateCookie->setLength(length);
                stream.readByteRepeatedly(0, length - 4);
                initAckChunk->setStateCookie(stateCookie);
                break;
            }
            default: {
                break;
            }
        }
    }
}

void serializeSackChunk(MemoryOutputStream& stream, const SctpSackChunk* sackChunk) {
    stream.writeByte(sackChunk->getSctpChunkType());
    stream.writeByte(0);
    stream.writeUint16Be(sackChunk->getByteLength());
    uint32_t cumtsnack = sackChunk->getCumTsnAck();
    stream.writeUint32Be(cumtsnack);
    stream.writeUint32Be(sackChunk->getA_rwnd());
    uint16_t numgaps = sackChunk->getNumGaps();
    stream.writeUint16Be(numgaps);
    uint16_t numdups = sackChunk->getNumDupTsns();
    stream.writeUint16Be(numdups);
    for (uint16_t i = 0; i < numgaps; ++i) {
        stream.writeUint16Be(sackChunk->getGapStart(i) - cumtsnack);
        stream.writeUint16Be(sackChunk->getGapStop(i) - cumtsnack);
    }
    for (uint16_t i = 0; i < numdups; ++i) {
        stream.writeUint32Be(sackChunk->getDupTsns(i));
    }
}

void deserializeSackChunk(MemoryInputStream& stream, SctpSackChunk *sackChunk) {
    sackChunk->setIsNrSack(false);
    sackChunk->setSctpChunkType(SACK);
    stream.readByte();
    sackChunk->setByteLength(stream.readUint16Be());
    uint32_t cumtsnack = stream.readUint32Be();
    sackChunk->setCumTsnAck(cumtsnack);
    sackChunk->setA_rwnd(stream.readUint32Be());
    uint16_t numgaps = stream.readUint16Be();
    sackChunk->setNumGaps(numgaps);
    uint16_t numdups = stream.readUint16Be();
    sackChunk->setNumDupTsns(numdups);
    sackChunk->setGapStartArraySize(numgaps);
    sackChunk->setGapStopArraySize(numgaps);
    for (uint16_t i = 0; i < numgaps; ++i) {
        sackChunk->setGapStart(i, stream.readUint16Be() + cumtsnack);
        sackChunk->setGapStop(i, stream.readUint16Be() + cumtsnack);
    }
    sackChunk->setDupTsnsArraySize(numdups);
    for (uint16_t i = 0; i < numdups; ++i) {
        sackChunk->setDupTsns(i, stream.readUint32Be());
    }
}
/*
void serializeNrSackChunk(MemoryOutputStream& stream, const SctpSackChunk* sackChunk) {
    stream.writeByte(sackChunk->getSctpChunkType());
    stream.writeByte(0);
    stream.writeUint16Be(sackChunk->getByteLength());
    uint32_t cumtsnack = sackChunk->getCumTsnAck();
    stream.writeUint32Be(cumtsnack);
    stream.writeUint32Be(sackChunk->getA_rwnd());
    uint16_t numgaps = sackChunk->getNumGaps();
    stream.writeUint16Be(numgaps);
    uint16_t numnrgaps = sackChunk->getNumNrGaps();
    stream.writeUint16Be(numnrgaps);
    uint16_t numdups = sackChunk->getNumDupTsns();
    stream.writeUint16Be(numdups);
    stream.writeUint16Be(0);
    for (uint16_t i = 0; i < numgaps; ++i) {
        stream.writeUint16Be(sackChunk->getGapStart(i) - cumtsnack);
        stream.writeUint16Be(sackChunk->getGapStop(i) - cumtsnack);
    }
    for (uint16_t i = 0; i < numnrgaps; ++i) {
        stream.writeUint16Be(sackChunk->getNrGapStart(i) - cumtsnack);
        stream.writeUint16Be(sackChunk->getNrGapStop(i) - cumtsnack);
    }
    for (uint16_t i = 0; i < numdups; ++i) {
        stream.writeUint32Be(sackChunk->getDupTsns(i));
    }
}

void deserializeNrSackChunk(MemoryInputStream& stream, SctpSackChunk *sackChunk) {
    sackChunk->setIsNrSack(true);
    sackChunk->setSctpChunkType(NR_SACK);
    stream.readByte();
    sackChunk->setByteLength(stream.readUint16Be());
    uint32_t cumtsnack = stream.readUint32Be();
    sackChunk->setCumTsnAck(cumtsnack);
    sackChunk->setA_rwnd(stream.readUint32Be());
    uint16_t numgaps = stream.readUint16Be();
    sackChunk->setNumGaps(numgaps);
    uint16_t numnrgaps = stream.readUint16Be();
    sackChunk->setNumNrGaps(numnrgaps);
    uint16_t numdups = stream.readUint16Be();
    sackChunk->setNumDupTsns(numdups);
    sackChunk->setGapStartArraySize(numgaps);
    sackChunk->setGapStopArraySize(numgaps);
    for (uint16_t i = 0; i < numgaps; ++i) {
        sackChunk->setGapStart(i, stream.readUint16Be() + cumtsnack);
        sackChunk->setGapStop(i, stream.readUint16Be() + cumtsnack);
    }
    sackChunk->setNrGapStartArraySize(numnrgaps);
    sackChunk->setNrGapStopArraySize(numnrgaps);
    for (uint16_t i = 0; i < numnrgaps; ++i) {
        sackChunk->setNrGapStart(i, stream.readUint16Be() + cumtsnack);
        sackChunk->setNrGapStop(i, stream.readUint16Be() + cumtsnack);
    }
    sackChunk->setDupTsnsArraySize(numdups);
    for (uint16_t i = 0; i < numdups; ++i) {
        sackChunk->setDupTsns(i, stream.readUint32Be());
    }
}
*/
void serializeHeartbeatChunk(MemoryOutputStream& stream, const SctpHeartbeatChunk* heartbeatChunk) {
    stream.writeByte(heartbeatChunk->getSctpChunkType());
    stream.writeByte(0);
    stream.writeUint16Be(heartbeatChunk->getByteLength());
    L3Address addr = heartbeatChunk->getRemoteAddr();
    if (addr.getType() == L3Address::IPv4) {
        stream.writeUint16Be(1);    // Heartbeat Info Type=1
        stream.writeUint16Be(12 + 9);   // HB Info Length (+ 9 because of simtime)
        stream.writeUint16Be(INIT_PARAM_IPV4);
        stream.writeUint16Be(8);
        stream.writeIpv4Address(addr.toIpv4());
    }
    else if (addr.getType() == L3Address::IPv6) {
        stream.writeUint16Be(1);    // Heartbeat Info Type=1
        stream.writeUint16Be(24 + 9);   // HB Info Length (+ 9 because of simtime)
        stream.writeUint16Be(INIT_PARAM_IPV6);
        stream.writeUint16Be(20 + 9);
        stream.writeIpv6Address(addr.toIpv6());
    }
    stream.writeSimTime(heartbeatChunk->getTimeField());
}

void deserializeHeartbeatChunk(MemoryInputStream& stream, SctpHeartbeatChunk *heartbeatChunk) {
    heartbeatChunk->setSctpChunkType(HEARTBEAT);
    stream.readByte();
    heartbeatChunk->setByteLength(stream.readUint16Be());
    stream.readUint16Be();
    uint16_t infolen = stream.readUint16Be();
    uint16_t paramType = stream.readUint16Be();
    stream.readUint16Be();
    switch (paramType) {
        case INIT_PARAM_IPV4: {
            heartbeatChunk->setRemoteAddr(stream.readIpv4Address());
            break;
        }
        case INIT_PARAM_IPV6: {
            heartbeatChunk->setRemoteAddr(stream.readIpv6Address());
            break;
        }
        default:
            stream.readByteRepeatedly(0, infolen - 4);
    }
    heartbeatChunk->setTimeField(stream.readSimTime());
}

void serializeHeartbeatAckChunk(MemoryOutputStream& stream, const SctpHeartbeatAckChunk* heartbeatAckChunk) {
    stream.writeByte(heartbeatAckChunk->getSctpChunkType());
    stream.writeByte(0);
    stream.writeUint16Be(heartbeatAckChunk->getByteLength());
    uint32_t infolen = heartbeatAckChunk->getInfoArraySize();
    stream.writeUint16Be(1);
    if (infolen > 0) {
        stream.writeUint16Be(infolen + 4);
        for (uint32_t i = 0; i < infolen; ++i) {
            stream.writeByte(heartbeatAckChunk->getInfo(i));
        }
    }
    else {
        L3Address addr = heartbeatAckChunk->getRemoteAddr();
        if (addr.getType() == L3Address::IPv4) {
            stream.writeUint16Be(23);
            stream.writeUint16Be(1);
            uint32_t infolen = sizeof(addr.toIpv4().getInt()) + sizeof(uint32_t);
            stream.writeUint16Be(infolen + 4);
            stream.writeUint16Be(INIT_PARAM_IPV4);
            stream.writeUint16Be(8);
            stream.writeIpv4Address(addr.toIpv4());
        }
        else if (addr.getType() == L3Address::IPv6) {
            stream.writeUint16Be(35);
            stream.writeUint16Be(1);
            uint32_t infolen = 20 + sizeof(uint32_t);
            stream.writeUint16Be(infolen + 4);
            stream.writeUint16Be(INIT_PARAM_IPV6);
            stream.writeUint16Be(20);
            stream.writeIpv6Address(addr.toIpv6());
        }
    }
    stream.writeSimTime(heartbeatAckChunk->getTimeField());
}

void deserializeHeartbeatAckChunk(MemoryInputStream& stream, SctpHeartbeatAckChunk *heartbeatAckChunk) {
    heartbeatAckChunk->setSctpChunkType(HEARTBEAT_ACK);
    stream.readByte();
    heartbeatAckChunk->setByteLength(stream.readUint16Be());
    stream.readUint16Be();
    uint16_t infolen = stream.readUint16Be();
    if (infolen == 23 || infolen == 35) {
        stream.readUint16Be();
        infolen = stream.readUint16Be();
        switch (stream.readUint16Be()) {
            case INIT_PARAM_IPV4: {
                stream.readUint16Be();
                heartbeatAckChunk->setRemoteAddr(stream.readIpv4Address());
                break;
            }
            case INIT_PARAM_IPV6: {
                stream.readUint16Be();
                heartbeatAckChunk->setRemoteAddr(stream.readIpv6Address());
                break;
            }
            default:
                stream.readByteRepeatedly(0, infolen - 4);
        }
    }
    else {
        ASSERT(infolen - 4 >= 0);
        heartbeatAckChunk->setInfoArraySize(infolen - 4);
        for (uint16_t i = 0; i < infolen - 4; ++i) {
            heartbeatAckChunk->setInfo(i, stream.readByte());
        }
    }
    heartbeatAckChunk->setTimeField(stream.readSimTime());
}

void serializeAbortChunk(MemoryOutputStream& stream, const SctpAbortChunk* abortChunk) {
    stream.writeByte(abortChunk->getSctpChunkType());
    stream.writeNBitsOfUint64Be(0, 7);
    stream.writeBit(abortChunk->getT_Bit());
    stream.writeUint16Be(abortChunk->getByteLength());
}

void deserializeAbortChunk(MemoryInputStream& stream, SctpAbortChunk *abortChunk) {
    abortChunk->setSctpChunkType(ABORT);
    stream.readNBitsToUint64Be(7);
    abortChunk->setT_Bit(stream.readBit());
    abortChunk->setByteLength(stream.readUint16Be());
}

void serializeShutdownChunk(MemoryOutputStream& stream, const SctpShutdownChunk* shutdownChunk) {
    stream.writeByte(shutdownChunk->getSctpChunkType());
    stream.writeByte(0);
    stream.writeUint16Be(shutdownChunk->getByteLength());   // must be 8
    stream.writeUint32Be(shutdownChunk->getCumTsnAck());
}

void deserializeShutdownChunk(MemoryInputStream& stream, SctpShutdownChunk *shutdownChunk) {
    shutdownChunk->setSctpChunkType(SHUTDOWN);
    stream.readByte();
    shutdownChunk->setByteLength(stream.readUint16Be());
    shutdownChunk->setCumTsnAck(stream.readUint32Be());
}

void serializeShutdownAckChunk(MemoryOutputStream& stream, const SctpShutdownAckChunk* shutdownAckChunk) {
    stream.writeByte(shutdownAckChunk->getSctpChunkType());
    stream.writeByte(0);
    stream.writeUint16Be(shutdownAckChunk->getByteLength());   // must be 4
}

void deserializeShutdownAckChunk(MemoryInputStream& stream, SctpShutdownAckChunk *shutdownAckChunk) {
    shutdownAckChunk->setSctpChunkType(SHUTDOWN_ACK);
    stream.readByte();
    shutdownAckChunk->setByteLength(stream.readUint16Be());
}

void serializeCookieEchoChunk(MemoryOutputStream& stream, const SctpCookieEchoChunk* cookieChunk) {
    stream.writeByte(cookieChunk->getSctpChunkType());
    stream.writeByte(0);
    uint16_t length = cookieChunk->getByteLength();
    stream.writeUint16Be(length);
    stream.writeByteRepeatedly(0, length - 4);
}

void deserializeCookieEchoChunk(MemoryInputStream& stream, SctpCookieEchoChunk *cookieChunk) {
    cookieChunk->setSctpChunkType(COOKIE_ECHO);
    stream.readByte();
    uint16_t length = stream.readUint16Be();
    cookieChunk->setByteLength(length);
    SctpCookie *stateCookie = new SctpCookie();
    stateCookie->setLength(length - 4);
    stream.readByteRepeatedly(0, length - 4);
    cookieChunk->setStateCookie(stateCookie);
}

void serializeCookieAckChunk(MemoryOutputStream& stream, const SctpCookieAckChunk* cookieAckChunk) {
    stream.writeByte(cookieAckChunk->getSctpChunkType());
    stream.writeByte(0);
    ASSERT(cookieAckChunk->getByteLength() == 4);
    stream.writeUint16Be(cookieAckChunk->getByteLength());
}

void deserializeCookieAckChunk(MemoryInputStream& stream, SctpCookieAckChunk *cookieAckChunk) {
    cookieAckChunk->setSctpChunkType(COOKIE_ACK);
    stream.readByte();
    uint16_t length = stream.readUint16Be();
    cookieAckChunk->setByteLength(length);  // must be 4
}

void serializeShutdownCompleteChunk(MemoryOutputStream& stream, const SctpShutdownCompleteChunk* shutdownCompleteChunk) {
    stream.writeByte(shutdownCompleteChunk->getSctpChunkType());
    stream.writeNBitsOfUint64Be(0, 7);
    stream.writeBit(shutdownCompleteChunk->getTBit());
    stream.writeUint16Be(shutdownCompleteChunk->getByteLength());  // must be 4
}

void deserializeShutdownCompleteChunk(MemoryInputStream& stream, SctpShutdownCompleteChunk *shutdownCompleteChunk) {
    shutdownCompleteChunk->setSctpChunkType(SHUTDOWN_COMPLETE);
    stream.readNBitsToUint64Be(7);
    shutdownCompleteChunk->setTBit(stream.readBit());
    shutdownCompleteChunk->setByteLength(stream.readUint16Be());  // must be 4
}
/*
void serializeAuthenticationChunk(MemoryOutputStream& stream, const SctpAuthenticationChunk* authChunk) {
    stream.writeByte(authChunk->getSctpChunkType());
    stream.writeByte(0);
    stream.writeUint16Be(SCTP_AUTH_CHUNK_LENGTH + SHA_LENGTH);
    stream.writeUint16Be(authChunk->getSharedKey());
    stream.writeUint16Be(authChunk->getHMacIdentifier());
    for (uint8_t i = 0; i < SHA_LENGTH; ++i) {
        stream.writeByte(0);
    }
}

void deserializeAuthenticationChunk(MemoryInputStream& stream, SctpAuthenticationChunk *authChunk) {
    authChunk->setSctpChunkType(AUTH);
    stream.readByte();
    uint16_t len = stream.readUint16Be();
    authChunk->setByteLength(len);
    authChunk->setSharedKey(stream.readUint16Be());
    authChunk->setHMacIdentifier(stream.readUint16Be());
    for (uint8_t i = 0; i < len - SCTP_AUTH_CHUNK_LENGTH; ++i) {
        stream.readByte();
    }
}

void serializeForwardTsnChunk(MemoryOutputStream& stream, const SctpForwardTsnChunk* forward) {
    stream.writeByte(forward->getSctpChunkType());
    stream.writeByte(0);
    stream.writeUint16Be(forward->getByteLength());
    stream.writeUint32Be(forward->getNewCumTsn());
    ASSERT(forward->getSidArraySize() == forward->getSsnArraySize());
    for (uint32_t i = 0; i < forward->getSidArraySize(); ++i) {
        stream.writeUint16Be(forward->getSid(i));
        stream.writeUint16Be(forward->getSsn(i));
    }
}

void deserializeForwardTsnChunk(MemoryInputStream& stream, SctpForwardTsnChunk *forward) {
    forward->setSctpChunkType(FORWARD_TSN);
    stream.readByte();
    forward->setByteLength(stream.readUint16Be());
    forward->setNewCumTsn(stream.readUint32Be());
    uint32_t num = (forward->getByteLength() - 8) / 4;
    forward->setSidArraySize(num);
    forward->setSsnArraySize(num);
    for (uint32_t i = 0; i < num; ++i) {
        forward->setSid(i, stream.readUint16Be());
        forward->setSsn(i, stream.readUint16Be());
    }
}

void serializeAsconfChangeChunk(MemoryOutputStream& stream, const SctpAsconfChunk* asconfChunk) {
    stream.writeByte(asconfChunk->getSctpChunkType());
    stream.writeByte(0);
    stream.writeUint16Be(asconfChunk->getByteLength());
    stream.writeUint32Be(asconfChunk->getSerialNumber());

    stream.writeByte(INIT_PARAM_IPV4);
    stream.writeByte(0);
    stream.writeUint16Be(8);
    stream.writeIpv4Address(asconfChunk->getAddressParam().toIpv4());

    for (uint32_t i = 0; i < asconfChunk->getAsconfParamsArraySize(); ++i) {
        SctpParameter *parameter = (SctpParameter *)(asconfChunk->getAsconfParams(i));
        switch (parameter->getParameterType()) {
            case ADD_IP_ADDRESS: {
                SctpAddIPParameter *addip = check_and_cast<SctpAddIPParameter *>(parameter);
                stream.writeUint16Be(ADD_IP_ADDRESS);
                stream.writeUint16Be(addip->getByteLength());
                stream.writeUint32Be(addip->getRequestCorrelationId());
                stream.writeByte(INIT_PARAM_IPV4);
                stream.writeByte(8);
                stream.writeIpv4Address(addip->getAddressParam().toIpv4());
                break;
            }
            case DELETE_IP_ADDRESS: {
                SctpDeleteIPParameter *deleteip = check_and_cast<SctpDeleteIPParameter *>(parameter);
                stream.writeUint16Be(DELETE_IP_ADDRESS);
                stream.writeUint16Be(deleteip->getByteLength());
                stream.writeUint32Be(deleteip->getRequestCorrelationId());
                stream.writeByte(INIT_PARAM_IPV4);
                stream.writeByte(8);
                stream.writeIpv4Address(deleteip->getAddressParam().toIpv4());
                break;
            }
            case SET_PRIMARY_ADDRESS: {
                SctpSetPrimaryIPParameter *setip = check_and_cast<SctpSetPrimaryIPParameter *>(parameter);
                stream.writeUint16Be(SET_PRIMARY_ADDRESS);
                stream.writeUint16Be(setip->getByteLength());
                stream.writeUint32Be(setip->getRequestCorrelationId());
                stream.writeByte(INIT_PARAM_IPV4);
                stream.writeByte(8);
                stream.writeIpv4Address(setip->getAddressParam().toIpv4());
                break;
            }
            default:
                throw cRuntimeError("Parameter Type %d not supported", parameter->getParameterType());
        }
    }
}

void deserializeAsconfChangeChunk(MemoryInputStream& stream, SctpAsconfChunk *asconfChunk) {
    asconfChunk->setSctpChunkType(ASCONF);
    stream.readByte();
    asconfChunk->setByteLength(stream.readUint16Be());
    asconfChunk->setSerialNumber(stream.readUint32Be());

    stream.readByte();
    stream.readByte();
    stream.readUint16Be();
    asconfChunk->setAddressParam(stream.readIpv4Address());

    uint8_t arrsize = (asconfChunk->getByteLength() - 16) / 12;
    asconfChunk->setAsconfParamsArraySize(arrsize);
    for (uint32_t i = 0; i < arrsize; ++i) {
        uint16_t type = stream.readUint16Be();
        switch (type) {
            case ADD_IP_ADDRESS: {
                SctpAddIPParameter *addip = new SctpAddIPParameter();
                stream.readUint16Be();
                addip->setRequestCorrelationId(stream.readUint32Be());
                stream.readByte();
                stream.readByte();
                addip->setAddressParam(stream.readIpv4Address());
                asconfChunk->setAsconfParams(i, addip);
                break;
            }
            case DELETE_IP_ADDRESS: {
                SctpDeleteIPParameter *deleteip = new SctpDeleteIPParameter();
                stream.readUint16Be();
                deleteip->setRequestCorrelationId(stream.readUint32Be());
                stream.readByte();
                stream.readByte();
                deleteip->setAddressParam(stream.readIpv4Address());
                asconfChunk->setAsconfParams(i, deleteip);
                break;
            }
            case SET_PRIMARY_ADDRESS: {
                SctpSetPrimaryIPParameter *setip = new SctpSetPrimaryIPParameter();
                stream.readUint16Be();
                setip->setRequestCorrelationId(stream.readUint32Be());
                stream.readByte();
                stream.readByte();
                setip->setAddressParam(stream.readIpv4Address());
                asconfChunk->setAsconfParams(i, setip);
                break;
            }
            default:
                throw cRuntimeError("Parameter Type %d not supported", type);
        }
    }
}

void serializeAsconfAckChunk(MemoryOutputStream& stream, const SctpAsconfAckChunk* asconfAckChunk) {
    stream.writeByte(asconfAckChunk->getSctpChunkType());
    stream.writeByte(0);
    stream.writeUint16Be(asconfAckChunk->getByteLength());
    stream.writeUint32Be(asconfAckChunk->getSerialNumber());

    for (uint32_t i = 0; i < asconfAckChunk->getAsconfResponseArraySize(); ++i) {
        SctpParameter *parameter = check_and_cast<SctpParameter *>(asconfAckChunk->getAsconfResponse(i));
        switch (parameter->getParameterType()) {
            case ERROR_CAUSE_INDICATION: {
                SctpErrorCauseParameter *error = check_and_cast<SctpErrorCauseParameter *>(parameter);
                stream.writeByte(error->getParameterType());
                stream.writeByte(error->getByteLength());
                stream.writeUint32Be(error->getResponseCorrelationId());

                if (check_and_cast<SctpParameter *>(error->getEncapsulatedPacket()) != nullptr) {
                    SctpParameter *encParameter = check_and_cast<SctpParameter *>(error->getEncapsulatedPacket());
                    switch (encParameter->getParameterType()) {
                        case ADD_IP_ADDRESS: {
                            SctpAddIPParameter *addip = check_and_cast<SctpAddIPParameter *>(encParameter);
                            stream.writeByte(ADD_IP_ADDRESS);
                            stream.writeByte(addip->getByteLength());
                            stream.writeUint32Be(addip->getRequestCorrelationId());
                            stream.writeByte(INIT_PARAM_IPV4);
                            stream.writeByte(8);
                            stream.writeIpv4Address(addip->getAddressParam().toIpv4());
                            break;
                        }
                        case DELETE_IP_ADDRESS: {
                            SctpDeleteIPParameter *deleteip = check_and_cast<SctpDeleteIPParameter *>(encParameter);
                            stream.writeByte(DELETE_IP_ADDRESS);
                            stream.writeByte(deleteip->getByteLength());
                            stream.writeUint32Be(deleteip->getRequestCorrelationId());
                            stream.writeByte(INIT_PARAM_IPV4);
                            stream.writeByte(8);
                            stream.writeIpv4Address(deleteip->getAddressParam().toIpv4());
                            break;
                        }
                        case SET_PRIMARY_ADDRESS: {
                            SctpSetPrimaryIPParameter *setip = check_and_cast<SctpSetPrimaryIPParameter *>(encParameter);
                            stream.writeByte(SET_PRIMARY_ADDRESS);
                            stream.writeByte(setip->getByteLength());
                            stream.writeUint32Be(setip->getRequestCorrelationId());
                            stream.writeByte(INIT_PARAM_IPV4);
                            stream.writeByte(8);
                            stream.writeIpv4Address(setip->getAddressParam().toIpv4());
                            break;
                        }
                        throw cRuntimeError("Parameter Type %d not supported", encParameter->getParameterType());
                    }
                }
                break;
            }
            case SUCCESS_INDICATION: {
                SctpSuccessIndication *success = check_and_cast<SctpSuccessIndication *>(parameter);
                stream.writeByte(success->getParameterType());
                stream.writeByte(8);
                stream.writeUint32Be(success->getResponseCorrelationId());
                break;
            }
            default:
                throw cRuntimeError("Parameter Type %d not supported", parameter->getParameterType());
        }
    }
}

void deserializeAsconfAckChunk(MemoryInputStream& stream, SctpAsconfAckChunk *asconfAckChunk) {
    asconfAckChunk->setSctpChunkType(ASCONF_ACK);
    stream.readByte();
    asconfAckChunk->setByteLength(stream.readUint16Be());
    asconfAckChunk->setSerialNumber(stream.readUint32Be());

    uint32_t bytes_to_read = asconfAckChunk->getByteLength() - 8;
    while (bytes_to_read > 0) {
        uint8_t type = stream.readByte();
        switch (type) {
            case ERROR_CAUSE_INDICATION: {
                SctpErrorCauseParameter *error = new SctpErrorCauseParameter("ERROR_CAUSE");
                error->setParameterType(stream.readByte());
                error->setByteLength(stream.readByte());
                error->setResponseCorrelationId(stream.readUint32Be());
                uint8_t paramType = stream.readByte();
                //chunk->encapsulate(smsg);
                switch (paramType) {
                    case ADD_IP_ADDRESS: {
                        SctpAddIPParameter *addip = new SctpAddIPParameter();
                        stream.readByte();
                        addip->setByteLength(stream.readByte());
                        addip->setRequestCorrelationId(stream.readUint32Be());
                        stream.readByte();
                        stream.readByte();
                        addip->setAddressParam(stream.readIpv4Address());
                        error->encapsulate(addip);
                        break;
                    }
                    case DELETE_IP_ADDRESS: {
                        SctpDeleteIPParameter *deleteip = new SctpDeleteIPParameter();
                        stream.readByte();
                        deleteip->setByteLength(stream.readByte());
                        deleteip->setRequestCorrelationId(stream.readUint32Be());
                        stream.readByte();
                        stream.readByte();
                        deleteip->setAddressParam(stream.readIpv4Address());
                        error->encapsulate(deleteip);
                        break;
                    }
                    case SET_PRIMARY_ADDRESS: {
                        SctpSetPrimaryIPParameter *setip = new SctpSetPrimaryIPParameter();
                        stream.readByte();
                        setip->setByteLength(stream.readByte());
                        setip->setRequestCorrelationId(stream.readUint32Be());
                        stream.readByte();
                        stream.readByte();
                        setip->setAddressParam(stream.readIpv4Address());
                        error->encapsulate(setip);
                        break;
                    }
                }
                asconfAckChunk->addAsconfResponse(error);
                break;
            }
            case SUCCESS_INDICATION: {
                SctpSuccessIndication *success = new SctpSuccessIndication();
                success->setParameterType(stream.readByte());
                stream.readByte();
                success->setResponseCorrelationId(stream.readUint32Be());
                break;
            }
            default: {
                stream.readByteRepeatedly(0, bytes_to_read);
                break;
            }
        }
    }
}

void serializeErrorChunk(MemoryOutputStream& stream, const SctpErrorChunk* errorchunk) {
    stream.writeByte(errorchunk->getSctpChunkType());
    stream.writeNBitsOfUint64Be(0, 6);
    stream.writeBit(errorchunk->getMBit());
    stream.writeBit(errorchunk->getTBit());
    stream.writeUint16Be(errorchunk->getByteLength());
    if (errorchunk->getParametersArraySize() > 0) {
        SctpParameter *parameter = check_and_cast<SctpParameter *>(errorchunk->getParameters(0));
        switch (parameter->getParameterType()) {
            case MISSING_NAT_ENTRY: {
                SctpSimpleErrorCauseParameter *ecp = check_and_cast<SctpSimpleErrorCauseParameter *>(parameter);
                stream.writeUint16Be(ecp->getParameterType());
                stream.writeUint16Be(ecp->getByteLength());
                stream.writeByteRepeatedly(ecp->getValue(), ecp->getByteLength() - 4);
                break;
            }
            case INVALID_STREAM_IDENTIFIER: {
                SctpSimpleErrorCauseParameter *ecp = check_and_cast<SctpSimpleErrorCauseParameter *>(parameter);
                stream.writeUint16Be(ecp->getParameterType());
                stream.writeUint16Be(ecp->getByteLength());
                stream.writeUint16Be(ecp->getValue());
                stream.writeUint16Be(0);
                break;
            }
            default:
                throw cRuntimeError("Parameter Type %d not supported", parameter->getParameterType());
        }
    }
}

void deserializeErrorChunk(MemoryInputStream& stream, SctpErrorChunk *errorchunk) {
    errorchunk->setSctpChunkType(ERRORTYPE);
    stream.readNBitsToUint64Be(6);
    errorchunk->setMBit(stream.readBit());
    errorchunk->setTBit(stream.readBit());
    errorchunk->setByteLength(stream.readUint16Be());
    if (errorchunk->getByteLength() > 4) {
        errorchunk->setParametersArraySize(1);
        uint8_t type = stream.readByte();
        switch (type) {
            case MISSING_NAT_ENTRY: {
                SctpSimpleErrorCauseParameter *ecp = new SctpSimpleErrorCauseParameter();
                ecp->setParameterType(MISSING_NAT_ENTRY);
                ecp->setByteLength(stream.readUint16Be());
                ecp->setValue(stream.readByteRepeatedly(0, ecp->getByteLength() - 4));
                break;
            }
            case INVALID_STREAM_IDENTIFIER: {
                SctpSimpleErrorCauseParameter *ecp = new SctpSimpleErrorCauseParameter();
                ecp->setParameterType(INVALID_STREAM_IDENTIFIER);
                ecp->setByteLength(stream.readUint16Be());
                ecp->setValue(stream.readUint16Be());
                stream.readUint16Be();
                break;
            }
            default:
                break;
        }
    }
}

void serializeReConfigurationChunk(MemoryOutputStream& stream, const SctpStreamResetChunk* streamReset) {
    stream.writeByte(streamReset->getSctpChunkType());
    stream.writeByte(0);
    stream.writeUint16Be(streamReset->getByteLength());
    uint16_t numParameters = streamReset->getParametersArraySize();
    for (uint16_t i = 0; i < numParameters; ++i) {
        SctpParameter *parameter = (SctpParameter *)(streamReset->getParameters(i));
        switch (parameter->getParameterType()) {
            case OUTGOING_RESET_REQUEST_PARAMETER: {
                SctpOutgoingSsnResetRequestParameter *outparam = check_and_cast<SctpOutgoingSsnResetRequestParameter *>(parameter);
                stream.writeUint16Be(outparam->getParameterType());
                stream.writeUint16Be(16 + 2 * outparam->getStreamNumbersArraySize());
                stream.writeUint32Be(outparam->getSrReqSn());
                stream.writeUint32Be(outparam->getSrResSn());
                stream.writeUint32Be(outparam->getLastTsn());
                for (uint32_t i = 0; i < outparam->getStreamNumbersArraySize(); ++i) {
                    stream.writeUint16Be(outparam->getStreamNumbers(i));
                }
                break;
            }
            case INCOMING_RESET_REQUEST_PARAMETER: {
                SctpIncomingSsnResetRequestParameter *inparam = check_and_cast<SctpIncomingSsnResetRequestParameter *>(parameter);
                stream.writeUint16Be(inparam->getParameterType());
                stream.writeUint16Be(8 + 2 * inparam->getStreamNumbersArraySize());
                stream.writeUint32Be(inparam->getSrReqSn());
                for (uint32_t i = 0; i < inparam->getStreamNumbersArraySize(); ++i) {
                    stream.writeUint16Be(inparam->getStreamNumbers(i));
                }
                break;
            }
            case SSN_TSN_RESET_REQUEST_PARAMETER: {
                SctpSsnTsnResetRequestParameter *ssnparam = check_and_cast<SctpSsnTsnResetRequestParameter *>(parameter);
                stream.writeUint16Be(ssnparam->getParameterType());
                stream.writeUint16Be(8);
                stream.writeUint32Be(ssnparam->getSrReqSn());
                break;
            }
            case STREAM_RESET_RESPONSE_PARAMETER: {
                SctpStreamResetResponseParameter *response = check_and_cast<SctpStreamResetResponseParameter *>(parameter);
                stream.writeUint16Be(response->getParameterType());
                if (response->getSendersNextTsn() != 0)
                    stream.writeUint16Be(20);
                else
                    stream.writeUint16Be(12);
                stream.writeUint32Be(response->getSrResSn());
                stream.writeUint32Be(response->getResult());
                if (response->getSendersNextTsn() != 0) {
                    stream.writeUint32Be(response->getSendersNextTsn());
                    stream.writeUint32Be(response->getReceiversNextTsn());
                }
                break;
            }
            case ADD_OUTGOING_STREAMS_REQUEST_PARAMETER: {
                SctpAddStreamsRequestParameter *outstreams = check_and_cast<SctpAddStreamsRequestParameter *>(parameter);
                stream.writeUint16Be(outstreams->getParameterType());
                stream.writeUint16Be(12);
                stream.writeUint32Be(outstreams->getSrReqSn());
                stream.writeUint16Be(outstreams->getNumberOfStreams());
                stream.writeUint16Be(0);
                break;
            }
            case ADD_INCOMING_STREAMS_REQUEST_PARAMETER: {
                SctpAddStreamsRequestParameter *instreams = check_and_cast<SctpAddStreamsRequestParameter *>(parameter);
                stream.writeUint16Be(instreams->getParameterType());
                stream.writeUint16Be(12);
                stream.writeUint32Be(instreams->getSrReqSn());
                stream.writeUint16Be(instreams->getNumberOfStreams());
                stream.writeUint16Be(0);
                break;
            }
            default:
                throw cRuntimeError("Parameter Type %d not supported", parameter->getParameterType());
        }
    }
}
*/

}

void SctpHeaderSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& sctpHeader = staticPtrCast<const SctpHeader>(chunk);
    stream.writeUint16Be(sctpHeader->getSourcePort());
    stream.writeUint16Be(sctpHeader->getDestPort());
    stream.writeUint32Be(sctpHeader->getVTag());
    stream.writeUint32Be(sctpHeader->getCrc());

    int32 numChunks = sctpHeader->getSctpChunksArraySize();
    for (int32 cc = 0; cc < numChunks; cc++) {
        SctpChunk *chunk = const_cast<SctpChunk *>(check_and_cast<const SctpChunk *>((sctpHeader)->getSctpChunks(cc)));
        unsigned char chunkType = chunk->getSctpChunkType();
        switch (chunkType) {
            case DATA: {
                SctpDataChunk *dataChunk = check_and_cast<SctpDataChunk *>(chunk);
                uint32_t startPos = b(stream.getLength()).get();
                serializeDataChunk(stream, dataChunk);
                uint32_t len = (b(stream.getLength()).get() - startPos) / 8;
                stream.writeByteRepeatedly(0, paddingLength(len));
                break;
            }
            case INIT: {
                SctpInitChunk *initChunk = check_and_cast<SctpInitChunk *>(chunk);
                uint32_t startPos = b(stream.getLength()).get();
                serializeInitChunk(stream, initChunk);
                uint32_t len = (b(stream.getLength()).get() - startPos) / 8;
                stream.writeByteRepeatedly(0, paddingLength(len));
                break;
            }
            case INIT_ACK: {
                SctpInitAckChunk *initAckChunk = check_and_cast<SctpInitAckChunk *>(chunk);
                uint32_t startPos = b(stream.getLength()).get();
                serializeInitAckChunk(stream, initAckChunk);
                uint32_t len = (b(stream.getLength()).get() - startPos) / 8;
                stream.writeByteRepeatedly(0, paddingLength(len));
                break;
            }
            case SACK: {
                SctpSackChunk *sackChunk = check_and_cast<SctpSackChunk *>(chunk);
                uint32_t startPos = b(stream.getLength()).get();
                serializeSackChunk(stream, sackChunk);
                uint32_t len = (b(stream.getLength()).get() - startPos) / 8;
                stream.writeByteRepeatedly(0, paddingLength(len));
                break;
            }
            //case NR_SACK: {
            //    SctpSackChunk *sackChunk = check_and_cast<SctpSackChunk *>(chunk);
            //    serializeNrSackChunk(stream, sackChunk);
            //    break;
            //}
            case HEARTBEAT : {
                SctpHeartbeatChunk *heartbeatChunk = check_and_cast<SctpHeartbeatChunk *>(chunk);
                uint32_t startPos = b(stream.getLength()).get();
                serializeHeartbeatChunk(stream, heartbeatChunk);
                uint32_t len = (b(stream.getLength()).get() - startPos) / 8;
                stream.writeByteRepeatedly(0, paddingLength(len));
                break;
            }
            case HEARTBEAT_ACK : {
                SctpHeartbeatAckChunk *heartbeatAckChunk = check_and_cast<SctpHeartbeatAckChunk *>(chunk);
                uint32_t startPos = b(stream.getLength()).get();
                serializeHeartbeatAckChunk(stream, heartbeatAckChunk);
                uint32_t len = (b(stream.getLength()).get() - startPos) / 8;
                stream.writeByteRepeatedly(0, paddingLength(len));
                break;
            }
            case ABORT: {
                SctpAbortChunk *abortChunk = check_and_cast<SctpAbortChunk *>(chunk);
                serializeAbortChunk(stream, abortChunk);
                break;
            }
            case COOKIE_ECHO: {
                SctpCookieEchoChunk *cookieChunk = check_and_cast<SctpCookieEchoChunk *>(chunk);
                serializeCookieEchoChunk(stream, cookieChunk);
                break;
            }
            case COOKIE_ACK: {
                SctpCookieAckChunk *cookieAckChunk = check_and_cast<SctpCookieAckChunk *>(chunk);
                serializeCookieAckChunk(stream, cookieAckChunk);
                break;
            }
            case SHUTDOWN: {
                SctpShutdownChunk *shutdownChunk = check_and_cast<SctpShutdownChunk *>(chunk);
                serializeShutdownChunk(stream, shutdownChunk);
                break;
            }
            case SHUTDOWN_ACK: {
                SctpShutdownAckChunk *shutdownAckChunk = check_and_cast<SctpShutdownAckChunk *>(chunk);
                serializeShutdownAckChunk(stream, shutdownAckChunk);
                break;
            }
            case SHUTDOWN_COMPLETE: {
                SctpShutdownCompleteChunk *shutdownCompleteChunk = check_and_cast<SctpShutdownCompleteChunk *>(chunk);
                serializeShutdownCompleteChunk(stream, shutdownCompleteChunk);
                break;
            }
            //case AUTH: {
            //    SctpAuthenticationChunk *authChunk = check_and_cast<SctpAuthenticationChunk *>(chunk);
            //    serializeAuthenticationChunk(stream, authChunk);
            //    break;
            //}
            //case FORWARD_TSN: {
            //    SctpForwardTsnChunk *forward = check_and_cast<SctpForwardTsnChunk *>(chunk);
            //    serializeForwardTsnChunk(stream, forward);
            //    break;
            //}
            //case ASCONF: {
            //    SctpAsconfChunk *asconfChunk = check_and_cast<SctpAsconfChunk *>(chunk);
            //    serializeAsconfChangeChunk(stream, asconfChunk);
            //    break;
            //}
            //case ASCONF_ACK: {
            //    SctpAsconfAckChunk *asconfAckChunk = check_and_cast<SctpAsconfAckChunk *>(chunk);
            //    serializeAsconfAckChunk(stream, asconfAckChunk);
            //    break;
            //}
            //case ERRORTYPE: {
            //    SctpErrorChunk *errorchunk = check_and_cast<SctpErrorChunk *>(chunk);
            //    serializeErrorChunk(stream, errorchunk);
            //    break;
            //}
            //case RE_CONFIG: {
            //    SctpStreamResetChunk *streamReset = check_and_cast<SctpStreamResetChunk *>(chunk);
            //    serializeReConfigurationChunk(stream, streamReset);
            //    break;
            //}
            //case PKTDROP: {
                //SctpPacketDropChunk *packetdrop = check_and_cast<SctpPacketDropChunk *>(chunk);
                // TODO
            //    break;
            //}
            default:
                throw new cRuntimeError("Unknown chunk type %d in outgoing packet on external interface!", chunkType);
        }
    }
}

const Ptr<Chunk> SctpHeaderSerializer::deserialize(MemoryInputStream& stream) const
{
    auto sctpHeader = makeShared<SctpHeader>();
    sctpHeader->setSourcePort(stream.readUint16Be());
    sctpHeader->setDestPort(stream.readUint16Be());
    sctpHeader->setVTag(stream.readUint32Be());
    sctpHeader->setCrc(stream.readUint32Be());

    while (stream.getRemainingLength() > B(0)) {
        int8_t chunkType = stream.readByte();
        switch (chunkType) {
            case DATA: {
                SctpDataChunk *dataChunk = new SctpDataChunk("DATA");
                uint32_t startPos = b(stream.getRemainingLength()).get() + 8;
                deserializeDataChunk(stream, dataChunk);
                uint32_t len = (startPos - b(stream.getRemainingLength()).get()) / 8;
                stream.readByteRepeatedly(0, paddingLength(len));
                sctpHeader->insertSctpChunks(dataChunk);
                break;
            }
            case INIT: {
                SctpInitChunk *initChunk = new SctpInitChunk("INIT");
                uint32_t startPos = b(stream.getRemainingLength()).get() + 8;
                deserializeInitChunk(stream, initChunk);
                uint32_t len = (startPos - b(stream.getRemainingLength()).get()) / 8;
                stream.readByteRepeatedly(0, paddingLength(len));
                sctpHeader->insertSctpChunks(initChunk);
                break;
            }
            case INIT_ACK: {
                SctpInitAckChunk *initAckChunk = new SctpInitAckChunk("INIT_ACK");
                uint32_t startPos = b(stream.getRemainingLength()).get() + 8;
                deserializeInitAckChunk(stream, initAckChunk);
                uint32_t len = (startPos - b(stream.getRemainingLength()).get()) / 8;
                stream.readByteRepeatedly(0, paddingLength(len));
                sctpHeader->insertSctpChunks(initAckChunk);
                break;
            }
            case SACK: {
                SctpSackChunk *sackChunk = new SctpSackChunk("SACK");
                uint32_t startPos = b(stream.getRemainingLength()).get() + 8;
                deserializeSackChunk(stream, sackChunk);
                uint32_t len = (startPos - b(stream.getRemainingLength()).get()) / 8;
                stream.readByteRepeatedly(0, paddingLength(len));
                sctpHeader->insertSctpChunks(sackChunk);
                break;
            }
            //case NR_SACK: {
            //    SctpSackChunk *sackChunk = new SctpSackChunk("NR_SACK");
            //    deserializeNrSackChunk(stream, sackChunk);
            //    sctpHeader->insertSctpChunks(sackChunk);
            //    break;
            //}
            case HEARTBEAT: {
                SctpHeartbeatChunk *heartbeatChunk = new SctpHeartbeatChunk("HEARTBEAT");
                uint32_t startPos = b(stream.getRemainingLength()).get() + 8;
                deserializeHeartbeatChunk(stream, heartbeatChunk);
                uint32_t len = (startPos - b(stream.getRemainingLength()).get()) / 8;
                stream.readByteRepeatedly(0, paddingLength(len));
                sctpHeader->insertSctpChunks(heartbeatChunk);
                break;
            }
            case HEARTBEAT_ACK: {
                SctpHeartbeatAckChunk *heartbeatAckChunk = new SctpHeartbeatAckChunk("HEARTBEAT_ACK");
                uint32_t startPos = b(stream.getRemainingLength()).get() + 8;
                deserializeHeartbeatAckChunk(stream, heartbeatAckChunk);
                uint32_t len = (startPos - b(stream.getRemainingLength()).get()) / 8;
                stream.readByteRepeatedly(0, paddingLength(len));
                sctpHeader->insertSctpChunks(heartbeatAckChunk);
                break;
            }
            case ABORT: {
                SctpAbortChunk *abortChunk = new SctpAbortChunk("ABORT");
                deserializeAbortChunk(stream, abortChunk);
                sctpHeader->insertSctpChunks(abortChunk);
                break;
            }
            case COOKIE_ECHO: {
                SctpCookieEchoChunk *cookieChunk = new SctpCookieEchoChunk("COOKIE_ECHO");
                deserializeCookieEchoChunk(stream, cookieChunk);
                sctpHeader->insertSctpChunks(cookieChunk);
                break;
            }
            case COOKIE_ACK: {
                SctpCookieAckChunk *cookieAckChunk = new SctpCookieAckChunk("COOKIE_ACK");
                deserializeCookieAckChunk(stream, cookieAckChunk);
                sctpHeader->insertSctpChunks(cookieAckChunk);
                break;
            }
            case SHUTDOWN: {
                SctpShutdownChunk *shutdownChunk = new SctpShutdownChunk("SHUTDOWN");
                deserializeShutdownChunk(stream, shutdownChunk);
                sctpHeader->insertSctpChunks(shutdownChunk);
                break;
            }
            case SHUTDOWN_ACK: {
                SctpShutdownAckChunk *shutdownAckChunk = new SctpShutdownAckChunk("SHUTDOWN_ACK");
                deserializeShutdownAckChunk(stream, shutdownAckChunk);
                sctpHeader->insertSctpChunks(shutdownAckChunk);
                break;
            }
            case SHUTDOWN_COMPLETE: {
                SctpShutdownCompleteChunk *shutdownCompleteChunk = new SctpShutdownCompleteChunk("SHUTDOWN_COMPLETE");
                deserializeShutdownCompleteChunk(stream, shutdownCompleteChunk);
                sctpHeader->insertSctpChunks(shutdownCompleteChunk);
                break;
            }
            //case ERRORTYPE: {
            //    SctpErrorChunk *errorchunk = new SctpErrorChunk("ERROR");
            //    deserializeErrorChunk(stream, errorchunk);
            //    sctpHeader->insertSctpChunks(errorchunk);
            //    break;
            //}
            //case FORWARD_TSN: {
            //    SctpForwardTsnChunk *forward = new SctpForwardTsnChunk("FORWARD_TSN");
            //    deserializeForwardTsnChunk(stream, forward);
            //    sctpHeader->insertSctpChunks(forward);
            //    break;
            //}
            //case AUTH: {
            //    SctpAuthenticationChunk *authChunk = new SctpAuthenticationChunk("AUTH");
            //    deserializeAuthenticationChunk(stream, authChunk);
            //    sctpHeader->insertSctpChunks(authChunk);
            //    break;
            //}
            //case ASCONF: {
            //    SctpAsconfChunk *asconfChunk = new SctpAsconfChunk("ASCONF");
            //    deserializeAsconfChangeChunk(stream, asconfChunk);
            //    sctpHeader->insertSctpChunks(asconfChunk);
            //    break;
            //}
            //case ASCONF_ACK: {
            //    SctpAsconfAckChunk *asconfAckChunk = new SctpAsconfAckChunk("ASCONF_ACK");
            //    deserializeAsconfAckChunk(stream, asconfAckChunk);
            //    sctpHeader->insertSctpChunks(asconfAckChunk);
            //    break;
            //}
            //case RE_CONFIG: {
            //    SctpStreamResetChunk *chunk = new SctpStreamResetChunk("RE_CONFIG");
            //    // TODO
            //    break;
            //}
            //case PKTDROP: {
            //    // TODO
            //    break;
            //}
            default:
                EV_ERROR << "Parser: Unknown SCTP chunk type " << chunkType;
                sctpHeader->markIncorrect();
                return sctpHeader;
                break;
        }
    }
    sctpHeader->setChecksumOk(true);    // TODO: the serializer should not be concerned with checking the crc field
    return sctpHeader;
}


} // namespace sctp

} // namespace inet
