//
// Copyright (C) 2005 M. Bohge (bohge@tkn.tu-berlin.de), M. Renwanz
// Copyright (C) 2010 Zoltan Bojthe
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


#ifndef VOIPSTREAM_AUDIOOUTFILE_H
#define VOIPSTREAM_AUDIOOUTFILE_H

#define __STDC_CONSTANT_MACROS

#include "INETDefs.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
};

/**
 * Records audio into a file.
 */
class AudioOutFile
{
  public:
    AudioOutFile() : opened(false), audio_st(NULL), oc(NULL) {};
    ~AudioOutFile();

    void open(const char *resultFile, int sampleRate, short int sampleBits);
    void write(void *inbuf, int inbytes);
    bool close();
    bool isOpen() const { return opened; }

  protected:
    void addAudioStream(enum CodecID codec_id, int sampleRate, short int sampleBits);

  protected:
    bool opened;
    AVStream *audio_st;
    AVFormatContext *oc;
};


#endif // VOIPSTREAM_AUDIOOUTFILE_H
