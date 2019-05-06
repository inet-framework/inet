//
// Copyright (C) 2015 OpenSim Ltd.
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
// @author Zoltan Bojthe
//


#include "inet/common/TlvOptions_m.h"

namespace inet {

int TlvOptions::getLength() const
{
    int length = 0;
    for (size_t i = 0; i < tlvOption_arraysize; i++) {
        if (tlvOption[i])
            length += tlvOption[i]->getLength();
    }
    return length;
}

TlvOptionBase *TlvOptions::dropTlvOption(TlvOptionBase *option)
{
    for (size_t i = 0; i < tlvOption_arraysize; i++) {
        if (tlvOption[i] == option) {
            dropTlvOption(i);
            eraseTlvOption(i);
            return option;
        }
    }
    return nullptr;
}

void TlvOptions::deleteOptionByType(int type, bool firstOnly)
{
    for (size_t i = 0; i < tlvOption_arraysize; ) {
        if (tlvOption[i] && tlvOption[i]->getType() == type) {
            dropTlvOption(i);
            eraseTlvOption(i);
            if (firstOnly)
                break;
        }
        else
            ++i;
    }
}

int TlvOptions::findByType(short int type, int firstPos) const
{
    for (size_t i = firstPos; i < tlvOption_arraysize; i++)
        if (tlvOption[i] && tlvOption[i]->getType() == type)
            return i;
    return -1;
}

} // namespace inet
