//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
     */
    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const {
        return stream << EV_FAINT << "<object@" << static_cast<const void *>(this) << ">" << EV_NORMAL;
    }

    virtual std::string printToString() const {
        return "";
    }

    virtual std::string printToString(int level, int evFlags = 0) const {
        std::stringstream s;
        printToStream(s, level, evFlags);
        return s.str();
    }

    virtual std::string getInfoStringRepresentation(int evFlags = 0) const {
        std::stringstream s;
        printToStream(s, PRINT_LEVEL_INFO, evFlags);
        return s.str();
    }

    virtual std::string getDetailStringRepresentation(int evFlags = 0) const {
        std::stringstream s;
        printToStream(s, PRINT_LEVEL_DETAIL, evFlags);
        return s.str();
    }

    virtual std::string getDebugStringRepresentation(int evFlags = 0) const {
        std::stringstream s;
        printToStream(s, PRINT_LEVEL_DEBUG, evFlags);
        return s.str();
    }

    virtual std::string getTraceStringRepresentation(int evFlags = 0) const {
        std::stringstream s;
        printToStream(s, PRINT_LEVEL_TRACE, evFlags);
        return s.str();
    }

    virtual std::string getCompleteStringRepresentation(int evFlags = 0) const {
        std::stringstream s;
        printToStream(s, PRINT_LEVEL_COMPLETE, evFlags);
        return s.str();
    }
};

inline std::ostream& operator<<(std::ostream& stream, const IPrintableObject *object)
{
    if (object == nullptr)
        return stream << EV_FAINT << "<nullptr>" << EV_NORMAL;
    else
        return object->printToStream(stream, cLog::logLevel, 0);
};

inline std::ostream& operator<<(std::ostream& stream, const IPrintableObject& object)
{
    return object.printToStream(stream, cLog::logLevel, 0);
};

inline std::string printFieldToString(const IPrintableObject *object, int level, int evFlags = 0)
{
    std::stringstream stream;
    if (object == nullptr)
        stream << EV_FAINT << "<nullptr>" << EV_NORMAL;
    else {
        stream << "{ ";
        object->printToStream(stream, level, evFlags);
        stream << " }";
    }
    return stream.str();
}

template<typename, typename = void>
struct has_print_to_string : std::false_type {};

template<typename T>
struct has_print_to_string<T, inet::void_t<decltype(std::declval<T>().printToString())>>
    : std::true_type {};

template<typename, typename = void>
struct has_str : std::false_type {};

template<typename T>
struct has_str<T, inet::void_t<decltype(std::declval<T>().str())>>
    : std::true_type {};

template<typename T>
std::enable_if_t<has_print_to_string<T>::value, std::string> printToStringIfPossible(T *object, int evFlags)
{
    if (object == nullptr)
        return "<nullptr>";
    else
        return object->printToString(cLog::logLevel, evFlags);
}

template<typename T>
std::enable_if_t<has_print_to_string<T>::value, std::string> printToStringIfPossible(const T *object, int evFlags)
{
    if (object == nullptr)
        return "<nullptr>";
    else
        return object->printToString(cLog::logLevel, evFlags);
}

template<typename T>
std::enable_if_t<has_print_to_string<T>::value, std::string> printToStringIfPossible(T& object, int evFlags)
{
    return object.printToString(cLog::logLevel, evFlags);
}

template<typename T>
std::enable_if_t<has_print_to_string<T>::value, std::string> printToStringIfPossible(const T& object, int evFlags)
{
    return object.printToString(cLog::logLevel, evFlags);
}

template<typename T>
std::enable_if_t<std::is_polymorphic<T>::value && has_str<T>::value && !has_print_to_string<T>::value, std::string> printToStringIfPossible(T *object, int evFlags)
{
    if (object == nullptr)
        return "<nullptr>";
    else if (auto printableObject = dynamic_cast<IPrintableObject *>(object))
        return printableObject->printToString(cLog::logLevel, evFlags);
    else if (auto cobject = dynamic_cast<cObject *>(object)) {
        std::stringstream s;
        s << cobject;
        return s.str();
    }
    else
        return object->str();
}

template<typename T>
std::enable_if_t<std::is_polymorphic<T>::value && has_str<T>::value && !has_print_to_string<T>::value, std::string> printToStringIfPossible(const T& object, int evFlags)
{
    if (auto printableObject = dynamic_cast<const IPrintableObject *>(&object))
        return printableObject->printToString(cLog::logLevel, evFlags);
    else if (auto cobject = dynamic_cast<const cObject *>(&object)) {
        std::stringstream s;
        s << cobject;
        return s.str();
    }
    else
        return object.str();
}

template<typename T>
std::enable_if_t<std::is_polymorphic<T>::value && has_str<T>::value && !has_print_to_string<T>::value, std::string> printToStringIfPossible(const T *object, int evFlags)
{
    if (object == nullptr)
        return "<nullptr>";
    else if (auto printableObject = dynamic_cast<const IPrintableObject *>(object))
        return printableObject->printToString(cLog::logLevel, evFlags);
    else if (auto cobject = dynamic_cast<const cObject *>(object)) {
        std::stringstream s;
        s << cobject;
        return s.str();
    }
    else
        return object->str();
}

template<typename T>
std::enable_if_t<std::is_polymorphic<T>::value && !has_str<T>::value && !has_print_to_string<T>::value, std::string> printToStringIfPossible(T *object, int evFlags)
{
    if (object == nullptr)
        return "<nullptr>";
    else if (auto printableObject = dynamic_cast<IPrintableObject *>(object))
        return printableObject->printToString(cLog::logLevel, evFlags);
    else if (auto cobject = dynamic_cast<cObject *>(object))
        return cobject->str();
    else {
        std::stringstream s;
        s << object;
        return s.str();
    }
}

template<typename T>
std::enable_if_t<std::is_polymorphic<T>::value && !has_str<T>::value && !has_print_to_string<T>::value, std::string> printToStringIfPossible(const T *object, int evFlags)
{
    if (object == nullptr)
        return "<nullptr>";
    else if (auto printableObject = dynamic_cast<const IPrintableObject *>(object))
        return printableObject->printToString(cLog::logLevel, evFlags);
    else if (auto cobject = dynamic_cast<const cObject *>(object))
        return cobject->str();
    else {
        std::stringstream s;
        s << object;
        return s.str();
    }
}

template<typename T>
std::enable_if_t<std::is_polymorphic<T>::value && !has_str<T>::value && !has_print_to_string<T>::value, std::string> printToStringIfPossible(const T& object, int evFlags)
{
    if (auto printableObject = dynamic_cast<const IPrintableObject *>(&object))
        return printableObject->printToString(cLog::logLevel, evFlags);
    else if (auto cobject = dynamic_cast<const cObject *>(&object))
        return cobject->str();
    else {
        std::stringstream s;
        s << object;
        return s.str();
    }
}

template<typename T>
std::enable_if_t<!std::is_polymorphic<T>::value && has_str<T>::value && !has_print_to_string<T>::value, std::string> printToStringIfPossible(const T& object, int evFlags)
{
    std::stringstream s;
    s << object;
    return s.str();
}

template<typename T>
std::enable_if_t<!std::is_polymorphic<T>::value && !has_str<T>::value && !has_print_to_string<T>::value, std::string> printToStringIfPossible(const T& object, int evFlags)
{
    std::stringstream s;
    s << object;
    return s.str();
}

} // namespace inet

#endif

