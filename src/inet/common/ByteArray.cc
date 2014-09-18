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
        delete[] data_var;
        data_var = length ? new char[length] : NULL;
        data_arraysize = length;
    }
    if (length)
        memcpy(data_var, ptr, length);
}

void ByteArray::setDataFromByteArray(const ByteArray& other, unsigned int srcOffs, unsigned int length)
{
    ASSERT(srcOffs + length <= other.data_arraysize);
    setDataFromBuffer(other.data_var + srcOffs, length);
}

void ByteArray::addDataFromBuffer(const void *ptr, unsigned int length)
{
    if (0 == length)
        return;

    unsigned int nlength = data_arraysize + length;
    char *ndata_var = new char[nlength];
    if (data_arraysize)
        memcpy(ndata_var, data_var, data_arraysize);
    memcpy(ndata_var + data_arraysize, ptr, length);
    delete[] data_var;
    data_var = ndata_var;
    data_arraysize = nlength;
}

unsigned int ByteArray::copyDataToBuffer(void *ptr, unsigned int length, unsigned int srcOffs) const
{
    if (srcOffs >= data_arraysize)
        return 0;

    if (srcOffs + length > data_arraysize)
        length = data_arraysize - srcOffs;
    memcpy(ptr, data_var + srcOffs, length);
    return length;
}

void ByteArray::assignBuffer(void *ptr, unsigned int length)
{
    delete[] data_var;
    data_var = (char *)ptr;
    data_arraysize = length;
}

void ByteArray::truncateData(unsigned int truncleft, unsigned int truncright)
{
    ASSERT(data_arraysize >= (truncleft + truncright));

    if ((truncleft || truncright)) {
        unsigned int nlength = data_arraysize - (truncleft + truncright);
        char *ndata_var = NULL;
        if (nlength) {
            ndata_var = new char[nlength];
            memcpy(ndata_var, data_var + truncleft, nlength);
        }
        delete[] data_var;
        data_var = ndata_var;
        data_arraysize = nlength;
    }
}

} // namespace inet

