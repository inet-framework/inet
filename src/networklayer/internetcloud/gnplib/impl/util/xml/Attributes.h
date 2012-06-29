#ifndef __GNPLIB_IMPL_UTIL_XML_ATTRIBUTES_H
#define	__GNPLIB_IMPL_UTIL_XML_ATTRIBUTES_H

/*
 * File: Attributes.h
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

#include <xercesc/sax2/Attributes.hpp>
#include <gnplib/impl/util/xml/gnpstring.h>


namespace gnplib { namespace impl { namespace util { namespace xml {

typedef Str Attribute;

/**
 * Wrapper class for xerces::Attributes.
 * Provides access to attributes using operator[] returning a Str that
 * can be converted using as<>() template function:
 *
 * Attributes attribs(attribs);
 * double d = attribs["foo"].as<double>();
 */
class Attributes
{
public:

    inline Attributes(const XERCES_CPP_NAMESPACE::Attributes& _attribs)
    : attribs(_attribs) { }

    inline Attribute operator[](const XMLCh* name) const
    {
        return Attribute(attribs.getValue(name));
    }

    inline Attribute operator[](const char* name) const
    {
        return (*this)[XmlStr(name)];
    }

    inline Attribute operator[](const std::string& name) const
    {
        return (*this)[name.c_str()];
    }

private:
    const XERCES_CPP_NAMESPACE::Attributes& attribs;
};


}}}} // namespace gnplib::impl::util::xml

#endif // not defined __GNPLIB_IMPL_UTIL_XML_ATTRIBUTES_H
