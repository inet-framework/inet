//-------------------------------------------------------------------------------
//  distrib.cc --
//
//  This file provides additional random distributions which are not
//  currently implemented in OMNeT++.
//
//  Copyright (C) 2009-2011 Kyeong Soo (Joseph) Kim
//-------------------------------------------------------------------------------

#include <omnetpp.h>

#define DEF(FUNCTION, SIGNATURE, CATEGORY, DESCRIPTION, BODY) \
    static cNEDValue FUNCTION(cComponent *context, cNEDValue argv[], int argc) {BODY} \
    Define_NED_Function2(FUNCTION, SIGNATURE, CATEGORY, DESCRIPTION);

// function signatures
double trunc_lognormal(double m, double w, double min, double max, bool t, int rng);
double trunc_pareto(double k, double alpha, double m, int rng);

///
/// Implementation of a truncated log normal distribution based on
/// simple procedural truncation at both ends. It will be updated
/// later based on the algorithm in [1].
///
/// @par References:
/// <ol>
/// <li>Robert, Christian P. (1995), Simulation of truncated normal variables,
///     Statistics and Computing 5, 121-125. [Online] Available:
///     <a href="http://arxiv.org/PS_cache/arxiv/pdf/0907/0907.4010v1.pdf">http://arxiv.org/PS_cache/arxiv/pdf/0907/0907.4010v1.pdf</a>
/// </li>
/// <li>Saralees Nadarajah and Samuel Kotz,&quot;R programs for truncated distributions,&quot;
///     Journal of Statistical Software, Vol. 16, Code Snippet 2, Aug. 2006. [Online] Available:
///     <a href="http://www.jstatsoft.org/v16/c02/paper">http://www.jstatsoft.org/v16/c02/paper</a>
/// </ol>
///
DEF(nedf_trunc_lognormal,
    "quantity trunc_lognormal(quantity m, quantity w, quantity min, quantity max, bool t?, long rng?)",
    "random/continuous",
    "Returns a random number from the truncated Lognormal distribution",
{
	bool t = argc==5 ? (bool)argv[4] : true;
    int rng = argc==6 ? (int)argv[5] : 0;
    double argv1converted = argv[1].doubleValueInUnit(argv[0].getUnit());
    double argv2converted = argv[2].doubleValueInUnit(argv[0].getUnit());
    double argv3converted = argv[3].doubleValueInUnit(argv[0].getUnit());
    return cNEDValue(trunc_lognormal((double)argv[0], argv1converted, argv2converted, argv3converted, t, rng), argv[0].getUnit());
})

SIM_API double trunc_lognormal(double m, double w, double min, double max, bool t, int rng)
{
	double res;

	do {
		res = lognormal(m, w, rng);
	} while ((res < min) || (res > max));

	return res;
}

///
/// Implementation of an upper-truncated Pareto distribution [1].
///
/// @par References:
/// <ol>
/// <li>Aban, Inmaculada B., Meerschaert, Mark M., and Panorska, Anna K.,
/// Parameter estimation for the truncated Pareto distribution,
/// Journal of the American Statistical Association,
/// vol. 101, no. 473, pp. 270-277, Mar. 2006.
/// </li>
/// </ol>
///
DEF(nedf_trunc_pareto,
    "quantity trunc_pareto(quantity k, double alpha, quantity m, long rng?)",
    "random/continuous",
    "Returns a random number from the truncated Pareto distribution",
{
    int rng = argc==4 ? (int)argv[3] : 0;
    double argv2converted = argv[2].doubleValueInUnit(argv[0].getUnit());
    return cNEDValue(trunc_pareto((double)argv[0], (double)argv[1], argv2converted, rng), argv[0].getUnit());
})

SIM_API double trunc_pareto(double k, double alpha, double m, int rng)
{
	if ((k <= 0) || (alpha <= 0))
		throw cRuntimeError(
				"trunc_pareto(): parameters alpha and k must be positive (alpha=%g, k=%g)",
				alpha, k);

	return (k / pow((1.0 - (genk_dblrand(rng) * (1.0 - pow(k / m, alpha)))),
			1.0 / alpha));
}
