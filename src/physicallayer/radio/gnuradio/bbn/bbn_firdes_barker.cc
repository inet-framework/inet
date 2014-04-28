/* -*- c++ -*- */
/*
 * Copyright 2005 Free Software Foundation, Inc.
 *
 * Copyright (c) 2006 BBN Technologies Corp.  All rights reserved.
 * Effort sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and the Department of the Interior National Business
 * Center under agreement number NBCHC050166.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * See also ISO 3309 [ISO-3309] or ITU-T V.42 [ITU-V42] for a formal
 * specification.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <bbn_firdes_barker.h>
#include <gnuradio/filter/firdes.h>

static int sinc(int numSamples, float period, float *result);


std::vector<float> bbn_firdes_barker(int sample_rate) {
  int total_taps = sample_rate * 11 * 3;
  int mid_point = total_taps / 2;
  std::vector<float> rrc(total_taps);
  float Barker[] = {1, -1, 1, 1, -1, 1, 1, 1, -1, -1, -1};
  std::vector<float> result(sample_rate);
  int i, j;
  int filter_period;
  float maxVal;
  float ff[total_taps];

  if(sample_rate > 11) {
    filter_period = sample_rate;
  } else {
    filter_period = 11;
  }

  sinc(total_taps, sample_rate, ff);

  for(i=0; i<sample_rate; ++i) {
    result[i] = 0;
  }

  /* Convolve expanded Barker with the sinc pulse */
  for(i=0; i<11; ++i) {
    for(j=0; j<sample_rate; ++j) {
      result[j] += ff[i*sample_rate + mid_point - j*11] * 
	Barker[i];
    }
  }

  /* Normalize result so maximum amplitude is 1. */
  maxVal = 0;
  for(i=0; i<sample_rate; ++i) {
    if(fabsf(result[i]) > maxVal) {
      maxVal = fabs(result[i]);
    }
  }
  
  if(maxVal != 0) {
    for(i=0; i<sample_rate; ++i) {
      result[i] /= maxVal;
    }
  }

  return result;
}

static int sinc(int numSamples, float period, float *result) {
  int i;
  float f;

  if(numSamples < 2) {
    return -1;
  }

  f = (float)(-numSamples / 2);
  for(i=0; i<numSamples; ++i) {
    if(f == 0) {
      result[i] = 1;
    } else {
      result[i] = period * sin(M_PI * f / period) / (f * M_PI);
    }
    f += 1.0;
  }

  return 0;
}
