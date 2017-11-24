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


#include "TLVOption.h"

namespace inet {

Register_Class(TlvOptions);

void TlvOptions::copy(const TlvOptions& other)
{
    for (auto opt: other.optionVector)
        optionVector.push_back(opt->dup());
}

int TlvOptions::getLength() const
{
    int length = 0;
    for (auto opt: optionVector)
        length += opt->getLength();
    return length;
}

void TlvOptions::add(TlvOptionBase *opt, int atPos)
{
    ASSERT(opt->getLength() >= 1);
    if (atPos == -1)
        optionVector.push_back(opt);
    else
        optionVector.insert(optionVector.begin() + atPos, opt);
}

TlvOptionBase *TlvOptions::remove(TlvOptionBase *option)
{
    for (auto it = optionVector.begin(); it != optionVector.end(); ++it)
        if ((*it) == option) {
            optionVector.erase(it);
            return option;
        }
    return nullptr;
}

void TlvOptions::deleteOptionByType(int type, bool firstOnly)
{
    auto it = optionVector.begin();
    while (it != optionVector.end()) {
        if ((*it)->getType() == type) {
            delete (*it);
            it = optionVector.erase(it);
            if (firstOnly)
                break;
        }
        else
            ++it;
    }
}

void TlvOptions::clear()
{
    for (auto opt: optionVector)
        delete opt;
    optionVector.clear();
}

int TlvOptions::findByType(short int type, int firstPos) const
{
    for (unsigned int pos=firstPos; pos < optionVector.size(); pos++)
        if (optionVector[pos]->getType() == type)
            return pos;
    return -1;
}

void TlvOptions::parsimPack(cCommBuffer *b) const
{
    TlvOptions_Base::parsimPack(b);
    int s = (int)optionVector.size();
    doParsimPacking(b, s);
    for (auto opt: optionVector)
        b->packObject(opt);
}

void TlvOptions::parsimUnpack(cCommBuffer *b)
{
    TlvOptions_Base::parsimUnpack(b);
    int s;
    doParsimUnpacking(b, s);
    for (int i = 0; i < s; i++)
        optionVector.push_back(check_and_cast<TlvOptionBase *>(b->unpackObject()));
}

} // namespace inet
