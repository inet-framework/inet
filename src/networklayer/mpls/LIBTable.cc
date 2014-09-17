//
// (C) 2005 Vojtech Janota
// (C) 2003 Xuan Thang Nguyen
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#include <iostream>
#include "inet/networklayer/mpls/LIBTable.h"
#include "inet/common/XMLUtils.h"

namespace inet {

Define_Module(LIBTable);

void LIBTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        maxLabel = 0;
        WATCH_VECTOR(lib);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        // read configuration
        readTableFromXML(par("config").xmlValue());
    }
}

void LIBTable::handleMessage(cMessage *)
{
    ASSERT(false);
}

bool LIBTable::resolveLabel(std::string inInterface, int inLabel,
        LabelOpVector& outLabel, std::string& outInterface, int& color)
{
    bool any = (inInterface.length() == 0);

    for (unsigned int i = 0; i < lib.size(); i++) {
        if (!any && lib[i].inInterface != inInterface)
            continue;

        if (lib[i].inLabel != inLabel)
            continue;

        outLabel = lib[i].outLabel;
        outInterface = lib[i].outInterface;
        color = lib[i].color;

        return true;
    }
    return false;
}

int LIBTable::installLibEntry(int inLabel, std::string inInterface, const LabelOpVector& outLabel,
        std::string outInterface, int color)
{
    if (inLabel == -1) {
        LIBEntry newItem;
        newItem.inLabel = ++maxLabel;
        newItem.inInterface = inInterface;
        newItem.outLabel = outLabel;
        newItem.outInterface = outInterface;
        newItem.color = color;
        lib.push_back(newItem);
        return newItem.inLabel;
    }
    else {
        for (unsigned int i = 0; i < lib.size(); i++) {
            if (lib[i].inLabel != inLabel)
                continue;

            lib[i].inInterface = inInterface;
            lib[i].outLabel = outLabel;
            lib[i].outInterface = outInterface;
            lib[i].color = color;
            return inLabel;
        }
        ASSERT(false);
        return 0;    // prevent warning
    }
}

void LIBTable::removeLibEntry(int inLabel)
{
    for (unsigned int i = 0; i < lib.size(); i++) {
        if (lib[i].inLabel != inLabel)
            continue;

        lib.erase(lib.begin() + i);
        return;
    }
    ASSERT(false);
}

void LIBTable::readTableFromXML(const cXMLElement *libtable)
{
    using namespace xmlutils;

    ASSERT(libtable);
    ASSERT(!strcmp(libtable->getTagName(), "libtable"));
    checkTags(libtable, "libentry");
    cXMLElementList list = libtable->getChildrenByTagName("libentry");
    for (cXMLElementList::iterator it = list.begin(); it != list.end(); it++) {
        const cXMLElement& entry = **it;

        checkTags(&entry, "inLabel inInterface outLabel outInterface color");

        LIBEntry newItem;
        newItem.inLabel = getParameterIntValue(&entry, "inLabel");
        newItem.inInterface = getParameterStrValue(&entry, "inInterface");
        newItem.outInterface = getParameterStrValue(&entry, "outInterface");
        newItem.color = getParameterIntValue(&entry, "color", 0);

        cXMLElementList ops = getUniqueChild(&entry, "outLabel")->getChildrenByTagName("op");
        for (cXMLElementList::iterator oit = ops.begin(); oit != ops.end(); oit++) {
            const cXMLElement& op = **oit;
            const char *val = op.getAttribute("value");
            const char *code = op.getAttribute("code");
            ASSERT(code);
            LabelOp l;

            if (!strcmp(code, "push")) {
                l.optcode = PUSH_OPER;
                ASSERT(val);
                l.label = atoi(val);
                ASSERT(l.label > 0);
            }
            else if (!strcmp(code, "pop")) {
                l.optcode = POP_OPER;
                ASSERT(!val);
            }
            else if (!strcmp(code, "swap")) {
                l.optcode = SWAP_OPER;
                ASSERT(val);
                l.label = atoi(val);
                ASSERT(l.label > 0);
            }
            else
                ASSERT(false);

            newItem.outLabel.push_back(l);
        }

        lib.push_back(newItem);

        ASSERT(newItem.inLabel > 0);

        if (newItem.inLabel > maxLabel)
            maxLabel = newItem.inLabel;
    }
}

LabelOpVector LIBTable::pushLabel(int label)
{
    LabelOpVector vec;
    LabelOp lop;
    lop.optcode = PUSH_OPER;
    lop.label = label;
    vec.push_back(lop);
    return vec;
}

LabelOpVector LIBTable::swapLabel(int label)
{
    LabelOpVector vec;
    LabelOp lop;
    lop.optcode = SWAP_OPER;
    lop.label = label;
    vec.push_back(lop);
    return vec;
}

LabelOpVector LIBTable::popLabel()
{
    LabelOpVector vec;
    LabelOp lop;
    lop.optcode = POP_OPER;
    lop.label = 0;
    vec.push_back(lop);
    return vec;
}

std::ostream& operator<<(std::ostream& os, const LabelOpVector& label)
{
    os << "{";
    for (unsigned int i = 0; i < label.size(); i++) {
        switch (label[i].optcode) {
            case PUSH_OPER:
                os << "PUSH " << label[i].label;
                break;

            case SWAP_OPER:
                os << "SWAP " << label[i].label;
                break;

            case POP_OPER:
                os << "POP";
                break;

            default:
                ASSERT(false);
                break;
        }

        if (i < label.size() - 1)
            os << "; ";
        else
            os << "}";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const LIBTable::LIBEntry& lib)
{
    os << "inLabel:" << lib.inLabel;
    os << "    inInterface:" << lib.inInterface;
    os << "    outLabel:" << lib.outLabel;
    os << "    outInterface:" << lib.outInterface;
    os << "    color:" << lib.color;
    return os;
}

} // namespace inet

