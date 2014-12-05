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

#include <string.h>

#include "inet/transportlayer/rtp/sdes.h"

namespace inet {

namespace rtp {

Register_Class(SDESItem);

SDESItem::SDESItem() : cObject()
{
    _type = SDES_UNDEF;
    _length = 2;
    _content = "";
}

SDESItem::SDESItem(SDES_ITEM_TYPE type, const char *content) : cObject()
{
    if (nullptr == content)
        throw cRuntimeError("The content parameter must be a valid pointer.");
    _type = type;
    _content = content;
    // an sdes item requires one byte for the type field,
    // one byte for the length field and bytes for
    // the content string
    _length = 2 + strlen(_content);
}

SDESItem::SDESItem(const SDESItem& sdesItem) : cObject(sdesItem)
{
    copy(sdesItem);
}

SDESItem::~SDESItem()
{
    clean();
}

SDESItem& SDESItem::operator=(const SDESItem& sdesItem)
{
    if (this == &sdesItem)
        return *this;
    clean();
    cObject::operator=(sdesItem);
    copy(sdesItem);
    return *this;
}

void SDESItem::copy(const SDESItem& sdesItem)
{
    _type = sdesItem._type;
    _length = sdesItem._length;
    _content = opp_strdup(sdesItem._content);
}

SDESItem *SDESItem::dup() const
{
    return new SDESItem(*this);
}

std::string SDESItem::info() const
{
    std::stringstream out;
    out << "SDESItem=" << _content;
    return out.str();
}

void SDESItem::dump(std::ostream& os) const
{
    os << "SDESItem:" << endl;
    os << "  type = " << _type << endl;
    os << "  content = " << _content << endl;
}

SDESItem::SDES_ITEM_TYPE SDESItem::getType() const
{
    return _type;
}

const char *SDESItem::getContent() const
{
    return opp_strdup(_content);
}

int SDESItem::getLength() const
{
    // bytes needed for this sdes item are
    // one byte for type, one for length
    // and the string
    return _length + 2;
}

//
// SDESChunk
//

Register_Class(SDESChunk);

SDESChunk::SDESChunk(const char *name, uint32 ssrc) : cArray(name)
{
    _ssrc = ssrc;
    _length = 4;
}

SDESChunk::SDESChunk(const SDESChunk& sdesChunk) : cArray(sdesChunk)
{
    copy(sdesChunk);
}

SDESChunk::~SDESChunk()
{
}

SDESChunk& SDESChunk::operator=(const SDESChunk& sdesChunk)
{
    if (this == &sdesChunk)
        return *this;
    cArray::operator=(sdesChunk);
    copy(sdesChunk);
    return *this;
}

inline void SDESChunk::copy(const SDESChunk& sdesChunk)
{
    _ssrc = sdesChunk._ssrc;
    _length = sdesChunk._length;
}

SDESChunk *SDESChunk::dup() const
{
    return new SDESChunk(*this);
}

std::string SDESChunk::info() const
{
    std::stringstream out;
    out << "SDESChunk.ssrc=" << _ssrc << " items=" << size();
    return out.str();
}

void SDESChunk::dump(std::ostream& os) const
{
    os << "SDESChunk:" << endl;
    os << "  ssrc = " << _ssrc << endl;
    for (int i = 0; i < size(); i++) {
        if (exist(i)) {
            ((const SDESItem *)(get(i)))->dump(os);
        }
    }
}

void SDESChunk::addSDESItem(SDESItem *sdesItem)
{
    for (int i = 0; i < size(); i++) {
        if (exist(i)) {
            SDESItem *compareItem = (SDESItem *)(get(i));
            if (compareItem->getType() == sdesItem->getType()) {
                remove(compareItem);
                _length = _length - compareItem->getLength();
                delete compareItem;
            }
        }
    }

    //sdesItem->setOwner(this);
    add(sdesItem);
    _length += sdesItem->getLength();
}

uint32 SDESChunk::getSsrc() const
{
    return _ssrc;
}

void SDESChunk::setSsrc(uint32 ssrc)
{
    _ssrc = ssrc;
}

int SDESChunk::getLength() const
{
    return _length;
}

} // namespace rtp

} // namespace inet

