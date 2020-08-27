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

#ifndef __INET_IPRINTABLEOBJECT_H
#define __INET_IPRINTABLEOBJECT_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * This purely virtual interface provides an abstraction for printable objects.
 */
class INET_API IPrintableObject
{
  public:
    enum PrintLevel {
        PRINT_LEVEL_TRACE,
        PRINT_LEVEL_DEBUG,
        PRINT_LEVEL_DETAIL,
        PRINT_LEVEL_INFO,
        PRINT_LEVEL_COMPLETE = INT_MIN
    };

    enum PrintFlag {
        PRINT_FLAG_FORMATTED = (1 << 0),
        PRINT_FLAG_MULTILINE = (1 << 1),
    };

  public:
    virtual ~IPrintableObject() {}

    /**
     * Prints this object to the provided output stream.
     *
     * Function calls to operator<< with pointers or references either const
     * or not are all forwarded to this function.
     */
    virtual std::ostream& printToStream(std::ostream& stream, int level, int flags = 0) const {
        return stream << "<object@" << static_cast<const void *>(this) << ">";
    }

    virtual std::string getInfoStringRepresentation(int flags = 0) const {
        std::stringstream s; printToStream(s, PRINT_LEVEL_INFO, flags); return s.str();
    }

    virtual std::string getDetailStringRepresentation(int flags = 0) const {
        std::stringstream s; printToStream(s, PRINT_LEVEL_DETAIL, flags); return s.str();
    }

    virtual std::string getDebugStringRepresentation(int flags = 0) const {
        std::stringstream s; printToStream(s, PRINT_LEVEL_DEBUG, flags); return s.str();
    }

    virtual std::string getTraceStringRepresentation(int flags = 0) const {
        std::stringstream s; printToStream(s, PRINT_LEVEL_TRACE, flags); return s.str();
    }

    virtual std::string getCompleteStringRepresentation(int flags = 0) const {
        std::stringstream s; printToStream(s, PRINT_LEVEL_COMPLETE, flags); return s.str();
    }
};

inline std::ostream& operator<<(std::ostream& stream, const IPrintableObject *object)
{
    return object->printToStream(stream, cLog::logLevel, 0);
};

inline std::ostream& operator<<(std::ostream& stream, const IPrintableObject& object)
{
    return object.printToStream(stream, cLog::logLevel, 0);
};

inline std::string printObjectToString(const IPrintableObject *object, int level, int flags = 0)
{
    std::stringstream s;
    if (object == nullptr)
        return "nullptr";
    else {
        s << "{ ";
        object->printToStream(s, level, flags);
        s << " }";
        return s.str();
    }
}

} // namespace inet

#endif

