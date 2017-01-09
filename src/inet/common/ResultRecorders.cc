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

#include "ResultRecorders.h"
#include <sstream>

namespace inet {

Register_ResultRecorder("groupCount", GroupCountRecorder);

void GroupCountRecorder::collect(std::string value) {
    groupcounts[value]++;
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, bool b, cObject *details) {
    collect(b ? "true" : "false");
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, long l, cObject *details) {
    std::stringstream s;
    s << l;
    collect(s.str());
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, unsigned long l, cObject *details) {
    std::stringstream s;
    s << l;
    collect(s.str());
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, double d, cObject *details) {
    std::stringstream s;
    s << d;
    collect(s.str());
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, const SimTime& v, cObject *details) {
    collect(v.str());
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, const char *s, cObject *details) {
    collect(s);
}

void GroupCountRecorder::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *obj, cObject *details) {
    collect(obj->getFullPath());
}

void GroupCountRecorder::finish(cResultFilter *prev) {
    opp_string_map attributes = getStatisticAttributes();

    for(auto & elem : groupcounts) {
        std::stringstream name;
        name << getResultName().c_str() << ":" << elem.first;
        getEnvir()->recordScalar(getComponent(), name.str().c_str(), elem.second, &attributes); // note: this is NaN if count==0
    }
}

Register_ResultRecorder("elapsedTime", ElapsedTimeRecorder);

ElapsedTimeRecorder::ElapsedTimeRecorder()
{
    startTime = clock();
}

void ElapsedTimeRecorder::finish(cResultFilter* prev)
{
    clock_t t = clock();
    double elapsedTime = (t - startTime) / (double)CLOCKS_PER_SEC;
    opp_string_map attributes = getStatisticAttributes();
    getEnvir()->recordScalar(getComponent(), getResultName().c_str(), elapsedTime, &attributes);
}

} // namespace inet

