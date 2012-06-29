#ifndef __GNPLIB_IMPL_UTIL_XML_STRING_H
#define	__GNPLIB_IMPL_UTIL_XML_STRING_H

/*
 * File: string.h
 * Copyright (C) 2011 Philipp Berndt <philipp.berndt@tu-berlin.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLString.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>

namespace gnplib { namespace impl { namespace util { namespace xml {

/**
 * Smart pointer / handle for XMLCh* strings with automatic transformation.
 * @author philipp.berndt@tu-berlin.de
 */
template <typename src_t, typename dst_t>
class _Str : public boost::noncopyable
{
    dst_t* s;

    /**
     * This is for enabling functions to return a Str.
     */
private:

    class trans
    {
    public:

        inline explicit trans(_Str str)
        {
            s=str.s;
            str.s=0;
        }

        inline dst_t* release()
        { dst_t* res(s); s=0; return res; }
    private:
        dst_t* s;
    };
public:

    inline _Str(const src_t* xml_string)
    : s(XERCES_CPP_NAMESPACE::XMLString::transcode(xml_string)) { }

    inline _Str(_Str& other)
    { s = other.s; other.s = 0; }

    inline _Str(trans t)
    : s(t.release()) {}

    inline operator trans()
    { return trans(*this); }

    inline operator const dst_t*() const
    {
        return s;
    }

    inline const dst_t* c_str() const
    {
        return s;
    }

    template<typename T>
    T as() const
    {
        return boost::lexical_cast<T>(s);
    }

    /** Convenience function returning specified default value iff this Str is nullptr or an empty string
     * e.g. iff the attribute does not exist.
     * Still throws on unparsable input.
     */
    template<typename T>
    T as(const T& dflt) const
    {
        return s && s[0] ? boost::lexical_cast<T>(s) : dflt;
    }

    inline ~_Str()
    {
        if (s)
            XERCES_CPP_NAMESPACE::XMLString::release(&s);
    }

};

typedef _Str<XMLCh, char> Str;
typedef _Str<char, XMLCh> XmlStr;

class EmptyString
{
};

inline const char* operator+(const EmptyString&, const Str& str)
{
    return str.c_str() ? str.c_str() : "";
}


}}}} // namespace gnplib::impl::util::xml

#endif // not defined __GNPLIB_IMPL_UTIL_XML_STRING_H

