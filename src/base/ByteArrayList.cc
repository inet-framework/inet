//
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

#include "ByteArrayList.h"

ByteArrayList::ByteArrayList()
 :
    dataLengthM(0)
{
    // FIXME
}

ByteArrayList::ByteArrayList(const ByteArrayList& other)
 :
    dataLengthM(other.dataLengthM),
    dataListM(other.dataListM)
{
}

ByteArrayList& ByteArrayList::operator=(const ByteArrayList& other)
{
    dataLengthM = other.dataLengthM;
    dataListM = other.dataListM;
    return *this;
}

void ByteArrayList::push(const ByteArray& byteArrayP)
{
    dataLengthM += byteArrayP.getDataArraySize();
    dataListM.push_back(byteArrayP);
}

unsigned int ByteArrayList::getBytesToBuffer(void* bufferP, unsigned int bufferLengthP)
{
    // FIXME
    unsigned int copiedBytes = 0;
    unsigned int bytes = bufferLengthP;

    DataList::iterator i;
    for(i=this->dataListM.begin(); (0 < bytes) && (i != dataListM.end()); ++i)
    {
        unsigned int cbytes = std::min(bytes, i->getDataArraySize());
        i->copyDataToBuffer((char *)bufferP + copiedBytes, cbytes);
        copiedBytes += cbytes;
        bytes -= cbytes;
    }
    return copiedBytes;
}

unsigned int ByteArrayList::moveBytesToBuffer(void* bufferP, unsigned int bufferLengthP)
{
    return drop(getBytesToBuffer(bufferP,bufferLengthP));
}

unsigned int ByteArrayList::drop(unsigned int lengthP)
{
    ASSERT(lengthP <= dataLengthM);

    unsigned int length = lengthP;
    while (length > 0)
    {
        DataList::iterator i = dataListM.begin();
        unsigned int cbytes = i->getDataArraySize();
        if (cbytes <= length)
        {
            dataListM.pop_front();
            dataLengthM -= cbytes;
            length -= cbytes;
        }
        else
        {
            i->truncateData(length);
            dataLengthM -= length;
            length = 0;
        }
    }
    ASSERT(0 == length);
    return lengthP;
}

void ByteArrayList::clear()
{
    dataLengthM = 0;
    while (dataListM.begin() != dataListM.end())
    {
        dataListM.pop_front();
    }
}
