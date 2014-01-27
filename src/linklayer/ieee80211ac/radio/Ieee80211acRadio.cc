//
// Copyright (C) 2014 Andrea Tino
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

//
// Ieee80211acRadio.cc
//

#include "Ieee80211acRadio.h"

#include "PhyControlInfo_m.h"
#include "Radio80211aControlInfo_m.h"

Define_Module(Ieee80211acRadio);

Ieee80211acRadio::Ieee80211acRadio() : Radio() {
}

Ieee80211acRadio::~Ieee80211acRadio() {
}

void Ieee80211acRadio::initialize() {
}

void Ieee80211acRadio::finish() {
  Radio::finish();
}

void Ieee80211acRadio::handleMessage(cMessage* msg) {
}

void Ieee80211acRadio::handleupperMsg(AirFrame* airframe) {
}

void Ieee80211acRadio::handleSelfMsg(cMessage* msg) {
}

void Ieee80211acRadio::handleCommand(int msgkind, cObject* ctrl) {
}

void Ieee80211acRadio::handleLowerMsgStart(AirFrame* airframe) {
}

void Ieee80211acRadio::handleLowerMsgEnd(AirFrame* airframe) {
}

void Ieee80211acRadio::bufferMsg(AirFrame* airframe) {
  Radio::bufferMsg(airframe);
}

AirFrame* Ieee80211acRadio::unbufferMsg(cMessage* msg) {
  return Radio::unbufferMsg(msg);
}

void Ieee80211acRadio::sendUp(AirFrame* airframe) {
}

void Ieee80211acRadio::sendDown(AirFrame* airframe) {
}

AirFrame* Ieee80211acRadio::encapsulatePacket(cPacket* pkt) {
  return Radio::encapsulatePacket(pkt);
}

bool Ieee80211acRadio::processAirFrame(AirFrame* airframe) {
  return Radio::processAirFrame(airframe);
}

