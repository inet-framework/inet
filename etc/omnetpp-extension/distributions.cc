//-------------------------------------------------------------------------------
//	distributions.cc --
//
//	This file implements classes for additional probability distributions
//	which are not implemented in OMNeT++.
//
//	Copyright (C) 2009 Kyeong Soo (Joseph) Kim
//-------------------------------------------------------------------------------


#include "distributions.h"


//FIXME cDynamicExpression to add function name to exceptions thrown from functions

void nedfunctions_dummy() {} //see util.cc

typedef cDynamicExpression::Value Value;  // abbreviation for local use

#define DEF(FUNCTION, SIGNATURE, CATEGORY, DESCRIPTION, BODY) \
	static Value FUNCTION(cComponent *context, Value argv[], int argc) {BODY} \
    Define_NED_Function2(FUNCTION, SIGNATURE, CATEGORY, DESCRIPTION);


//------------------------------------------------------------------------------
//	Distribution functions
//------------------------------------------------------------------------------

DEF(nedf_trunc_lognormal,
    "double trunc_lognormal(double m, double w, double min, double max, bool t?, int rng?)",
    "random/continuous",
    "Returns a random number from the truncated Lognormal distribution",
{
	bool t = argc==5 ? (bool)argv[4].bl : true;
    int rng = argc==6 ? (int)argv[5].dbl : 0;
    argv[0].dbl = trunc_lognormal(argv[0].dbl, argv[1].dbl, argv[2].dbl, argv[3].dbl, t, rng);
    return argv[0];
})

double trunc_lognormal(double m, double w, double min, double max, bool t, int rng)
{
	return 10;	// for test!
}


DEF(nedf_trunc_pareto,
    "quantity trunc_pareto(quantity k, double alpha, quantity m, int rng?)",
    "random/continuous",
    "Returns a random number from the truncated Pareto distribution",
{
    int rng = argc==4 ? (int)argv[3].dbl : 0;
    double argv2converted = UnitConversion::convertUnit(argv[2].dbl, argv[2].dblunit, argv[0].dblunit);
    argv[0].dbl = trunc_pareto(argv[0].dbl, argv[1].dbl, argv2converted, rng);
    return argv[0];
})

//DEF(nedf_trunc_pareto,
//    "double trunc_pareto(double k, double alpha, double m, int rng?)",
//    "random/continuous",
//    "Returns a random number from the truncated Pareto distribution",
//{
//    int rng = argc==4 ? (int)argv[3].dbl : 0;
////    double argv2converted = UnitConversion::convertUnit(argv[2].dbl, argv[2].dblunit, argv[0].dblunit);
//    argv[0].dbl = trunc_pareto(argv[0].dbl, argv[1].dbl, argv[2].dbl, rng);
//    return argv[0];
//})

double trunc_pareto(double k, double alpha, double m, int rng) {
	if ((k <= 0) || (alpha <= 0))
		throw cRuntimeError(
				"trunc_pareto(): parameters alpha and k must be positive (alpha=%g, k=%g)",
				alpha, k);

	return (k / pow((1.0 - (genk_dblrand(rng) * (1.0 - pow(k / m, alpha)))),
			1.0 / alpha));
}


//DEF(nedf_trunc_pareto_shifted,
//    "double trunc_pareto_shifted(double a, double b, double c, double min, double max, bool t?, long rng?)",
//    "random/continuous",
//    "Returns a random number from the truncated Pareto-shifted distribution",
//{
//	bool t = argc==6 ? (bool)argv[5].bl : true;
//    int rng = argc==7 ? (int)argv[6].dbl : 0;
////    double argv1converted = UnitConversion::convertUnit(argv[1].dbl, argv[1].dblunit, argv[1].dblunit);
////    double argv2converted = UnitConversion::convertUnit(argv[2].dbl, argv[2].dblunit, argv[1].dblunit);
//    argv[0].dbl = trunc_pareto_shifted(argv[0].dbl, argv[1].dbl, argv[2].dbl, argv[3].dbl, argv[4].dbl, t, rng);
//    return argv[0];
//})
//
//double trunc_pareto_shifted(double a, double b, double c, double min, double max, bool t, int rng)
//{
//	return 10;	// for test!
//}

