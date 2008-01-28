/***************************************************************************
                          sdes.cc  -  description
                             -------------------
    begin                : Tue Oct 23 2001
    copyright            : (C) 2001 by Matthias Oppitz
    email                : Matthias.Oppitz@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** \file sdes.cc
 * This file contains the implementations for member functions of the
 * classes SDESItem and SDESChunk.
 */

#include <string.h>

#include "types.h"
#include "sdes.h"

Register_Class(SDESItem);


SDESItem::SDESItem(const char *name) : cObject(name) {
    _type = SDES_UNDEF;
    _length = 2;
    _content = "";
};


SDESItem::SDESItem(SDES_ITEM_TYPE type, const char *content) : cObject() {
    _type = type;
    _content = content;
    // an sdes item requires one byte for the type field,
    // one byte for the length field and bytes for
    // the content string
    _length = 2 + strlen(_content);
};


SDESItem::SDESItem(const SDESItem& sdesItem) : cObject() {
    setName(sdesItem.name());
    operator=(sdesItem);
};


SDESItem::~SDESItem() {
};


SDESItem& SDESItem::operator=(const SDESItem& sdesItem) {
    cObject::operator=(sdesItem);
    _type = sdesItem._type;
    _length = sdesItem._length;
    _content = opp_strdup(sdesItem._content);
    return *this;
};


cObject *SDESItem::dup() const {
    return new SDESItem(*this);
};


const char *SDESItem::className() const {
    return "SDESItem";
};


std::string SDESItem::info() {
    std::stringstream out;
    out << "SDESItem=" << _content;
    return out.str();
};


void SDESItem::writeContents(std::ostream& os) {
    os << "SDESItem:" << endl;
    os << "  type = " << _type << endl;
    os << "  content = " << _content << endl;
};


SDESItem::SDES_ITEM_TYPE SDESItem::type() {
    return _type;
};


const char *SDESItem::content() {
    return opp_strdup(_content);
};


int SDESItem::length() {
    // bytes needed for this sdes item are
    // one byte for type, one for length
    // and the string
    return _length + 2;
};


//
// SDESChunk
//

Register_Class(SDESChunk);


SDESChunk::SDESChunk(const char *name, u_int32 ssrc) : cArray(name) {
    _ssrc = ssrc;
    _length = 4;
};


SDESChunk::SDESChunk(const SDESChunk& sdesChunk) : cArray(sdesChunk) {
    setName(sdesChunk.name());
    operator=(sdesChunk);
};


SDESChunk::~SDESChunk() {
};


SDESChunk& SDESChunk::operator=(const SDESChunk& sdesChunk) {
    cArray::operator=(sdesChunk);
    _ssrc = sdesChunk._ssrc;
    _length = sdesChunk._length;
    return *this;
};


cObject *SDESChunk::dup() const {
    return new SDESChunk(*this);
};


const char *SDESChunk::className() const {
    return "SDESChunk";
};


std::string SDESChunk::info() {
    std::stringstream out;
    out << "SDESChunk.ssrc=" << _ssrc << " items=" << items();
    return out.str();
};


void SDESChunk::writeContents(std::ostream& os) {
    os << "SDESChunk:" << endl;
    os << "  ssrc = " << _ssrc << endl;
    for (int i = 0; i < items(); i++) {
        if (exist(i)) {
            get(i)->writeContents(os);
        };
    };
};


void SDESChunk::addSDESItem(SDESItem *sdesItem) {
    for (int i = 0; i < items(); i++) {
        if (exist(i)) {
            SDESItem *compareItem = (SDESItem *)(get(i));
            if (compareItem->type() == sdesItem->type()) {
                remove(compareItem);
                _length = _length - compareItem->length();
                delete compareItem;
            };
        }
    };

    //sdesItem->setOwner(this);
    add(sdesItem);
    _length = _length + (sdesItem->length());

};


u_int32 SDESChunk::ssrc() {
    return _ssrc;
};


void SDESChunk::setSSRC(u_int32 ssrc) {
    _ssrc = ssrc;
};


int SDESChunk::length() {
    return _length;
};
