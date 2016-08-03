//
// (C) 2005 Vojtech Janota
// (C) 2010 Zoltan Bojthe
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

#include "inet/common/ByteArray.h"

namespace inet {

void ByteArray::setDataFromBuffer(const void *ptr, unsigned int length)
{
    if (length != data_arraysize) {
        delete[] data;
        data = length ? new char[length] : nullptr;
        data_arraysize = length;
    }
    if (length)
        memcpy(data, ptr, length);
}

void ByteArray::copyDataFromBuffer(unsigned int destOffset, const void *ptr, unsigned int length)
{
    unsigned int nlength = destOffset + length;
    if (data_arraysize < nlength) {
        char *ndata = new char[nlength];
        if (data_arraysize)
            memcpy(ndata, data, data_arraysize);
        delete[] data;
        data = ndata;
    }
    memcpy(data + destOffset, ptr, length);
}

void ByteArray::setDataFromByteArray(const ByteArray& other, unsigned int srcOffs, unsigned int length)
{
    ASSERT(srcOffs + length <= other.data_arraysize);
    setDataFromBuffer(other.data + srcOffs, length);
}

void ByteArray::copyDataFromByteArray(unsigned int destOffset, const ByteArray& other, unsigned int srcOffset, unsigned int length)
{
    ASSERT(srcOffset + length <= other.data_arraysize);
    copyDataFromBuffer(destOffset, other.data + srcOffset, length);
}


void ByteArray::addDataFromBuffer(const void *ptr, unsigned int length)
{
    if (0 == length)
        return;

    unsigned int nlength = data_arraysize + length;
    char *ndata = new char[nlength];
    if (data_arraysize)
        memcpy(ndata, data, data_arraysize);
    memcpy(ndata + data_arraysize, ptr, length);
    delete[] data;
    data = ndata;
    data_arraysize = nlength;
}

unsigned int ByteArray::copyDataToBuffer(void *ptr, unsigned int length, unsigned int srcOffs) const
{
    if (srcOffs >= data_arraysize)
        return 0;

    if (srcOffs + length > data_arraysize)
        length = data_arraysize - srcOffs;
    memcpy(ptr, data + srcOffs, length);
    return length;
}

void ByteArray::assignBuffer(void *ptr, unsigned int length)
{
    delete[] data;
    data = (char *)ptr;
    data_arraysize = length;
}

void ByteArray::truncateData(unsigned int truncleft, unsigned int truncright)
{
    ASSERT(data_arraysize >= (truncleft + truncright));

    if ((truncleft || truncright)) {
        unsigned int nlength = data_arraysize - (truncleft + truncright);
        char *ndata = nullptr;
        if (nlength) {
            ndata = new char[nlength];
            memcpy(ndata, data + truncleft, nlength);
        }
        delete[] data;
        data = ndata;
        data_arraysize = nlength;
    }
}

void ByteArray::expandData(unsigned int addleft, unsigned int addright)
{
    ASSERT(data_arraysize <= (data_arraysize + addleft + addright));

    if ((addleft || addright)) {
        unsigned int nlength = data_arraysize + (addleft + addright);
        char *ndata = new char[nlength];
        memmove(ndata + addleft, data, data_arraysize);
        delete[] data;
        data = ndata;
        data_arraysize = nlength;
    }
}

} // namespace inet

