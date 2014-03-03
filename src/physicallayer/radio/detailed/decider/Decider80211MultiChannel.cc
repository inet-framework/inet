/*
 * Decider80211MultiChannel.cpp
 *
 *  Created on: Mar 22, 2011
 *      Author: karl
 */

#include "IRadio.h"
#include "Decider80211MultiChannel.h"
#include "DeciderToPhyInterface.h"
#include "DeciderResult80211.h"
#include "Ieee80211Consts.h"

Decider80211MultiChannel::~Decider80211MultiChannel()
{
}

DeciderResult* Decider80211MultiChannel::createResult(const DetailedRadioFrame* frame) const {
    DeciderResult80211* result = static_cast<DeciderResult80211*>(Decider80211::createResult(frame));

    OldIRadio *radio = check_and_cast<OldIRadio *>(phy);
	if(result->isSignalCorrect() && frame->getChannel() != radio->getOldRadioChannel()) {
		EV << "Channel changed during reception. packet is lost!\n";
		DeciderResult80211* oldResult = result;

		result = new DeciderResult80211(false, oldResult->getBitrate(), oldResult->getSnr());
		delete oldResult;
	}

	return result;
}

void Decider80211MultiChannel::channelChanged(int newChannel) {
	assert(1 <= newChannel && newChannel <= 14);
	centerFrequency = CENTER_FREQUENCIES[newChannel];

	Decider80211::channelChanged(newChannel);
}
