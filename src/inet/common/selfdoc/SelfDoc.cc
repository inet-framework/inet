//
// Copyright (C) 2019 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// @author: Zoltan Bojthe
//

#include <cstdarg>
#include <fstream>

#include "inet/common/INETDefs.h"

#include "inet/common/packet/Message.h"
#include "inet/common/packet/Packet.h"

namespace inet {

OPP_THREAD_LOCAL SelfDoc globalSelfDoc;

Register_PerRunConfigOption(CFGID_GENERATE_SELFDOC, "generate-selfdoc", CFG_BOOL, "false", "Enable/disable the generate SelfDoc file");

OPP_THREAD_LOCAL bool SelfDoc::generateSelfdoc = false;

EXECUTE_PRE_NETWORK_SETUP(SelfDoc::generateSelfdoc = cSimulation::getActiveEnvir()->getConfig()->getAsBool(CFGID_GENERATE_SELFDOC));


SelfDoc::~SelfDoc() noexcept(false)
{
    if (generateSelfdoc) {
        std::ofstream file;
        file.open("/tmp/SelfDoc.txt", std::ofstream::out | std::ofstream::app);
        if (file.fail())
            throw std::ios_base::failure(std::strerror(errno));

        //make sure write fails with exception if something is wrong
        file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

        for (const auto& elem : textSet)
            file << elem << ',' << std::endl;
        file.close();
    }
}

std::string SelfDoc::kindToStr(int kind, cProperties *properties1, const char *propName1, cProperties *properties2, const char *propName2)
{
    if (generateSelfdoc) {
        if (properties1) {
            auto prop = properties1->get(propName1);
            if (!prop && properties2)
                prop = properties2->get(propName2);
            if (prop) {
                if (auto propValue = prop->getValue()) {
                    if (auto e = omnetpp::cEnum::find(propValue)) {
                        if (auto t = e->getStringFor(kind)) {
                            std::ostringstream os;
                            os << kind << " (" << propValue << "::" << t << ")";
                            return os.str();
                        }
                    }
                }
            }
        }
        return std::to_string(kind);
    }
    else
        return std::string("");
}

std::string SelfDoc::val(const char *str)
{
    std::ostringstream os;
    os << '"';
    char hexbuf[5];
    while (*str) {
        switch (*str) {
            case '\b': os << "\\b"; break;
            case '\f': os << "\\f"; break;
            case '\n': os << "\\n"; break;
            case '\r': os << "\\r"; break;
            case '\t': os << "\\t"; break;
            case '"':  os << "\\\""; break;
            case '\\': os << "\\\\"; break;
            default: if (iscntrl(*str)) { sprintf(hexbuf,"\\x%2.2X",*str); os << hexbuf; } else { os << *str; } break;
        }
        str++;
    }
    os << '"';
    return os.str();
}

const char *SelfDoc::enterMethodInfo(const char *methodFmt, ...)
{
    if (methodFmt == nullptr)
        return "";
    if (0 == strcmp("%s", methodFmt)) {
        std::va_list args;
        va_start(args, methodFmt);
        const char *str = va_arg(args, const char *);
        va_end(args);
        return str;
    }
    else
        return methodFmt;
}

static SharingTagSet *findTags(cMessage *msg)
{
    if (auto packet = dynamic_cast<Packet *>(msg))
        return &packet->getTags();
    if (auto message = dynamic_cast<Message *>(msg))
        return &message->getTags();
    return nullptr;
}

std::string SelfDoc::tagsToJson(const char *key, cMessage *msg)
{
    std::set<std::string> tagSet;
    std::ostringstream os;
    auto tags = findTags(msg);
    if (tags) {
        auto cnt = tags->getNumTags();
        for (int i=0; i < cnt; i++) {
            auto tag = tags->getTag(i).get();
            if (tag)
                tagSet.insert(opp_typename(typeid(*tag)));
        }
    }
    os << SelfDoc::val(key) << " : [ ";
    const char *sep = "";
    for (const auto& t : tagSet) {
        os << sep << SelfDoc::val(t);
        sep = ", ";
    }
    os << " ]";
    return os.str();
}

std::string SelfDoc::gateInfo(cGate *gate)
{
    if (gate) {
        std::ostringstream os;
        os << gate->getName() << (gate->isVector() ? "[]" : "");
        return os.str();
    }
    else
        return "";
}

} // namespace inet

