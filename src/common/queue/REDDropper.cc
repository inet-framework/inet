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

#include "inet/common/queue/REDDropper.h"
#include "inet/common/INETUtils.h"

namespace inet {

Define_Module(REDDropper);

REDDropper::~REDDropper()
{
    delete[] minths;
    delete[] maxths;
    delete[] maxps;
}

void REDDropper::initialize()
{
    AlgorithmicDropperBase::initialize();

    wq = par("wq");
    if (wq < 0.0 || wq > 1.0)
        throw cRuntimeError("Invalid value for wq parameter: %g", wq);

    minths = new double[numGates];
    maxths = new double[numGates];
    maxps = new double[numGates];

    cStringTokenizer minthTokens(par("minths"));
    cStringTokenizer maxthTokens(par("maxths"));
    cStringTokenizer maxpTokens(par("maxps"));
    for (int i = 0; i < numGates; ++i) {
        minths[i] = minthTokens.hasMoreTokens() ? utils::atod(minthTokens.nextToken()) :
            (i > 0 ? minths[i - 1] : 5.0);
        maxths[i] = maxthTokens.hasMoreTokens() ? utils::atod(maxthTokens.nextToken()) :
            (i > 0 ? maxths[i - 1] : 50.0);
        maxps[i] = maxpTokens.hasMoreTokens() ? utils::atod(maxpTokens.nextToken()) :
            (i > 0 ? maxps[i - 1] : 0.02);

        if (minths[i] < 0.0)
            throw cRuntimeError("minth parameter must not be negative");
        if (maxths[i] < 0.0)
            throw cRuntimeError("maxth parameter must not be negative");
        if (minths[i] >= maxths[i])
            throw cRuntimeError("minth must be smaller than maxth");
        if (maxps[i] < 0.0 || maxps[i] > 1.0)
            throw cRuntimeError("Invalid value for maxp parameter: %g", maxps[i]);
    }
}

bool REDDropper::shouldDrop(cPacket *packet)
{
    int i = packet->getArrivalGate()->getIndex();
    ASSERT(i >= 0 && i < numGates);
    double minth = minths[i];
    double maxth = maxths[i];
    double maxp = maxps[i];

    int queueLength = getLength();

    avg = (1 - wq) * avg + wq * queueLength;

    if (minth <= avg && avg < maxth) {
        double pb = maxp * (avg - minth) / (maxth - minth);
        //double pa = pb / (1-count*pb);
        if (dblrand() < pb) {
            EV << "Random early packet drop (avg queue len=" << avg << ", pa=" << pb << ")\n";
            return true;
        }
    }
    else if (avg >= maxth) {
        EV << "Avg queue len " << avg << " >= maxth, dropping packet.\n";
        return true;
    }
    else if (queueLength >= maxth) {    // maxth is also the "hard" limit
        EV << "Queue len " << queueLength << " >= maxth, dropping packet.\n";
        return true;
    }

    return false;
}

} // namespace inet

