//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_BONNMOTIONFILECACHE_H
#define __INET_BONNMOTIONFILECACHE_H

#include <list>
#include <vector>

#include "inet/common/INETDefs.h"

namespace inet {

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
 */
class INET_API BonnMotionFileCache
{
  protected:
    std::map<std::string, BonnMotionFile> cache;
    void parseFile(const char *filename, BonnMotionFile& bmFile);

  public:
    const BonnMotionFile *getFile(const char *filename);
};

} // namespace inet

#endif

