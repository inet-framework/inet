//
// Copyright (C) 2010 Alfonso Ariza
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "MMapBoard.h"

Define_Module(MMapBoard);

const void *MMapBoard::mmap(std::string name, int *size, int flags, int *status)
{
    Enter_Method("map memory (%s)", name.c_str());

    // find or create entry for this category
    MemoryMap *mapData;
    ClientMemoryMap::iterator it = clientMemoryMap.find(name);
    if (it != clientMemoryMap.end())
    {
        if (status != NULL)
            *status = 1;

        if (flags)
            return NULL;

        mapData = it->second;
        *size = mapData->size;
        mapData->numProcAsociated++;
    }
    else
    {
        if (status != NULL)
            *status = 0;
        mapData = new MemoryMap;
        mapData->size = *size;
        char *c = new char [*size];
        mapData->commonPtr = (void *)c;
        mapData->numProcAsociated = 1;
        clientMemoryMap.insert(std::pair<std::string,MemoryMap*>(name,mapData));
    }
    return mapData->commonPtr;
}

void MMapBoard::munmap(std::string name)
{
    Enter_Method("unmap memory (%s)", name.c_str());

    // find (or create) entry for this category
    ClientMemoryMap::iterator it = clientMemoryMap.find(name);
    if (it != clientMemoryMap.end())
    {
        MemoryMap *mapData = it->second;
        mapData->numProcAsociated--;
        if (!mapData->numProcAsociated)
        {
            if (mapData->commonPtr)
            {
                char *c = (char*)mapData->commonPtr;
                delete []c;
            }
            clientMemoryMap.erase(it);
            delete mapData;
        }
    }
}

void MMapBoard::initialize()
{
    WATCH_MAP(clientMemoryMap);
}

void MMapBoard::handleMessage(cMessage *msg)
{
    error("MMapBoard doesn't handle messages, it can be accessed via direct method calls");
}

MMapBoard::~MMapBoard()
{
    while (!clientMemoryMap.empty())
    {
        MemoryMap *mapData = clientMemoryMap.begin()->second;
        if (mapData->commonPtr)
        {
            char *c = (char*)mapData->commonPtr;
            delete []c;
        }
        delete mapData;
        clientMemoryMap.erase(clientMemoryMap.begin());
    }
}

