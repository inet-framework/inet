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

#include "ByteArrayMessage.h"

void ByteArrayMessage::setDataFromBuffer(const void *ptr, int length)
{
    ASSERT(length > 0);

    delete[] data_var;
    data_var = new char[length];
    data_arraysize = length;
    memcpy(data_var, ptr, length);
}

void ByteArrayMessage::copyDataToBuffer(void *ptr, int length)
{
    ASSERT((uint)length <= data_arraysize);

    memcpy(ptr, data_var, length);
}

void ByteArrayMessage::removePrefix(int length)
{
    ASSERT(data_arraysize > (uint)length);
    ASSERT(length > 0);

    int nlength = data_arraysize - length;
    char *data_var2 = new char[nlength];
    memcpy(data_var2, data_var+length, nlength);
    delete[] data_var;
    data_var = data_var2;
    data_arraysize = nlength;
}


