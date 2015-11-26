//
// Copyright (C) 2011 OpenSim Ltd.
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
// @author Zoltan Bojthe
//

#include "inet/common/ResultFilters.h"

#include "inet/common/geometry/common/Coord.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/networklayer/contract/INetworkProtocolControlInfo.h"

namespace inet {

namespace utils {

namespace filters {

Register_ResultFilter("messageAge", MessageAgeFilter);

void MessageAgeFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object DETAILS_ARG)
{
    if (dynamic_cast<cMessage *>(object)) {
        cMessage *msg = (cMessage *)object;
        fire(this, t, t - msg->getCreationTime() DETAILS_ARG_NAME);
    }
}

Register_ResultFilter("messageTSAge", MessageTSAgeFilter);

void MessageTSAgeFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object DETAILS_ARG)
{
    if (dynamic_cast<cMessage *>(object)) {
        cMessage *msg = (cMessage *)object;
        fire(this, t, t - msg->getTimestamp() DETAILS_ARG_NAME);
    }
}

Register_ResultFilter("mobilityPos", MobilityPosFilter);

void MobilityPosFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object DETAILS_ARG)
{
    IMobility *module = dynamic_cast<IMobility *>(object);
    if (module) {
        Coord coord = module->getCurrentPosition();
        fire(this, t, &coord DETAILS_ARG_NAME);
    }
}

Register_ResultFilter("xCoord", XCoordFilter);

void XCoordFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object DETAILS_ARG)
{
    if (dynamic_cast<Coord *>(object))
        fire(this, t, ((Coord *)object)->x DETAILS_ARG_NAME);
}

Register_ResultFilter("yCoord", YCoordFilter);

void YCoordFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object DETAILS_ARG)
{
    if (dynamic_cast<Coord *>(object))
        fire(this, t, ((Coord *)object)->y DETAILS_ARG_NAME);
}

Register_ResultFilter("zCoord", ZCoordFilter);

void ZCoordFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object DETAILS_ARG)
{
    if (dynamic_cast<Coord *>(object))
        fire(this, t, ((Coord *)object)->z DETAILS_ARG_NAME);
}

Register_ResultFilter("sourceAddr", MessageSourceAddrFilter);

void MessageSourceAddrFilter::receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object DETAILS_ARG)
{
    if (dynamic_cast<cMessage *>(object)) {
        cMessage *msg = (cMessage *)object;

        INetworkProtocolControlInfo *ctrl = dynamic_cast<INetworkProtocolControlInfo *>(msg->getControlInfo());
        if (ctrl != nullptr) {
            fire(this, t, ctrl->getSourceAddress().str().c_str() DETAILS_ARG_NAME);
        }
    }
}

} // namespace filters

} // namespace utils

} // namespace inet

