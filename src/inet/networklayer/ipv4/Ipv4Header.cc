//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/INETUtils.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"

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
    options.insertTlvOption(opt);
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

} // namespace inet

