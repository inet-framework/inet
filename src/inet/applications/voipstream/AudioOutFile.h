//
// Copyright (C) 2005 M. Bohge (bohge@tkn.tu-berlin.de), M. Renwanz
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_AUDIOOUTFILE_H
#define __INET_AUDIOOUTFILE_H

#define __STDC_CONSTANT_MACROS

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
};

// bugfix for g++ 11 compile error: taking address of temporary array at av_err2str()
#ifdef av_err2str
#undef av_err2str
#include <string>
av_always_inline std::string av_err2string(int errnum) {
    char str[AV_ERROR_MAX_STRING_SIZE];
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}
#define av_err2str(err) av_err2string(err).c_str()
#endif  // av_err2str

#include "inet/common/INETDefs.h"

namespace inet {

void inet_av_log(void *avcontext, int level, const char *format, va_list va);

/**
 * Records audio into a file.
 */
class INET_API AudioOutFile
{
  public:
    AudioOutFile() {}
    ~AudioOutFile();

    void open(const char *resultFile, int sampleRate, short int sampleBits);
    void write(void *inbuf, int inbytes);
    bool close();
    bool isOpen() const { return opened; }

  protected:
    bool opened = false;
    AVStream *audio_st = nullptr;
    AVFormatContext *oc = nullptr;
    AVCodecContext *codecCtx = nullptr;
};

} // namespace inet

#endif

