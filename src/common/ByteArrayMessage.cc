//
// (C) 2005 Vojtech Janota
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#include "inet/common/ByteArrayMessage.h"

namespace inet {

void ByteArrayMessage::setDataFromBuffer(const void *ptr, unsigned int length)
{
    byteArray_var.setDataFromBuffer(ptr, (unsigned int)length);
}

void ByteArrayMessage::addDataFromBuffer(const void *ptr, unsigned int length)
{
    byteArray_var.addDataFromBuffer(ptr, length);
}

unsigned int ByteArrayMessage::copyDataToBuffer(void *ptr, unsigned int length) const
{
    return byteArray_var.copyDataToBuffer(ptr, length);
}

void ByteArrayMessage::removePrefix(unsigned int length)
{
    byteArray_var.truncateData(length);
}

} // namespace inet

