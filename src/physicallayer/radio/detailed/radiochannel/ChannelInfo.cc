#include "ChannelInfo.h"

#include <iostream>
#include <assert.h>

const_simtime_t                   ChannelInfo::invalidSimTime(-1);
const ChannelInfo::const_iterator ChannelInfo::cConstItEnd(NULL, SIMTIME_ZERO, SIMTIME_ZERO);
const ChannelInfo::iterator       ChannelInfo::cItEnd(NULL, SIMTIME_ZERO, SIMTIME_ZERO);

void ChannelInfo::addAirFrame(DetailedRadioFrame* frame, simtime_t_cref startTime)
{
	assert(airFrameStarts.count(frame->getTreeId()) == 0);

	//calculate endTime of AirFrame
	simtime_t endTime = startTime + frame->getDuration();

	AirFrameMatrix::iterator pos = activeAirFrames.lower_bound(endTime);
	if(pos == activeAirFrames.end() || (activeAirFrames.key_comp()(endTime, pos->first))) {
	    // key does not exists
		pos = activeAirFrames.insert(pos, std::make_pair(endTime, AirFrameMatrix::mapped_type()));
	}
	pos->second.insert(std::make_pair(startTime, frame));

	//add to start time map
	airFrameStarts[frame->getTreeId()] = startTime;

	assert(!isChannelEmpty());
}

void ChannelInfo::addToInactives(DetailedRadioFrame* frame,
                                 simtime_t_cref startTime,
                                 simtime_t_cref endTime)
{
	// At first, check if some inactive AirFrames can be removed because the
	// AirFrame to in-activate was the last one they intersected with.
	checkAndCleanInterval(startTime, endTime);

	if(!canDiscardInterval(startTime, endTime)) {
		AirFrameMatrix::iterator pos = inactiveAirFrames.lower_bound(endTime);
		if(pos == inactiveAirFrames.end() || (inactiveAirFrames.key_comp()(endTime, pos->first))) {
		    // key does not exists
			pos = inactiveAirFrames.insert(pos, std::make_pair(endTime, AirFrameMatrix::mapped_type()));
		}
		pos->second.insert(std::make_pair(startTime, frame));
	}
	else {
		airFrameStarts.erase(frame->getTreeId());
		delete frame;
	}
}

simtime_t ChannelInfo::findEarliestInfoPoint(simtime_t_cref returnTimeIfEmpty /*= invalidSimTime*/) const
{
	c_min_start_time_fctr fctrMin;

	AirFrameMatrix::const_iterator itActivesMin   = std::min_element(activeAirFrames.begin()  , activeAirFrames.end()  , fctrMin);
	AirFrameMatrix::const_iterator itInactivesMin = std::min_element(inactiveAirFrames.begin(), inactiveAirFrames.end(), fctrMin);

	if (itActivesMin != activeAirFrames.end() && itInactivesMin != inactiveAirFrames.end()) {
		return std::min(itActivesMin->second.begin()->first, itInactivesMin->second.begin()->first);
	}
	if (itActivesMin != activeAirFrames.end())
		return itActivesMin->second.begin()->first;
	if (itInactivesMin != inactiveAirFrames.end())
		return itInactivesMin->second.begin()->first;

	return returnTimeIfEmpty;
}

simtime_t ChannelInfo::removeAirFrame(DetailedRadioFrame* frame, simtime_t_cref returnTimeIfEmpty /*= invalidSimTime*/)
{
	assert(airFrameStarts.count(frame->getTreeId()) > 0);

	//get start of AirFrame
	simtime_t_cref startTime = airFrameStarts[frame->getTreeId()];

	//calculate end time
	simtime_t      endTime   = startTime + frame->getDuration();

	//remove this AirFrame from active AirFrames
	deleteAirFrame(activeAirFrames, frame, startTime, endTime);

	//add to inactive AirFrames
	addToInactives(frame, startTime, endTime);


	// Now check, whether the earliest time-point we need to store information
	// for might have moved on in time, since an AirFrame has been deleted.
	if(isChannelEmpty()) {
		return returnTimeIfEmpty;
	}
	return findEarliestInfoPoint(returnTimeIfEmpty);
}

void ChannelInfo::assertNoIntersections() const {
	const bool bIsValidStartTime = recordStartTime >= SIMTIME_ZERO;

	for(AirFrameMatrix::const_iterator it1 = inactiveAirFrames.begin(); it1 != inactiveAirFrames.end(); ++it1)
	{
		simtime_t_cref e0 = it1->first;
		for(AirFrameMatrix::mapped_type::const_iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
		{
			simtime_t_cref s0 = it2->first;

			bool bIntersects = (bIsValidStartTime && recordStartTime <= e0);

			for(AirFrameMatrix::const_iterator it3 = activeAirFrames.begin();
				it3 != activeAirFrames.end() && !bIntersects; ++it3)
			{
				simtime_t_cref e1 = it3->first;
				for(AirFrameMatrix::mapped_type::const_iterator it4 = it3->second.begin();
					it4 != it3->second.end() && !bIntersects; ++it4)
				{
					simtime_t_cref s1 = it4->first;

					if(e0 >= s1 && s0 <= e1)
						bIntersects = true;
				}
			}
			assert(bIntersects);
		}
	}
}

void ChannelInfo::deleteAirFrame(AirFrameMatrix& airFrames,
                                 DetailedRadioFrame* frame,
                                 simtime_t_cref startTime, simtime_t_cref endTime)
{
	AirFrameMatrix::iterator listIt = airFrames.find(endTime);

	if (listIt != airFrames.end()) {
		AirFrameMatrix::mapped_type&          startTimeList = listIt->second;
		AirFrameMatrix::mapped_type::iterator itEnd         = startTimeList.upper_bound(startTime);

		for(AirFrameMatrix::mapped_type::iterator it = startTimeList.lower_bound(startTime); it != itEnd; ++it) {
			if(it->second == frame || it->second->getTreeId() == frame->getTreeId()) {
				startTimeList.erase(it);
				if(startTimeList.empty()) {
					airFrames.erase(listIt);
				}
				return;
			}
		}
	}
	assert(false);
}

bool ChannelInfo::canDiscardInterval(simtime_t_cref startTime,
									 simtime_t_cref endTime)
{
	assert(recordStartTime >= SIMTIME_ZERO || recordStartTime == invalidSimTime);

	// only if it ends before the point in time we started recording or if
	// we aren't recording at all and it does not intersect with any active one
	// anymore this AirFrame can be deleted
	return (recordStartTime > endTime || recordStartTime == invalidSimTime)
		   && !isIntersecting(activeAirFrames, startTime, endTime);
}

void ChannelInfo::checkAndCleanInterval(simtime_t_cref startTime,
                                        simtime_t_cref endTime)
{
	// get through inactive AirFrame which intersected with the passed interval
	iterator itInactiveFrames(&inactiveAirFrames, startTime, endTime);
	iterator itEnd = end();
	for (; itInactiveFrames != itEnd;) {
		DetailedRadioFrame* pInactiveFrame = itInactiveFrames->second;
		simtime_t      currentEnd     = itInactiveFrames->first + pInactiveFrame->getDuration();

		if(canDiscardInterval(itInactiveFrames->first, currentEnd)) {
			erase(itInactiveFrames++);

			airFrameStarts.erase(pInactiveFrame->getTreeId());
			delete pInactiveFrame;
			continue;
		}
		++itInactiveFrames;
	}
}

bool ChannelInfo::isIntersecting( const AirFrameMatrix& airFrames
                                , simtime_t_cref        from
                                , simtime_t_cref        to)
{
	const_iterator it(&airFrames, from, to);
	return (it != cConstItEnd);
}


