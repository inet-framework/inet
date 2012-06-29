/* 
 * File:   PathSAX2Handler.cpp
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

#include <gnplib/impl/util/xml/PathSAX2Handler.h>
#include <gnplib/impl/util/xml/string.h>

#include <iostream>
#include <boost/tokenizer.hpp>

using std::string;
using std::cerr;
using std::endl;
using std::cout;
using std::vector;
using namespace gnplib::impl::util::xml;

using boost::tokenizer;
using boost::char_separator;

using XERCES_CPP_NAMESPACE::SAXParseException;
using XERCES_CPP_NAMESPACE::XMLString;

MySAX2Handler::MySAX2Handler()
{
    current_node.push_back(new Node());
}

MySAX2Handler::~MySAX2Handler()
{
    delete current_node.front();
}

MySAX2Handler::Node::~Node()
{
    for (children_t::iterator it=children.begin(); it!=children.end(); ++it)
        delete it->second;
}

void MySAX2Handler::startElement(const XMLCh*const uri,
                                 const XMLCh*const localname,
                                 const XMLCh*const qname,
                                 const XERCES_CPP_NAMESPACE::Attributes& attrs)
{
    string name(Str(localname).c_str());
    current_path.push_back(name);

//    cout<<"Path:";
//    for (vector<string>::const_iterator i=current_path.begin(); i!=current_path.end(); ++i)
//        cout<< *i<<" ";
//    cout<<endl;

    Node*node(current_node.back());
    if (node)
    {
        const Node::children_t&children(node->children);
        Node::children_t::const_iterator child=children.find(name);
        node=child!=children.end() ? child->second : 0;
        current_node.push_back(node);
    }
    if (node&& !node->startElementSignal.empty())
    {
//        cout<<"Calling handler"<<endl;
        node->startElementSignal(uri, localname, qname, attrs);
    }
}

void MySAX2Handler::endElement(const XMLCh*const uri,
                               const XMLCh*const localname,
                               const XMLCh*const qname)
{
    if (!current_path.empty())
    {
        current_path.pop_back();
        if (current_node.size()>current_path.size()+1)
            current_node.pop_back();
    }
}

void MySAX2Handler::fatalError(const SAXParseException& exception)
{
    Str message(exception.getMessage());
    cerr<<"Fatal Error: "<<message
            <<" at line: "<<exception.getLineNumber()<<endl;
    throw std::runtime_error(message.c_str());
}

boost::signals::connection MySAX2Handler::registerStartElementListener(const string& path, const startElement_t::slot_type& slot)
{
    typedef tokenizer< char_separator<char> > tokenizer;
    char_separator<char> sep("/");
    tokenizer tokens(path, sep);
    Node*node(current_node.front());
    for (tokenizer::iterator tok_iter=tokens.begin();
         tok_iter!=tokens.end(); ++tok_iter)
    {
        string name(*tok_iter);
        Node*& child(node->children[name]);
        if (!child)
            child=new Node();
        node=child;
    }
    return node->startElementSignal.connect(slot);
}
