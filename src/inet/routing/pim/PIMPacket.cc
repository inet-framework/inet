//
// Copyright (C) 2014 OpenSim Ltd.
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

#include "inet/routing/pim/PIMPacket.h"

namespace inet {

Register_Class(PIMHello);

PIMHello::PIMHello(const char *name, int kind) : PIMHello_Base(name, kind)
{
    this->setType(Hello);

    options_arraysize = 0;
    this->options_var = nullptr;
}

PIMHello::PIMHello(const PIMHello& other) : PIMHello_Base(other)
{
    options_arraysize = 0;
    this->options_var = nullptr;
    copy(other);
}

PIMHello::~PIMHello()
{
    if (options_var) {
        for (unsigned int i = 0; i < options_arraysize; i++)
            delete options_var[i];
    }
    delete[] options_var;
}

PIMHello& PIMHello::operator=(const PIMHello& other)
{
    if (this == &other)
        return *this;
    PIMPacket::operator=(other);
    copy(other);
    return *this;
}

void PIMHello::copy(const PIMHello& other)
{
    for (unsigned int i = 0; i < options_arraysize; i++)
        delete options_var[i];
    delete[] options_var;

    this->options_var = (other.options_arraysize == 0) ? nullptr : new HelloOptionPtr[other.options_arraysize];
    options_arraysize = other.options_arraysize;
    for (unsigned int i = 0; i < options_arraysize; i++)
        this->options_var[i] = other.options_var[i]->dup();
}

void PIMHello::parsimPack(cCommBuffer *b) PARSIMPACK_CONST
{
    PIMHello_Base::parsimPack(b);
    b->pack(options_arraysize);
    for (unsigned int i = 0; i < options_arraysize; i++) {
        if (options_var[i]) {
            doParsimPacking(b, options_var[i]->getType());
            doParsimPacking(b, options_var[i]);
        }
        else
            doParsimPacking(b, (short int)0);
    }
}

void PIMHello::parsimUnpack(cCommBuffer *b)
{
    PIMHello_Base::parsimUnpack(b);
    for (unsigned int i = 0; i < options_arraysize; i++)
        delete options_var[i];
    delete[] this->options_var;
    b->unpack(options_arraysize);
    if (options_arraysize == 0) {
        this->options_var = nullptr;
    }
    else {
        this->options_var = new HelloOptionPtr[options_arraysize];
        for (unsigned int i = 0; i < options_arraysize; i++) {
            short int type;
            doParsimUnpacking(b, type);
            switch (type) {
                case 0:
                    options_var[i] = nullptr;
                    break;

                case Holdtime:
                    options_var[i] = new HoldtimeOption();
                    break;

                case LANPruneDelay:
                    options_var[i] = new LANPruneDelayOption();
                    break;

                case DRPriority:
                    options_var[i] = new DRPriorityOption();
                    break;

                case GenerationID:
                    options_var[i] = new GenerationIDOption();
                    break;

                //case StateRefreshCapable: TODO  break;
                //case AddressList: break;
                default:
                    throw cRuntimeError("PIMHello::parsimUnpack(): unknown option type: %sd.", type);
            }
            if (options_var[i])
                doParsimUnpacking(b, *options_var[i]);
        }
    }
}

void PIMHello::setOptionsArraySize(unsigned int size)
{
    HelloOptionPtr *options_var2 = (size == 0) ? nullptr : new HelloOptionPtr[size];
    for (unsigned int i = 0; i < size; i++)
        options_var2[i] = i < options_arraysize ? options_var[i] : nullptr;
    for (unsigned int i = size; i < options_arraysize; i++)
        delete options_var[i];
    options_arraysize = size;
    delete[] options_var;
    this->options_var = options_var2;
}

unsigned int PIMHello::getOptionsArraySize() const
{
    return options_arraysize;
}

HelloOptionPtr& PIMHello::getOptions(unsigned int k)
{
    if (k >= options_arraysize)
        throw cRuntimeError("Array of size %d indexed by %d", options_arraysize, k);
    return options_var[k];
}

void PIMHello::setOptions(unsigned int k, const HelloOptionPtr& options)
{
    if (k >= options_arraysize)
        throw cRuntimeError("Array of size %d indexed by %d", options_arraysize, k);
    delete this->options_var[k];
    this->options_var[k] = options;
}

}    // namespace inet

