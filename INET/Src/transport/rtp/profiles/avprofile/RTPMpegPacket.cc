/***************************************************************************
                          RTPMpegPacket.cc  -  description
                             -------------------
    begin                : Fri Dec 14 2001
    copyright            : (C) 2001 by Matthias Oppitz
    email                : Matthias.Oppitz@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** \file RTPMpegPacket.cc
 * This file contains the implementation of member functions of
 * the class RTPMpegPacket.
 */

#include "RTPMpegPacket.h"

Register_Class(RTPMpegPacket);

RTPMpegPacket::RTPMpegPacket(const char *name = NULL) : cPacket(name) {
    _mzb = 0;
    _two = 0;
    _temporalReference = 0;
    _activeN = 0;
    _newPictureHeader = 0;
    _sequenceHeaderPresent = 0;
    _beginningOfSlice = 0;
    _endOfSlice = 0;
    _pictureType = 0;
    _fbv = 0;
    _bfc = 0;
    _ffv = 0;
    _ffc = 0;
    // the standard header is 4 bytes long
    setLength(headerLength());
};


RTPMpegPacket::RTPMpegPacket(const RTPMpegPacket& packet) : cPacket(packet) {
    setName(packet.name());
    _mzb = packet._mzb;
    _two = packet._two;
    _temporalReference = packet._temporalReference;
    _activeN = packet._activeN;
    _newPictureHeader = packet._newPictureHeader;
    _sequenceHeaderPresent = packet._sequenceHeaderPresent;
    _beginningOfSlice = packet._beginningOfSlice;
    _endOfSlice = packet._endOfSlice;
    _pictureType = packet._pictureType;
    _fbv = packet._fbv;
    _bfc = packet._bfc;
    _ffv = packet._ffv;
    _ffc = packet._ffc;
};


RTPMpegPacket::~RTPMpegPacket() {
};


RTPMpegPacket& RTPMpegPacket::operator=(const RTPMpegPacket& packet) {
    cPacket::operator=(packet);
    return *this;
};


cObject *RTPMpegPacket::dup() const {
    return new RTPMpegPacket(*this);
};


const char *RTPMpegPacket::className() const {
    return "RTPMpegPacket";
};


int RTPMpegPacket::headerLength() {
    return 4;
};


int RTPMpegPacket::payloadLength() {
    return length() - headerLength();
};


int RTPMpegPacket::pictureType() {
    return _pictureType;
};


void RTPMpegPacket::setPictureType(int pictureType) {
    _pictureType = pictureType;
};