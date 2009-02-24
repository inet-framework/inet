//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef BONNMOTIONFILECACHE_H
#define BONNMOTIONFILECACHE_H

#include <list>
#include <vector>
#include <omnetpp.h>
#include "BasicMobility.h"

class BonnMotionFileCache;

/**
 * Represents a BonnMotion file's contents.
 * @see BonnMotionFileCache, BonnMotionMobility
 */
class INET_API BonnMotionFile
{
  public:
    typedef std::vector<double> Line;
  protected:
    friend class BonnMotionFileCache;
    typedef std::list<Line> LineList;
    LineList lines;
  public:
    const Line *getLine(int nodeId) const;
};


/**
 * Singleton object to read and store BonnMotion files. Used within
 * BonnMotionMobility.  Needed because otherwise every node would
 * have to open and read the file independently.
 *
 * @ingroup mobility
 * @author Andras Varga
 */
class INET_API BonnMotionFileCache
{
  protected:
    typedef std::map<std::string,BonnMotionFile> BMFileMap;
    BMFileMap cache;
    static BonnMotionFileCache *inst;
    void parseFile(const char *filename, BonnMotionFile& bmFile);
    BonnMotionFileCache() {}
    virtual ~BonnMotionFileCache() {}

  public:
    /**
     * Returns the singleton instance.
     */
    static BonnMotionFileCache *getInstance();

    /**
     * Deletes the singleton instance.
     */
    static void deleteInstance();

    /**
     * Returns the given document.
     */
    virtual const BonnMotionFile *getFile(const char *filename);
};

#endif

