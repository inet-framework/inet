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
// Ieee80211acFrameParts.h
//

#ifndef IEEE80211ACFRAMEPARTS_H_
#define IEEE80211ACFRAMEPARTS_H_

#include "Ieee80211acTypes.h"

/**
 *
 */
class Ieee80211acFrameLSTF {
public:
  // Ctors
  Ieee80211acFrameLSTF();
  ~Ieee80211acFrameLSTF();

  // Getters setters
  int getValue();
  void setValue(int value);

protected:
  // Fields
  int value;
};

/**
 *
 */
class Ieee80211acFrameLLTF {
public:
  // Ctors
  Ieee80211acFrameLLTF();
  ~Ieee80211acFrameLLTF();

  // Getters setters
  int getValue();
  void setValue(int value);

protected:
  // Fields
  int value;
};

/**
 *
 */
class Ieee80211acFrameLSIG {
public:
  // Ctors
  Ieee80211acFrameLSIG();
  ~Ieee80211acFrameLSIG();
};

/**
 *
 */
class Ieee80211acFrameVHTSIGA {
public:
  // Ctors
  Ieee80211acFrameVHTSIGA();
  ~Ieee80211acFrameVHTSIGA();

protected:
  // Fields
  int bw;
  bool reservedBit2;
  bool stbc;
  int groupId;
  int nsts;
  int partialAid;
};

/**
 *
 */
class Ieee80211acFrameVHTSTF {
public:
  // Ctors
  Ieee80211acFrameVHTSTF();
  ~Ieee80211acFrameVHTSTF();
};

/**
 *
 */
class Ieee80211acFrameVHTLTF {
public:
  // Ctors
  Ieee80211acFrameVHTLTF();
  ~Ieee80211acFrameVHTLTF();
};

/**
 *
 */
class Ieee80211acFrameVHTSIGB {
public:
  // Ctors
  Ieee80211acFrameVHTSIGB();
  ~Ieee80211acFrameVHTSIGB();
};

/**
 *
 */
class Ieee80211acFrameDATA {
public:
  // Ctors
  Ieee80211acFrameDATA();
  ~Ieee80211acFrameDATA();

  // Getters setters
  int getValue();
  void setValue(int value);

protected:
  // Fields
  int value;
};

#endif


