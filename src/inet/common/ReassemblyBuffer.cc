//
// Copyright (C) 2004-2005 Andras Varga
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
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <stdlib.h>
#include <string.h>

#include "inet/common/INETDefs.h"

#include "inet/common/ReassemblyBuffer.h"

namespace inet {

ReassemblyBuffer::ReassemblyBuffer()
{
    main.beg = main.end = 0;
    main.islast = false;
    fragments = NULL;
}

ReassemblyBuffer::~ReassemblyBuffer()
{
    delete fragments;
}

bool ReassemblyBuffer::addFragment(ushort beg, ushort end, bool islast)
{
    merge(beg, end, islast);

    // do we have the complete datagram?
    return main.beg == 0 && main.islast;
}

void ReassemblyBuffer::merge(ushort beg, ushort end, bool islast)
{
    if (main.end == beg) {
        // most typical case (<95%): new fragment follows last one
        main.end = end;
        if (islast)
            main.islast = true;
        if (fragments)
            mergeFragments();
    }
    else if (main.beg == end) {
        // new fragment precedes what we already have
        main.beg = beg;
        if (fragments)
            mergeFragments();
    }
    else if (main.end < beg || main.beg > end) {
        // disjoint fragment, store it until another fragment fills in the gap
        if (!fragments)
            fragments = new RegionVector();
        Region r;
        r.beg = beg;
        r.end = end;
        r.islast = islast;
        fragments->push_back(r);
    }
    else {
        // overlapping is not possible;
        // fragment's range already contained in buffer (probably duplicate fragment)
    }
}

void ReassemblyBuffer::mergeFragments()
{
    RegionVector& frags = *fragments;

    bool oncemore;
    do {
        oncemore = false;
        for (RegionVector::iterator i = frags.begin(); i != frags.end(); ) {
            bool deleteit = false;
            Region& frag = *i;
            if (main.end == frag.beg) {
                main.end = frag.end;
                if (frag.islast)
                    main.islast = true;
                deleteit = true;
            }
            else if (main.beg == frag.end) {
                main.beg = frag.beg;
                deleteit = true;
            }
            else if (main.beg <= frag.beg && main.end >= frag.end) {
                // we already have this region (duplicate fragment), delete it
                deleteit = true;
            }

            if (deleteit) {
                // deletion is tricky because erase() invalidates iterator
                int pos = i - frags.begin();
                frags.erase(i);
                i = frags.begin() + pos;
                oncemore = true;
            }
            else {
                ++i;
            }
        }
    } while (oncemore);
}

} // namespace inet

