
#include "SocketMsg.h"

void SocketMsg::setDataFromBuffer(const void *ptr, int length)
{
    ASSERT(length > 0);

    delete[] data_var;
    data_var = new char[length];
    data_arraysize = length;
    memcpy(data_var, ptr, length);
}

void SocketMsg::copyDataToBuffer(void *ptr, int length)
{
    ASSERT(length <= data_arraysize);

    memcpy(ptr, data_var, length);
}

void SocketMsg::removePrefix(int length)
{
    ASSERT(data_arraysize > length);
    ASSERT(length > 0);

    int nlength = data_arraysize - length;
    char *data_var2 = new char[nlength];
    memcpy(data_var2, data_var+length, nlength);
    delete[] data_var;
    data_var = data_var2;
    data_arraysize = nlength;
}