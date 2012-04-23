//
// Copyright (C) 2012 Opensim Ltd.
// Author: Tamas Borbely
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

#ifndef IQUEUEACCESS_H_
#define IQUEUEACCESS_H_

#include "INETDefs.h"

/**
 * This class defines that interface of the queues that
 * algorithmic droppers use.
 */
class INET_API IQueueAccess
{
    public:
        virtual ~IQueueAccess() {};

        /**
         * Returns the number of frames in the queue.
         */
        virtual int getLength() const = 0;

        /**
         * Returns the number of bytes in the queue.
         */
        virtual int getByteLength() const = 0;
};

#endif
