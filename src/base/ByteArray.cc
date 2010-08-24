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

#include "ByteArray.h"

void ByteArray::setDataFromBuffer(const void *ptr, unsigned int length)
{
    delete[] data_var;
    data_var = NULL;
    if (length)
    {
        data_var = new char[length];
        memcpy(data_var, ptr, length);
    }
    data_arraysize = length;
}

void ByteArray::addDataFromBuffer(const void *ptr, unsigned int length)
{
    if (0 == length)
        return;

    unsigned int nlength = data_arraysize + length;
    char *ndata_var = new char[nlength];
    if (data_arraysize)
        memcpy(ndata_var, data_var, data_arraysize);
    memcpy(ndata_var, data_var + data_arraysize, length);
    delete[] data_var;
    data_var = ndata_var;
}

void ByteArray::copyDataToBuffer(void *ptr, unsigned int length) const
{
    ASSERT(length <= data_arraysize);

    memcpy(ptr, data_var, length);
}

void ByteArray::truncateData(unsigned int truncleft, unsigned int truncright)
{
    ASSERT(data_arraysize >= (truncleft + truncright));

    if ((truncleft || truncright))
    {
        unsigned int nlength = data_arraysize - (truncleft + truncright);
        char *ndata_var = NULL;
        if (nlength)
        {
            ndata_var = new char[nlength];
            memcpy(ndata_var, data_var+truncleft, nlength);
        }
        delete[] data_var;
        data_var = ndata_var;
        data_arraysize = nlength;
    }
}
