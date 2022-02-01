//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/TagBase.h"

namespace inet {

std::ostream& TagBase::printToStream(std::ostream &stream, int level, int evFlags) const
{
    std::string className = getClassName();
    auto index = className.rfind("::");
    if (index != std::string::npos)
        className = className.substr(index + 2);
    stream << EV_FAINT << className << EV_NORMAL;
    return printFieldsToStream(stream, level, evFlags);
}

std::ostream& TagBase::printFieldsToStream(std::ostream &stream, int level, int evFlags) const
{
    auto descriptor = getDescriptor();
    // TODO make this more sophisticated, e.g. add properties to fields to control what is printed
    if (level <= PRINT_LEVEL_DETAIL)
        for (int i = 0; i < descriptor->getFieldCount(); i++)
            if (!descriptor->getFieldIsArray(i) && strcmp("omnetpp::cObject", descriptor->getFieldDeclaredOn(i)))
                stream << ", " << EV_BOLD << descriptor->getFieldName(i) << EV_NORMAL << " = " << descriptor->getFieldValueAsString(toAnyPtr(this), i, 0);
    return stream;
}

std::string TagBase::str() const
{
    std::stringstream stream;
    printFieldsToStream(stream, PRINT_LEVEL_COMPLETE, 0);
    return stream.tellp() == 0 ? "" : stream.str().substr(2);
}

} // namespace inet

