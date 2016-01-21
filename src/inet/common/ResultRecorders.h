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

#ifndef __INET_RESULTRECORDERS_H
#define __INET_RESULTRECORDERS_H

#include <string>
#include "inet/common/INETDefs.h"

namespace inet {

/**
 * Listener for counting the occurrences of signals with the same attribute
 */
class INET_API GroupCountRecorder : public cResultRecorder
{
    protected:
        std::map<std::string,long> groupcounts;
    protected:
        virtual void collect(std::string val);
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b DETAILS_ARG) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, long l DETAILS_ARG) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, unsigned long l DETAILS_ARG) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, double d DETAILS_ARG) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const SimTime& v DETAILS_ARG) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, const char *s DETAILS_ARG) override;
        virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *obj DETAILS_ARG) override;

    public:
        GroupCountRecorder() {}
        virtual void finish(cResultFilter *prev) override;
};

} // namespace inet

#endif
