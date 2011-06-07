
// ***************************************************************************
//
// HttpTools Project
//// This file is a part of the HttpTools project. The project was created at
// Reykjavik University, the Laboratory for Dependable Secure Systems (LDSS).
// Its purpose is to create a set of OMNeT++ components to simulate browsing
// behaviour in a high-fidelity manner along with a highly configurable
// Web server component.
//
// Maintainer: Kristjan V. Jonsson (LDSS) kristjanvj@gmail.com
// Project home page: code.google.com/p/omnet-httptools
//
// ***************************************************************************
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License version 3
// as published by the Free Software Foundation.
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
// ***************************************************************************

#include "httptRandom.h"

string rdObject::typeStr()
{
	switch(m_type)
	{
		case dt_normal: return DISTR_NORMAL_STR;
		case dt_uniform: return DISTR_UNIFORM_STR;
		case dt_exponential: return DISTR_EXPONENTIAL_STR;
		case dt_histogram: return DISTR_HISTOGRAM_STR;
		default: return "UNKNOWN";
	}
}

rdNormal::rdNormal(double mean, double sd, bool nonNegative)
{
	m_type=dt_normal;
	m_mean = mean;
	m_sd = sd;
	m_nonNegative = nonNegative;
	m_min = 0.0;
	m_bMinLimit = false;
}

rdNormal::rdNormal( cXMLAttributeMap attributes )
{
	m_type=dt_normal;
	if ( !_hasKey(attributes,"mean") )
		throw "Undefined parameter for random distribution. Mean must be defined for a normal distribution";
	if ( !_hasKey(attributes,"sd") )
		throw "Undefined parameter for random distribution. sd must be defined for a normal distribution";
	m_mean = atof(attributes["mean"].c_str());
	m_sd = atof(attributes["sd"].c_str());
	m_bMinLimit = _hasKey(attributes,"min");
	if ( m_bMinLimit )
		m_min = atof(attributes["min"].c_str());
	if ( _hasKey(attributes,"nonNegative") )
		m_nonNegative = strcmp(attributes["nonNegative"].c_str(),"true")==0;
	else
		m_nonNegative = false;
}

double rdNormal::get()
{
	double retval=0.0;
	do
	{
		if ( m_nonNegative )
			retval = truncnormal(m_mean,m_sd);
		else
			retval = normal(m_mean,m_sd);
	}
	while(m_bMinLimit && retval<m_min);

	return retval;
}

rdUniform::rdUniform(double beginning, double end)
{
	m_type=dt_uniform;
	m_beginning = beginning;
	m_end = end;
}

rdUniform::rdUniform( cXMLAttributeMap attributes )
{
	m_type=dt_uniform;
	if ( !_hasKey(attributes,"beginning") )
		throw "Undefined parameter for random distribution. Beginning must be defined for a normal distribution";
	if ( !_hasKey(attributes,"end") )
		throw "Undefined parameter for random distribution. End must be defined for a normal distribution";
	m_beginning = atof(attributes["beginning"].c_str());
	m_end = atof(attributes["end"].c_str());
}

double rdUniform::get()
{
	return uniform(m_beginning,m_end);
}

rdExponential::rdExponential( double mean )
{
	m_type=dt_exponential;
	m_mean = mean;
	m_min = 0.0;
	m_max = 0.0;
	m_bMinLimit = false;
	m_bMaxLimit = false;
}

rdExponential::rdExponential( cXMLAttributeMap attributes )
{
	m_type=dt_exponential;

	if ( !_hasKey(attributes,"mean") )
		throw "Undefined parameter for random distribution. Mean must be defined for an exponential distribution";
	m_mean = atof(attributes["mean"].c_str());

	m_min=0.0;
	m_max=0.0;
	m_bMinLimit = _hasKey(attributes,"min");
	m_bMaxLimit = _hasKey(attributes,"max");
	if ( m_bMinLimit )
		m_min = atof(attributes["min"].c_str());
	if ( m_bMaxLimit )
		m_max = atof(attributes["max"].c_str());
}

double rdExponential::get()
{
	double val;
	do
		val = exponential(m_mean);
	while( ( m_bMinLimit && val < m_min ) || ( m_bMaxLimit && val > m_max ) );
	return val;
}

rdHistogram::rdHistogram(rdHistogramBins bins,bool zeroBased)
{
	m_zeroBased = zeroBased;
	m_bins = rdHistogramBins(bins);
	__normalizeBins();
}

rdHistogram::rdHistogram( cXMLAttributeMap attributes )
{
	m_type=dt_histogram;
	if ( !_hasKey(attributes,"bins") )
		throw "No bins specified for a histogram distribution";
	string binstr = attributes["bins"];
	if ( _hasKey(attributes,"zeroBased") )
		m_zeroBased = strcmp(attributes["zeroBased"].c_str(),"true")==0;
	__parseBinString(binstr);
	__normalizeBins();
}

double rdHistogram::get()
{
	int i;
	int count = m_bins.size();
	rdHistogramBin bin;
	double val = uniform(0,1);
	double cumsum = 0;
	int cumcount = 0;
	for( i=0; i<count; i++ )
	{
		// First select the bin
		bin = m_bins[i];
		cumsum += bin.sum;
		if ( cumsum >= val )
		{
			// Then choose from the elements in the bin
			double n = uniform(1,bin.count)+cumcount;
			if ( m_zeroBased) return n-1.0;
			else return n;
		}
		cumcount += bin.count; // Keep the running count for the elements
	}
	return -1.0; // Default return in case something weird happens
}

void rdHistogram::__parseBinString( string binstr )
{
	// The bins string is of the form [(count1,sum1);(count2,sum2);...;(countn,sumn)]
	binstr = trimLeft(binstr,"[");
	binstr = trimRight(binstr,"]");
	cStringTokenizer tokenizer = cStringTokenizer(binstr.c_str(),";");
	std::vector<string> res = tokenizer.asVector();
	std::vector<string>::iterator i;
	string curtuple, countstr, sumstr;
	int count;
	double sum;
	int pos;
	rdHistogramBin bin;
	for( i=res.begin(); i!=res.end(); i++ )
	{
		curtuple = (*i);
		curtuple = trimLeft(curtuple,"(");
		curtuple = trimRight(curtuple,")");
		pos = curtuple.find(',');
		if ( pos==-1 ) continue;  // Invalid tuple -- raise error here?
		countstr = curtuple.substr(0,pos);
		sumstr = curtuple.substr(pos+1,curtuple.size()-pos-1);
		sum = safeatof(sumstr.c_str(),0.0);
		count = safeatoi(countstr.c_str(),0);
		bin.count = count;
		bin.sum = sum;
		m_bins.push_back(bin);
	}
}

void rdHistogram::__normalizeBins()
{
	unsigned int i;
	double sum=0;
	for( i=0; i<m_bins.size(); i++ )
		sum += m_bins[i].sum;
	if ( sum==0 ) return;
	for( i=0; i<m_bins.size(); i++ )
		m_bins[i].sum = m_bins[i].sum/sum;
}

rdConstant::rdConstant(double value)
{
	m_type=dt_constant;
	m_value = value;
}

rdConstant::rdConstant( cXMLAttributeMap attributes )
{
	m_type=dt_constant;
	if ( !_hasKey(attributes,"value") )
		throw "No value specified";
	m_value = atof(attributes["value"].c_str());
}

double rdConstant::get()
{
	return m_value;
}

rdZipf::rdZipf(cXMLAttributeMap attributes)
{
	m_type=dt_zipf;

	if ( !_hasKey(attributes,"n") )
		throw "Undefined parameter for zipf distribution. n must be defined for an exponential distribution";
	if ( !_hasKey(attributes,"alpha") )
		throw "Undefined parameter for zipf distribution. alpha must be defined for an exponential distribution";

	int n;
	double alpha;
	bool baseZero=false;

	if ( _hasKey(attributes,"zeroBased") )
		baseZero = strcmp(attributes["zeroBased"].c_str(),"true")==0;

	try
	{
		atoi(attributes["n"].c_str());
	}
	catch(...)
	{
		n=1;
	}

	try
	{
		alpha = atof(attributes["alpha"].c_str());
	}
	catch(...)
	{
		alpha=1.0;
	}

	__initialize(n,alpha,baseZero);
}

rdZipf::rdZipf(int n,double alpha, bool baseZero)
{
	m_type=dt_zipf;
	__initialize(n,alpha,baseZero);
}

double rdZipf::get()
{
	double sum_prob=0;
	double z=uniform(0.0001,0.9999);

	int i;
	for (i=1; i<=m_number; i++)
	{
		sum_prob += m_c / pow((double) i, m_alpha);
		if (sum_prob >= z) break;
	}
	if ( m_baseZero ) return i-1;
	else return i;
}

string rdZipf::toString()
{
	ostringstream str;
	str << "Zipf probability distribution. n=" << m_number << ", alpha=" << m_alpha;
	if (m_baseZero)
		str << " Zero-based";
	str << endl;
	return str.str();
}

void rdZipf::__initialize(int n,double alpha, bool baseZero)
{
	m_number = n;
	m_alpha = alpha;
	m_baseZero = baseZero;
	__setup_c();
}

void rdZipf::__setup_c()
{
	m_c=0.0;
	for (int i=1; i<=m_number; i++)
		m_c += (1.0 / pow((double) i, m_alpha));
	m_c = 1.0 / m_c;
}

rdObject* rdObjectFactory::create( cXMLAttributeMap attributes )
{
	string typeName = attributes["type"];
	DISTR_TYPE dt;
	if ( typeName=="normal" ) dt=dt_normal;
	else if ( typeName=="uniform" ) dt=dt_uniform;
	else if ( typeName=="exponential") dt=dt_exponential;
	else if ( typeName=="histogram") dt=dt_histogram;
	else if ( typeName=="constant") dt=dt_constant;
	else if ( typeName=="zipf") dt=dt_zipf;
	else return NULL;

	switch( dt )
	{
		case dt_normal:
			return new rdNormal(attributes);
		case dt_uniform:
			return new rdUniform(attributes);
		case dt_exponential:
			return new rdExponential(attributes);
		case dt_histogram:
			return new rdHistogram(attributes);
		case dt_constant:
			return new rdConstant(attributes);
		case dt_zipf:
			return new rdZipf(attributes);
		default:
			return NULL;
	}
}

