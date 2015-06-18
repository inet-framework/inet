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

#include "inet/networklayer/ipv4/IPv4Datagram.h"

#include "inet/common/INETUtils.h"

namespace inet {

Register_Class(IPv4Datagram);

int IPv4Datagram::getTotalLengthField() const
{
    return totalLengthField_var == -1 ? getByteLength() : totalLengthField_var;
}

TLVOptionBase *IPv4Datagram::findOptionByType(short int optionType, int index)
{
    int i = options_var.findByType(optionType, index);
    return i >= 0 ? &getOption(i) : nullptr;
}

void IPv4Datagram::addOption(TLVOptionBase *opt, int atPos)
{
    options_var.add(opt, atPos);
}

int IPv4Datagram::calculateHeaderByteLength() const
{
    int length = utils::roundUp(20 + options_var.getLength(), 4);
    ASSERT(length >= 20 && length <= 60 && (length % 4 == 0));

    return length;
}

} // namespace inet

