//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/INETUtils.h"
#include "inet/common/checksum/TcpIpChecksum.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/networklayer/ipv4/Ipv4HeaderSerializer.h"

namespace inet {

Register_Class(Ipv4Header);

TlvOptionBase *Ipv4Header::findMutableOptionByType(short int optionType, int index)
{
    handleChange();
    int i = options.findByType(optionType, index);
    return i >= 0 ? &getOptionForUpdate(i) : nullptr;
}

const TlvOptionBase *Ipv4Header::findOptionByType(short int optionType, int index) const
{
    int i = options.findByType(optionType, index);
    return i >= 0 ? &getOption(i) : nullptr;
}

void Ipv4Header::addOption(TlvOptionBase *opt)
{
    handleChange();
    options.appendTlvOption(opt);
}

void Ipv4Header::addOption(TlvOptionBase *opt, int atPos)
{
    handleChange();
    options.insertTlvOption(atPos, opt);
}

B Ipv4Header::calculateHeaderByteLength() const
{
    int length = utils::roundUp(20 + options.getLength(), 4);
    ASSERT(length >= 20 && length <= 60 && (length % 4 == 0));

    return B(length);
}

short Ipv4Header::getDscp() const
{
    return (typeOfService & 0xfc) >> 2;
}

void Ipv4Header::setDscp(short dscp)
{
    setTypeOfService(((dscp & 0x3f) << 2) | (typeOfService & 0x03));
}

short Ipv4Header::getEcn() const
{
    return typeOfService & 0x03;
}

void Ipv4Header::setEcn(short ecn)
{
    setTypeOfService((typeOfService & 0xfc) | (ecn & 0x03));
}

void Ipv4Header::updateCrc()
{
    switch (crcMode) {
        case CRC_DECLARED_CORRECT:
            // if the CRC mode is declared to be correct, then set the CRC to an easily recognizable value
            setCrc(0xC00D);
            break;
        case CRC_DECLARED_INCORRECT:
            // if the CRC mode is declared to be incorrect, then set the CRC to an easily recognizable value
            setCrc(0xBAAD);
            break;
        case CRC_COMPUTED: {
            // if the CRC mode is computed, then compute the CRC and set it
            // this computation is delayed after the routing decision, see INetfilter hook
            setCrc(0);
            MemoryOutputStream ipv4HeaderStream;
            Ipv4HeaderSerializer::serialize(ipv4HeaderStream, *this);
            // compute the CRC
            uint16_t crc = TcpIpChecksum::checksum(ipv4HeaderStream.getData());
            setCrc(crc);
            break;
        }
        default:
            throw cRuntimeError("Unknown CRC mode: %d", (int)crcMode);
    }
}

bool Ipv4Header::verifyCrc() const
{
    switch (crcMode) {
        case CRC_DECLARED_CORRECT: {
            // if the CRC mode is declared to be correct, then the check passes if and only if the chunk is correct
            return true;
        }
        case CRC_DECLARED_INCORRECT:
            // if the CRC mode is declared to be incorrect, then the check fails
            return false;
        case CRC_COMPUTED: {
            // compute the CRC, the check passes if the result is 0xFFFF (includes the received CRC) and the chunks are correct
            MemoryOutputStream ipv4HeaderStream;
            Ipv4HeaderSerializer::serialize(ipv4HeaderStream, *this);
            uint16_t computedCrc = TcpIpChecksum::checksum(ipv4HeaderStream.getData());
            return computedCrc == 0;
        }
        default:
            throw cRuntimeError("Unknown CRC mode");
    }
}

} // namespace inet

