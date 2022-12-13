//
// Copyright (C) 2005 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/mobility/single/BonnMotionFileCache.h"

#include <fstream>
#include <sstream>

namespace inet {

const BonnMotionFile::Line *BonnMotionFile::getLine(int nodeId) const
{
    LineList::const_iterator it = lines.begin();
    for (int i = 0; i < nodeId && it != lines.end(); i++)
        it++;
    return (it == lines.end()) ? nullptr : &(*it);
}

const BonnMotionFile *BonnMotionFileCache::getFile(const char *filename)
{
    // if found, return it from cache
    auto it = cache.find(std::string(filename));
    if (it != cache.end())
        return &(it->second);

    // load and store in cache
    BonnMotionFile& bmFile = cache[filename];
    parseFile(filename, bmFile);
    return &bmFile;
}

void BonnMotionFileCache::parseFile(const char *filename, BonnMotionFile& bmFile)
{
    std::ifstream in(filename, std::ios::in);
    if (in.fail())
        throw cRuntimeError("Cannot open file '%s'", filename);

    std::string line;
    while (std::getline(in, line)) {
        bmFile.lines.push_back(BonnMotionFile::Line());
        BonnMotionFile::Line& vec = bmFile.lines.back();

        std::stringstream linestream(line);
        double d;
        while (linestream >> d)
            vec.push_back(d);
    }
    in.close();
}

} // namespace inet

