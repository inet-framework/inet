//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2003 Xuan Thang Nguyen
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/mpls/LibTable.h"

#include <iostream>

#include "inet/common/XMLUtils.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace inet {

Define_Module(LibTable);

void LibTable::initialize(int stage)
{
    SimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        // dynamically allocated labels must never collide with the RFC 3032
        // reserved range (0-15), which the data plane (Mpls.cc) now interprets
        // specially (explicit null, unassigned)
        maxLabel = RESERVED_LABEL_MAX;
        ift.reference(this, "interfaceTableModule", true);
        WATCH(maxLabel);
        WATCH(lib);
        WATCH_EXPR("numLabels", lib.size());
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        // read configuration (by this stage all interfaces are registered in ift)
        readTableFromXML(par("config"));
    }
}

void LibTable::handleMessage(cMessage *msg)
{
    throw cRuntimeError("LibTable does not process messages, but received '%s'", msg->getName());
}

bool LibTable::resolveLabel(int inInterfaceId, int inLabel,
        LabelOpVector& outLabel, int& outInterfaceId)
{
    bool any = (inInterfaceId == ANY_INTERFACE);

    for (auto& elem : lib) {
        if (!any && elem.inInterfaceId != inInterfaceId)
            continue;

        if (elem.inLabel != inLabel)
            continue;

        outLabel = elem.outLabel;
        outInterfaceId = elem.outInterfaceId;

        return true;
    }
    return false;
}

int LibTable::installLibEntry(int inLabel, int inInterfaceId, const LabelOpVector& outLabel,
        int outInterfaceId)
{
    if (inLabel == -1) {
        LibEntry newItem;
        newItem.inLabel = ++maxLabel;
        newItem.inInterfaceId = inInterfaceId;
        newItem.outLabel = outLabel;
        newItem.outInterfaceId = outInterfaceId;
        lib.push_back(newItem);
        return newItem.inLabel;
    }
    else {
        for (auto& elem : lib) {
            if (elem.inLabel != inLabel)
                continue;

            elem.inInterfaceId = inInterfaceId;
            elem.outLabel = outLabel;
            elem.outInterfaceId = outInterfaceId;
            return inLabel;
        }
        throw cRuntimeError("Cannot update LIB entry: no entry with inLabel=%d", inLabel);
    }
}

void LibTable::removeLibEntry(int inLabel)
{
    for (unsigned int i = 0; i < lib.size(); i++) {
        if (lib[i].inLabel != inLabel)
            continue;

        lib.erase(lib.begin() + i);
        return;
    }
    throw cRuntimeError("Cannot remove LIB entry: no entry with inLabel=%d", inLabel);
}

void LibTable::readTableFromXML(const cXMLElement *libtable)
{
    using namespace xmlutils;

    ASSERT(libtable);
    ASSERT(!strcmp(libtable->getTagName(), "libtable"));
    checkTags(libtable, "libentry");
    cXMLElementList list = libtable->getChildrenByTagName("libentry");
    for (auto& elem : list) {
        const cXMLElement& entry = *elem;

        checkTags(&entry, "inLabel inInterface outLabel outInterface");

        LibEntry newItem;
        newItem.inLabel = getParameterIntValue(&entry, "inLabel");
        if (newItem.inLabel < 0)
            throw cRuntimeError("Invalid libentry at %s: inLabel must be >= 0, but is %d", entry.getSourceLocation(), newItem.inLabel);

        const char *inInterfaceName = getParameterStrValue(&entry, "inInterface");
        if (!strcmp(inInterfaceName, "any")) {
            // conventional sentinel for "matches regardless of incoming interface"
            // (used e.g. for statically configured ingress/PUSH entries)
            newItem.inInterfaceId = ANY_INTERFACE;
        }
        else {
            NetworkInterface *inIe = ift->findInterfaceByName(inInterfaceName);
            if (!inIe)
                throw cRuntimeError("Invalid libentry at %s: inInterface '%s' not registered by any interface", entry.getSourceLocation(), inInterfaceName);
            newItem.inInterfaceId = inIe->getInterfaceId();
        }

        const char *outInterfaceName = getParameterStrValue(&entry, "outInterface");
        NetworkInterface *outIe = ift->findInterfaceByName(outInterfaceName);
        if (!outIe)
            throw cRuntimeError("Invalid libentry at %s: outInterface '%s' not registered by any interface", entry.getSourceLocation(), outInterfaceName);
        newItem.outInterfaceId = outIe->getInterfaceId();

        cXMLElementList ops = getUniqueChild(&entry, "outLabel")->getChildrenByTagName("op");
        for (auto& ops_oit : ops) {
            const cXMLElement& op = *ops_oit;
            const char *val = op.getAttribute("value");
            const char *code = op.getAttribute("code");
            if (!code)
                throw cRuntimeError("Invalid label op at %s: missing 'code' attribute", op.getSourceLocation());
            LabelOp l;

            if (!strcmp(code, "push")) {
                l.optcode = PUSH_OPER;
                if (!val)
                    throw cRuntimeError("Invalid push op at %s: missing 'value' attribute", op.getSourceLocation());
                l.label = atoi(val);
                if (l.label < 0)
                    throw cRuntimeError("Invalid push op at %s: label must be >= 0, but is '%s'", op.getSourceLocation(), val);
            }
            else if (!strcmp(code, "pop")) {
                l.optcode = POP_OPER;
                if (val)
                    throw cRuntimeError("Invalid pop op at %s: must not have a 'value' attribute", op.getSourceLocation());
            }
            else if (!strcmp(code, "swap")) {
                l.optcode = SWAP_OPER;
                if (!val)
                    throw cRuntimeError("Invalid swap op at %s: missing 'value' attribute", op.getSourceLocation());
                l.label = atoi(val);
                if (l.label < 0)
                    throw cRuntimeError("Invalid swap op at %s: label must be >= 0, but is '%s'", op.getSourceLocation(), val);
            }
            else
                throw cRuntimeError("Invalid label op at %s: unknown code '%s'", op.getSourceLocation(), code);

            newItem.outLabel.push_back(l);
        }

        lib.push_back(newItem);

        if (newItem.inLabel > maxLabel)
            maxLabel = newItem.inLabel;
    }
}

LabelOpVector LibTable::pushLabel(int label)
{
    LabelOpVector vec;
    LabelOp lop;
    lop.optcode = PUSH_OPER;
    lop.label = label;
    vec.push_back(lop);
    return vec;
}

LabelOpVector LibTable::swapLabel(int label)
{
    LabelOpVector vec;
    LabelOp lop;
    lop.optcode = SWAP_OPER;
    lop.label = label;
    vec.push_back(lop);
    return vec;
}

LabelOpVector LibTable::popLabel()
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

std::ostream& operator<<(std::ostream& os, const LibTable::LibEntry& lib)
{
    os << "inLabel:" << lib.inLabel;
    os << "    inInterfaceId:" << lib.inInterfaceId;
    os << "    outLabel:" << lib.outLabel;
    os << "    outInterfaceId:" << lib.outInterfaceId;
    return os;
}

} // namespace inet

