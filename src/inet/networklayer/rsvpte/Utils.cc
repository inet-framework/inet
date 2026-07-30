//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/rsvpte/Utils.h"

#include "inet/networklayer/rsvpte/IntServ_m.h"

namespace inet {

std::string vectorToString(const EroVector& vec)
{
    return vectorToString(vec, ", ");
}

std::string vectorToString(const EroVector& vec, const char *delim)
{
    std::ostringstream stream;
    for (unsigned int i = 0; i < vec.size(); i++) {
        stream << vec[i].node;

        if (i < vec.size() - 1)
            stream << delim;
    }
    stream << std::flush;
    std::string str(stream.str());
    return str;
}

} // namespace inet

