//
// Copyright (C) 2014 Florian Meier <florian.meier@koalo.de>
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

#ifndef __RESULTRECORDERS_H
#define __RESULTRECORDERS_H

#include "cresultrecorder.h"
#include <map>
#include <string>

NAMESPACE_BEGIN

/**
 * Listener for counting the occurrences of signals with the same attribute
 */
class SIM_API GroupCountRecorder : public cResultRecorder
{
    protected:
        std::map<std::string,long> groupcounts;
    protected:
        virtual void collect(std::string val);
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b);
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, long l);
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, unsigned long l);
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, double d);
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const SimTime& v);
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const char *s);
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *obj);

    public:
        GroupCountRecorder() {}
        virtual void finish(cResultFilter *prev);
};

NAMESPACE_END

#endif
