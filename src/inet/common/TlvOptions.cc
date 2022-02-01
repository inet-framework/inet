//
// Copyright (C) 2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
            removeTlvOption(i);
            eraseTlvOption(i);
            return option;
        }
    }
    return nullptr;
}

void TlvOptions::deleteOptionByType(int type, bool firstOnly)
{
    for (size_t i = 0; i < tlvOption_arraysize;) {
        if (tlvOption[i] && tlvOption[i]->getType() == type) {
            removeTlvOption(i);
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

