//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/packet/Message.h"

namespace inet {

Register_Class(Message);

Message::Message(const char *name, short kind) :
    cMessage(name, kind)
{
}

Message::Message(const Message& other) :
    cMessage(other),
    tags(other.tags)
{
}

std::ostream& Message::printToStream(std::ostream& stream, int level, int evFlags) const
{
    std::string className = getClassName();
    auto index = className.rfind("::");
    if (index != std::string::npos)
        className = className.substr(index + 2);
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FAINT << "(" << className << ")" << EV_NORMAL;
    stream << EV_ITALIC << getName() << EV_NORMAL;
    return stream;
}

Request::Request(const char *name, short kind) :
    Message(name, kind)
{
}

Request::Request(const Request& other) :
    Message(other)
{
}

Indication::Indication(const char *name, short kind) :
    Message(name, kind)
{
}

Indication::Indication(const Indication& other) :
    Message(other)
{
}

} // namespace

