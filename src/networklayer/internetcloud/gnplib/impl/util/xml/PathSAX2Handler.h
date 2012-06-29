#ifndef __GNPLIB_IMPL_UTIL_XML_PATHSAX2HANDLER_H
#define	__GNPLIB_IMPL_UTIL_XML_PATHSAX2HANDLER_H

/*
 * File:   PathSAX2Handler.h
 * Copyright (C) 2009 Philipp Berndt <philipp.berndt@tu-berlin.de>
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

#include <xercesc/sax2/DefaultHandler.hpp>
#include <boost/signal.hpp>

namespace gnplib { namespace impl { namespace util { namespace xml {

class MySAX2Handler : public xercesc::DefaultHandler
{
public:
    typedef boost::signal<void (const XMLCh*const uri,
                                const XMLCh*const localname,
                                const XMLCh*const qname,
                                const xercesc::Attributes& attrs)>startElement_t;

    struct Node
    {
        typedef std::map<std::string, Node*> children_t;
        children_t children;
        startElement_t startElementSignal;
        ~Node();
    };

    std::vector<std::string> current_path;
    std::vector<Node*> current_node; // [0] holds root node

public:
    MySAX2Handler();
    ~MySAX2Handler();
    void startElement(const XMLCh*const uri,
                      const XMLCh*const localname,
                      const XMLCh*const qname,
                      const xercesc::Attributes& attrs);

    void endElement(const XMLCh*const uri,
                    const XMLCh*const localname,
                    const XMLCh*const qname);

    void fatalError(const xercesc::SAXParseException&);

    /**
     * Register callback slot to be called when node with given path is encountered
     */
    boost::signals::connection registerStartElementListener(const std::string& path, const startElement_t::slot_type& slot);
};

}}}} // namespace gnplib::impl::util::xml

#endif // not defined __GNPLIB_IMPL_UTIL_XML_PATHSAX2HANDLER_H
