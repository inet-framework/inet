#ifndef VEC_T_H
#define	VEC_T_H

#include "util/vec.h"
#if HAVE_CONFIG_H
# include <config.h>
#endif

namespace vibello
{
enum { D=(GNP_DIMENSIONS) };

typedef util::Vec<double, D> vec_t;
}


#endif	/* VEC_T_H */

