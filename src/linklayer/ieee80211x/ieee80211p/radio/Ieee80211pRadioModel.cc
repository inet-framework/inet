//
// Copyright (C) 2014 Andrea Tino
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

//
// Ieee80211acRadio.cc
//

#include "Ieee80211acRadioModel.h"

#include "PhyControlInfo_m.h"
#include "Radio80211aControlInfo_m.h"

Define_Module(Ieee80211acRadio);

Ieee80211acRadioModel::Ieee80211acRadioModel() {

}

Ieee80211acRadioModel::~Ieee80211acRadioModel() {

}

void Ieee80211acRadioModel::initializeFrom(cModule *radioModule) {

}

double Ieee80211acRadioModel::calculateDuration(AirFrame *) {

}

bool Ieee80211acRadioModel::isReceivedCorrectly(AirFrame *airframe,
  const SnrList& receivedList) {

}

bool Ieee80211acRadioModel::haveTestFrame() {

}

double Ieee80211acRadioModel::calculateDurationTestFrame(AirFrame *airframe) {

}

double Ieee80211acRadioModel::getTestFrameError(double snirMin,
  double bitrate) {

}

int Ieee80211acRadioModel::getTestFrameSize() {

}

