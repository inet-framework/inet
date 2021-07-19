//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

