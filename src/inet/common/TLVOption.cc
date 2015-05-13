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
// @author Zoltan Bojthe
//


#include "TLVOption.h"

namespace inet {

Register_Class(TLVOptions);

void TLVOptions::copy(const TLVOptions& other)
{
    for (auto opt: other.optionVector)
        optionVector.push_back(opt->dup());
}

int TLVOptions::getLength() const
{
    int length = 0;
    for (auto opt: optionVector)
        length += opt->getLength();
    return length;
}

void TLVOptions::add(TLVOptionBase *opt, int atPos)
{
    ASSERT(opt->getLength() >= 1);
    if (atPos == -1)
        optionVector.push_back(opt);
    else
        optionVector.insert(optionVector.begin() + atPos, opt);
}

TLVOptionBase *TLVOptions::remove(TLVOptionBase *option)
{
    for (auto it = optionVector.begin(); it != optionVector.end(); ++it)
        if ((*it) == option) {
            optionVector.erase(it);
            return option;
        }
    return nullptr;
}

void TLVOptions::deleteOptionByType(int type, bool firstOnly)
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

void TLVOptions::clear()
{
    for (auto opt: optionVector)
        delete opt;
    optionVector.clear();
}

int TLVOptions::findByType(short int type, int firstPos) const
{
    for (unsigned int pos=firstPos; pos < optionVector.size(); pos++)
        if (optionVector[pos]->getType() == type)
            return pos;
    return -1;
}

void TLVOptions::parsimPack(cCommBuffer *b)
{
    TLVOptions_Base::parsimPack(b);
    TLVOptionVector::size_type s = optionVector.size();
    doPacking(b, s);
    for (auto opt: optionVector)
        b->packObject(opt);
}

void TLVOptions::parsimUnpack(cCommBuffer *b)
{
    TLVOptions_Base::parsimUnpack(b);
    TLVOptionVector::size_type s;
    doUnpacking(b, s);
    for (TLVOptionVector::size_type i = 0; i < s; i++)
        optionVector.push_back(check_and_cast<TLVOptionBase *>(b->unpackObject()));
}

} // namespace inet
