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

#ifndef __MMAPBOARD_H
#define __MMAPBOARD_H

#include <map>

#include "ModuleAccess.h"


/**
 * This class include posix like mmap
 */
class INET_API MMapBoard : public cSimpleModule
{
  public:
    struct MemoryMap: public cPolymorphic
    {
        unsigned int numProcAsociated;
        unsigned int size;
        void *commonPtr;
        MemoryMap() {numProcAsociated = size = 0; commonPtr = NULL;}
    };
    typedef std::map<std::string, MemoryMap*> ClientMemoryMap;
    ClientMemoryMap clientMemoryMap;

  protected:
    /**
     * Initialize.
     */
    virtual void initialize();
    /**
     * Does nothing.
     */
    virtual void handleMessage(cMessage *msg);

  public:
    const void * mmap(std::string name, int *size, int flags = 0, int *status = NULL);
    void munmap(std::string name);
    ~MMapBoard();
};

/**
 * Gives access to the MamoryMap instance within the host/router.
 */
class INET_API MMapBoardAccess : public ModuleAccess<MMapBoard>
{
  public:
    MMapBoardAccess() : ModuleAccess<MMapBoard>("mmapBoard") {}
};

#endif

