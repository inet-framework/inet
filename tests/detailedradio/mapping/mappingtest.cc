#include <Mapping.h>
#include <iostream>
#include <sstream>
#include <string>
//#include "Time.h"
#include "../testUtils/asserts.h"
#include "../testUtils/OmnetTestBase.h"
#include "FWMath.h"
#include "Decider802154Narrow.h"

void assertEqualSilent(std::string msg, double target, simtime_t_cref actual) {

	assertEqualSilent(msg, target, SIMTIME_DBL(actual));
}

void assertClose(std::string msg, double target, simtime_t_cref actual){
	assertClose(msg, target, SIMTIME_DBL(actual));
}

void assertClose(std::string msg, simtime_t_cref target, double actual){
	assertClose(msg, SIMTIME_DBL(target), actual);
}

void assertClose(std::string msg, Argument target, Argument actual){
	if(actual.isClose(target)){
		pass(msg);
	} else {
		fail(msg, target, actual);
	}
}

void checkIteratorRange(std::string msg, ConstMappingIterator& it,
					   bool hasNext, bool inRange, Argument arg){
	assertEqual(msg + ": hasNext() at " + toString(arg), hasNext, it.hasNext());
	assertEqual(msg + ": inRange() at " + toString(arg), inRange, it.inRange());
}

void checkIterator(std::string msg, ConstMappingIterator& it,
					   bool hasNext, bool inRange,
					   Argument arg, Argument nextArg,
					   double val){
	checkIteratorRange(msg, it, hasNext, inRange, arg);
	assertClose(msg + ": currentPos() at " + toString(arg), arg, it.getPosition());
	if(hasNext){
		try{
			assertClose(msg + ": nextPos() at " + toString(arg), nextArg, it.getNextPosition());
		}catch(NoNextIteratorException& e){
			assertFalse("HasNext should be false on NoNextException.", hasNext);
		}
	}
	assertClose(msg + ": getValue() at " + toString(arg), val, it.getValue());
}

void checkIterator(std::string msg, ConstMappingIterator& it,
				   bool hasNext, bool inRange,
				   Argument arg, Argument nextArg,
				   double val, ConstMapping& f) {

	checkIterator(msg, it, hasNext, inRange, arg, nextArg, val);
	assertClose(msg + ": Equal with function at " + toString(arg), f.getValue(it.getPosition()), it.getValue());
}

void checkIteratorHard(std::string msg, ConstMappingIterator& it,
					   bool hasNext, bool inRange,
					   Argument arg, Argument nextArg,
					   double val, ConstMapping& f){
	checkIteratorRange(msg, it, hasNext, inRange, arg);
	assertEqual(msg + ": currentPos() at " + toString(arg), arg, it.getPosition());
	if(hasNext){
		try{
			assertEqual(msg + ": nextPos() at " + toString(arg), nextArg, it.getNextPosition());
		}catch(NoNextIteratorException& e){
			assertFalse("HasNext should be false on NoNextException.", hasNext);
		}
	}
	assertClose(msg + ": getValue() at " + toString(arg), val, it.getValue());
	assertClose(msg + ": Equal with function at " + toString(arg), f.getValue(it.getPosition()), it.getValue());
}

void checkNext(std::string msg, ConstMappingIterator& it,
			   bool hasNext, bool inRange,
			   Argument arg, Argument nextArg,
			   double val, ConstMapping& f) {
	it.next();
	checkIterator(msg, it, hasNext, inRange, arg, nextArg, val, f);
}

void checkNextHard(std::string msg, ConstMappingIterator& it,
			   bool hasNext, bool inRange,
			   Argument arg, Argument nextArg,
			   double val, ConstMapping& f) {
	it.next();
	checkIteratorHard(msg, it, hasNext, inRange, arg, nextArg, val, f);
}

void assertMappingItEqual(std::string msg, ConstMappingIterator* target, ConstMappingIterator* actual){
	checkIterator(msg, *actual, target->hasNext(), target->inRange(), target->getPosition(), target->getNextPosition(), target->getValue());
}

void assertMappingEqual(std::string msg, ConstMapping* target, ConstMapping* actual){
	ConstMappingIterator* it1 = target->createConstIterator();
	ConstMappingIterator* it2 = actual->createConstIterator();
	do{
		assertMappingItEqual(msg, it1, it2);

		if(!it1->hasNext() || !it2->hasNext())
			break;

		it1->next();
		it2->next();
	}while(true);

	delete it1;
	delete it2;
}


template<class T>
void assertEqualNotSmaller(std::string msg, T& v1, T& v2){
	assertEqual(msg, v1, v2);
	assertFalse(msg, v1 < v2);
	assertFalse(msg, v2 < v1);
	DimensionSet v1Dims = v1.getDimensions();
	assertEqual(msg, 0, v1.compare(v2, &v1Dims));
}



class MappingTest:public SimpleTest {
private:
	/** @brief Copy constructor is not allowed.
	 */
	MappingTest(const MappingTest&);
	/** @brief Assignment operator is not allowed.
	 */
	MappingTest& operator=(const MappingTest&);

protected:
	std::map<Argument::mapped_type, std::map<simtime_t, Argument> > a;
	std::map<simtime_t, Argument> t;
	Dimension time;
	Dimension freq;
	Dimension channel;
	Dimension space;

	class ArgFactory{
	protected:
		Dimension time;
		Dimension d2;
		Dimension d3;

		DimensionSet set2;
		DimensionSet set3;
	public:
		ArgFactory(Dimension d2, Dimension d3):
			time(Dimension::time), d2(d2), d3(d3), set2(time, d2), set3(time, d2, d3) {}

		Argument operator()(simtime_t_cref t){
			return Argument(t);
		}

		Argument operator[](simtime_t_cref t){
			return Argument(MappingUtils::pre(t));
		}

		Argument operator()(Argument::mapped_type v2, simtime_t_cref t){
			Argument res(set2, t);
			res.setArgValue(d2, v2);
			return res;
		}

		Argument operator()(Argument::mapped_type v3, Argument::mapped_type v2, simtime_t_cref t){
			Argument res(set3, t);
			res.setArgValue(d2, v2);
			res.setArgValue(d3, v3);
			return res;
		}

		Argument operator()(Dimension other, Argument::mapped_type v2, simtime_t_cref t){
			Argument res(t);
			res.setArgValue(other, v2);
			return res;
		}
	};

	ArgFactory A;

	Mapping* createMappingBuffer;
public:
	MappingTest()
		: SimpleTest()
		, a()
		, t()
		, time(Dimension::time)
		, freq("frequency")
		, channel(freq)
		, space("space")
		, A(freq, space)
		, createMappingBuffer(0)
	{
		for(Argument::mapped_type i = 0.0; i <= 6.0; i+=0.25) {
			for(simtime_t j = SIMTIME_ZERO; j <= 6.0; j+=0.25) {
				a[i][j].setTime(j);
				a[i][j].setArgValue(freq, i);
			}
		}

		t = a[3.0];
	}

	virtual ~MappingTest() {
		if(createMappingBuffer)
			delete createMappingBuffer;
	}
protected:
	void testDimension() {

		Dimension d1("time");
		Dimension d2("frequency");

		assertEqual("Check time dimensions name.", "time", d1.getName());
		assertEqual("Check freq dimensions name.", "frequency", d2.getName());
		assertTrue("Time dimension should be smaller then frequency.", d1 < d2);
		assertFalse("Time dimension should not be equal with frequency.", d1 == d2);

		assertEqual("With \"time\" created dimension should be equal with Dimension::time()", Dimension::time, d1);

		Dimension d3("time");
		assertEqual("Check time2 dimensions name.", "time", d3.getName());
		assertTrue("Copy of time dimension should be equal with original.", d1 == d3);
		assertFalse("Copy of time dimension should not be smaller than original.", d1 < d3);

		Dimension d4("space");
		assertEqual("Check space dimensions name.", "space", d4.getName());
		assertFalse("Space dimension should not be smaller then frequency.", d4 < d2);

		DimensionSet dims;
		dims.addDimension(d2);
		DimensionSet::reverse_iterator it = dims.rbegin();
		assertEqual("first dimension should be freq.", d2, *it);
		it++;
		assertEqual("next dimension of freq should be freq.", d2, *it);

		dims.addDimension(d4);
		it = dims.rbegin();
		assertEqual("first dimension should be space.", d4, *it);
		assertEqual("next dimension of freq should be freq.", d2, *(++it));
		assertEqual("next dimension of space should be freq.", d2, *(++it));
	}

	void testArg() {
		Argument a1(10.2);
		DimensionSet a1Dims = a1.getDimensions();

		assertClose("Check initial time value of a1.", 10.2, a1.getTime());

		a1.setTime(-4.2);
		assertEqualSilent("Check time value of a1 after setTimeValue.", -4.2, a1.getTime());

		Argument a2(-4.2);
		DimensionSet a2Dims = a2.getDimensions();

		assertEqualNotSmaller("a1 and a2 should be equal.", a1, a2);

		a2.setTime(-4.3);
		assertTrue("a2 with smaller time should be smaller than a1", a2 < a1);
		assertTrue("a2 with smaller time should be compared smaller than a1", a2.compare(a1, &a1Dims) < 0);

		a2.setTime(0.0);
		assertTrue("a1 with smaller time should be smaller than a2", a1 < a2);
		assertTrue("a1 with smaller time should be compared smaller than a2", a1.compare(a2, &a1Dims) < 0);


		a1.setArgValue(freq, 2.5);
		a1Dims = a1.getDimensions();
		assertEqualSilent("time dimension should still have same value.", -4.2, a1.getTime());
		assertEqualSilent("Check frequency dimension value.", 2.5, a1.getArgValue(freq));

		a2.setTime(-4.2);

		//assertFalse("a1 and a2 with same time and implicit same freq should not be smaller.", a1 < a2);
		assertTrue("a1 and a2 with same time and implicit same freq should be same.", a1.isSamePosition(a2));
		assertFalse("a1 and a2 with same time and implicit same freq should not be equal.", a1 == a2);
		assertEqual("a1 and a2 with same time and implicit same freq should be compared same.", 0, a1.compare(a2, &a2Dims));

		a1.setArgValue(freq, -2.2);
		//assertFalse("a1 and a2 with same time and implicit same freq should still not be smaller.", a1 < a2);
		assertEqual("a1 and a2 with same time and implicit same freq should still be compared same.", 0, a1.compare(a2, &a2Dims));
		assertTrue("a1 and a2 with same time and implicit same freq should still be same.", a1.isSamePosition(a2));
		assertFalse("a1 and a2 with same time and implicit same freq should still not be equal.", a1 == a2);

		a2.setTime(-5);
		//assertFalse("a1 with bigger time and implicit equal freq should not be smaller.", a1 < a2);
		assertTrue("a1 with bigger time and implicit equal freq should be compared bigger.", a1.compare(a2, &a2Dims) > 0);
		assertFalse("a1 with bigger time and implicit equal freq should not be same.", a1.isSamePosition(a2));
		assertFalse("a1 with bigger time and implicit equal freq should not be equal.", a1 == a2);

		a1.setTime(-6);
		//assertTrue("a1 with smaller time and implicit equal freq should be smaller.", a1 < a2);
		assertTrue("a1 with smaller time and implicit equal freq should be compared smaller.", a1.compare(a2, &a2Dims) < 0);
		assertFalse("a1 with smaller time and implicit equal freq should not be same.", a1.isSamePosition(a2));
		assertFalse("a1 with smaller time and implicit equal freq should not be equal.", a1 == a2);

		a1.setTime(-4.2);
		a1.setArgValue(freq, 2.5);
		a2.setTime(-4.2);
		a2.setArgValue(freq, 2.5);
		a2Dims = a2.getDimensions();

		assertEqual("a1 and a2 with same time and freq should be equal.", a1, a2);
		assertEqual("a1 and a2 with same time and freq should be compared equal.", 0, a1.compare(a2, &a1Dims));

		a2.setTime(-4.3);
		assertTrue("a2 with smaller time and same freq should be smaller than a1", a2 < a1);
		assertTrue("a2 with smaller time and same freq should be compared smaller than a1", a2.compare(a1, &a1Dims) < 0);
		assertFalse("a2 with smaller time and same freq should not be equal with a1.", a1 == a2);

		a2.setTime(0.0);
		assertTrue("a1 with smaller time and same freq should be compared smaller than a2", a1.compare(a2, &a2Dims) < 0);
		assertTrue("a1 with smaller time and same freq should be smaller than a2", a1 < a2);
		assertFalse("a1 with smaller time and same freq should not be equal with a2.", a1 == a2);

		a2.setTime(-4.2);
		a2.setArgValue(freq, 2.0);
		assertTrue("a2 with smaller freq should be smaller than a1", a2.compare(a1, &a1Dims) < 0);
		assertTrue("a2 with smaller freq should be smaller than a1", a2 < a1);
		assertFalse("a2 with smaller freq should not be equal with a1.", a1 == a2);

		a2.setArgValue(freq, 3.0);
		assertTrue("a1 with smaller freq should be smaller than a2", a1.compare(a2, &a2Dims) < 0);
		assertTrue("a1 with smaller freq should be smaller than a2", a1 < a2);
		assertFalse("a1 with smaller freq should not be equal with a2.", a1 == a2);

		a2.setTime(-20.0);
		assertTrue("a1 with smaller freq should still be smaller than a2 with smaller time", a2.compare(a1, &a2Dims) > 0);
		assertTrue("a1 with smaller freq should still be smaller than a2 with smaller time", a1 < a2);
		assertFalse("a1 with smaller freq should not be equal with a2 with smaller time.", a1 == a2);

		a2.setTime(40.0);
		a2.setArgValue(freq, 2.2);
		assertTrue("a2 with smaller freq should still be smaller than a1 with smaller time", a1.compare(a2, &a2Dims) > 0);
		assertTrue("a2 with smaller freq should still be smaller than a1 with smaller time", a2 < a1);
		assertFalse("a2 with smaller freq should not be equal with a1 with smaller time.", a1 == a2);

		//displayPassed = false;
	}



	template<class F>
	void testSimpleFunction() {
		F f;
		Argument a3(3.11);

		assertEqualSilent("Function should be zero initially.", 0.0, f.getValue(a3));
		assertEqualSilent("Function should be zero initially (with []).", 0.0, f[a3]);

		f.setValue(a3, 5.0);
		assertEqualSilent("Check function value at a1.", 5.0, f[a3]);

		Argument a4(4.11);
		assertEqualSilent("Linear function should be constant after single value.", 5.0, f[a4]);
		Argument a2(2.01);
		assertEqualSilent("Linear function should beconstant before single value.", 5.0, f[a2]);

		f.setValue(a4, 10.0);
		assertEqualSilent("Check new set value at a2.", 10.0, f[a4]);
		assertEqualSilent("Check if still same value at a1.", 5.0, f[a3]);
		assertEqualSilent("Linear function should beconstant before first value.", 5.0, f[a2]);

		Argument a5(5.0);
		assertEqualSilent("Linear function should beconstant after last value.", 10.0, f[a5]);

		Argument halfFirst(3.61);
		assertClose("CHeck interpoaltion at half way.", 7.5, f[halfFirst]);
		assertClose("CHeck interpoaltion at quarter way.", 6.25, f[Argument(3.36)]);

		f.setValue(a5, 7.0);

		assertEqualSilent("Check new set value at a5.", 7.0, f[a5]);
		assertEqualSilent("Check if still same value at a1.", 5.0, f[a3]);
		assertEqualSilent("Linear function should beconstant before first value.", 5.0, f[a2]);

		Argument a6(6.0);
		assertEqualSilent("Linear function should beconstant after last value.", 7.0, f[a6]);

		Argument halfSecond(4.555);
		assertClose("CHeck interpoaltion at half way.", 8.5, f[halfSecond]);

		F emptyF;

		MappingIterator* it = emptyF.createIterator();
		checkIterator("Empty Iterator initial", *it, false, false, Argument(0.0), Argument(1.0), 0.0, emptyF);
		checkNext("First next of Empty iterator", *it, false, false, Argument(1.0), Argument(2.0), 0.0, emptyF);

		it->iterateTo(a6);
		checkIterator("Empty Iterator iterateTo()", *it, false, false, a6, Argument(7.0), 0.0, emptyF);

		it->jumpTo(a2);
		checkIterator("Empty Iterator jumpTo(back)", *it, false, false, a2, Argument(3.01), 0.0, emptyF);

		it->jumpTo(a5);
		checkIterator("Empty Iterator jumpTo(forward)", *it, false, false, a5, Argument(6.0), 0.0, emptyF);

		it->setValue(6.0);
		checkIterator("Set empty iterator", *it, false, true, a5, Argument(6.0), 6.0, emptyF);

		delete it;

		it = f.createIterator();
		checkIterator("Initial iterator", *it, true, true, a3, a4, 5.0, f);

		it->setValue(6.0);
		checkIterator("Set Initial iterator", *it, true, true, a3, a4, 6.0, f);

		it->iterateTo(halfFirst);
		checkIterator("First half iterator", *it, true, true, halfFirst, a4, 8.0, f);

		it->setValue(7.5);
		checkIterator("Set first half iterator", *it, true, true, halfFirst, a4, 7.5, f);

		checkNext("Second (after first half) iterator", *it, true, true, a4, a5, 10.0, f);
		checkNext("Last iterator", *it, false, true, a5, Argument(6.0), 7.0, f);

		Argument prevAll(0.1);
		it->jumpTo(prevAll);
		checkIterator("Prev all", *it, true, false, prevAll, a3, 6.0, f);

		it->jumpTo(Argument(0.2));
		checkIterator("Second prev all", *it, true, false, Argument(0.2), a3, 6.0, f);

		it->setValue(3.0);
		checkIterator("Set second prev all", *it, true, true, Argument(0.2), a3, 3.0, f);

		checkNext("Next of Prev all (first)", *it, true, true, a3, halfFirst, 6.0, f);

		it->iterateTo(a6);
		checkIterator("After all", *it, false, false, a6, Argument(7.0), 7.0, f);

		it->setValue(3.0);
		checkIterator("Set after all", *it, false, true, a6, Argument(7.0), 3.0, f);

		delete it;

		it = f.createIterator();
		checkIterator("Total 1:", *it, true, true, Argument(0.2), a3, 3.0, f);
		checkNext("Total 2:", *it, true, true, a3, halfFirst, 6.0, f);
		checkNext("Total 3:", *it, true, true, halfFirst, a4, 7.5, f);
		checkNext("Total 4:", *it, true, true, a4, a5, 10.0, f);
		checkNext("Total 5:", *it, true, true, a5, a6, 7.0, f);
		checkNext("Total 6:", *it, false, true, a6, Argument(a6.getTime() + 1.0), 3.0, f);
		checkNext("Total 7:", *it, false, false, Argument(a6.getTime() + 1.0), Argument(a6.getTime() + 2.0), 3.0, f);
		delete it;


		F f2;
		f2.setValue(Argument(3.86), 1.0);

		Mapping* res = f * f2;

		it = res->createIterator();
		checkIterator("f*1 1:", *it, true, true, Argument(0.2), a3, 3.0, *res);
		checkNext("f*1 2:", *it, true, true, a3, halfFirst, 6.0, *res);
		checkNext("f*1 3:", *it, true, true, halfFirst, Argument(3.86), 7.5, *res);
		checkNext("f*1 3.5:", *it, true, true, Argument(3.86), a4, 8.75, *res);
		checkNext("f*1 4:", *it, true, true, a4, a5, 10.0, *res);
		checkNext("f*1 5:", *it, true, true, a5, a6, 7.0, *res);
		checkNext("f*1 6:", *it, false, true, a6, Argument(a6.getTime() + 1.0), 3.0, *res);
		checkNext("f*1 7:", *it, false, false, Argument(a6.getTime() + 1.0), Argument(a6.getTime() + 2.0), 3.0, *res);
		delete it;
		delete res;

		F f2b;
		f2b.setValue(Argument(0.0), 2.0);

		res = f * f2b;

		it = res->createIterator();
		checkIterator("f*2 0:", *it,true, true, Argument(0.0), Argument(0.2), 6.0, *res);
		checkNext("f*2 1:", *it,true, true, Argument(0.2), a3, 6.0, *res);
		checkNext("f*2 2:", *it, true, true, a3, halfFirst, 12.0, *res);
		checkNext("f*2 3:", *it, true, true, halfFirst, a4, 15.0, *res);
		checkNext("f*2 4:", *it, true, true, a4, a5, 20.0, *res);
		checkNext("f*2 5:", *it, true, true, a5, a6, 14.0, *res);
		checkNext("f*2 6:", *it, false, true, a6, Argument(a6.getTime() + 1.0), 6.0, *res);
		checkNext("f*2 7:", *it, false, false, Argument(a6.getTime() + 1.0), Argument(a6.getTime() + 2.0), 6.0, *res);
		delete it;
		delete res;

		F f3(f);

		res = f * f3;

		it = res->createIterator();
		checkIterator("f^2 1:", *it, true, true, Argument(0.2), a3, 3.0 * 3.0, *res);
		checkNext("f^2 2:", *it, true, true, a3, halfFirst, 6.0 * 6.0, *res);
		checkNext("f^2 3:", *it, true, true, halfFirst, a4, 7.5 * 7.5, *res);
		checkNext("f^2 4:", *it, true, true, a4, a5, 10.0 * 10.0, *res);
		checkNext("f^2 5:", *it, true, true, a5, a6, 7.0 * 7.0, *res);
		checkNext("f^2 6:", *it, false, true, a6, Argument(a6.getTime() + 1.0), 3.0 * 3.0, *res);
		checkNext("f^2 7:", *it, false, false, Argument(a6.getTime() + 1.0), Argument(a6.getTime() + 2.0), 3.0 * 3.0, *res);
		delete it;
		delete res;

		F f4;

		f4.setValue(Argument(0.2), 1.0);
		f4.setValue(a6, 2.0);

		res = f3 * f4;
		double fak = 1.0 / (a6.getTime() - 0.2);
		it = res->createIterator();
		checkIterator("f*1-2 1:", *it, true, true, Argument(0.2), a3, 3.0 * 1.0, *res);
		checkNext("f*1-2 2:", *it, true, true, a3, halfFirst, 6.0 * (1.0 + (SIMTIME_DBL(a3.getTime()) - 0.2) * fak), *res);
		checkNext("f*1-2 3:", *it, true, true, halfFirst, a4, 7.5 * (1.0 + (SIMTIME_DBL(halfFirst.getTime()) - 0.2) * fak), *res);
		checkNext("f*1-2 4:", *it, true, true, a4, a5, 10.0 * (1.0 + (SIMTIME_DBL(a4.getTime()) - 0.2) * fak), *res);
		checkNext("f*1-2 5:", *it, true, true, a5, a6, 7.0 * (1.0 + (SIMTIME_DBL(a5.getTime()) - 0.2) * fak), *res);
		checkNext("f*1-2 6:", *it, false, true, a6, Argument(a6.getTime() + 1.0), 3.0 * (1.0 + (SIMTIME_DBL(a6.getTime()) - 0.2) * fak), *res);
		checkNext("f*1-2 7:", *it, false, false, Argument(a6.getTime() + 1.0), Argument(a6.getTime() + 2.0), 3.0 * (1.0 + (SIMTIME_DBL(a6.getTime()) - 0.2) * fak), *res);
		delete it;
		delete res;
	}

	void testMultiFunctionInfinity() {
		DimensionSet dimSet(Dimension::time);
		dimSet.addDimension(freq);

		for (int k=2; k<=16; k += 2) {
			std::cerr << "sum += n_choose_k(16, " << k << ") * exp(snr * (20.0 / " << k << " - 1.0)) = " << Decider802154Narrow::n_choose_k(16, k) << " * exp(snr * " << (20.0 / k - 1.0) << ")" << std::endl;
		}
		for (int k=3; k<=16; k += 2) {
			std::cerr << "sum -= n_choose_k(16, " << k << ") * exp(snr * (20.0 / " << k << " - 1.0)) = " << Decider802154Narrow::n_choose_k(16, k) << " * exp(snr * " << (20.0 / k - 1.0) << ")" << std::endl;
		}
		double snr = 1.0, sum_k = 0.0;
		double dSNRFct = snr * 20.0;
		register int k = 2;
		sum_k = 0.0; for (k=2; k <= 16; ++k) {
			sum_k += pow(-1.0, k) * Decider802154Narrow::n_choose_k(16, k) * exp(20.0 * snr * (1.0 / k - 1.0));
		}
		std::cerr << "BER(org) = " << ((8.0 / 15) * (1.0 / 16) * sum_k) << std::endl;
		sum_k = 0.0; for (k=2; k <= 16; k += 2) {
			sum_k += Decider802154Narrow::n_choose_k(16, k) * exp(dSNRFct * (1.0 / k - 1.0));
		}
		for (k=3; k <= 16; k += 2) {
			sum_k -= Decider802154Narrow::n_choose_k(16, k) * exp(dSNRFct * (1.0 / k - 1.0));
		}
		std::cerr << "BER(std) = " << ((8.0 / 15) * (1.0 / 16) * sum_k) << std::endl;
		sum_k = 0.0; for (k=2; k < 8; k += 2) {
			// k will be 2, 4, 6 (symmetric values: 14, 12, 10)
			sum_k += Decider802154Narrow::n_choose_k(16, k) * (exp(dSNRFct * (1.0 / k - 1.0)) + exp(dSNRFct * (1.0 / (16 - k) - 1.0)));
		}
		// for k = 8
		k = 8; sum_k += Decider802154Narrow::n_choose_k(16, k) * exp(dSNRFct * (1.0 / k - 1.0));
		k =16; sum_k += exp(dSNRFct * (1.0 / k - 1.0));
		for (k = 3; k < 8; k += 2) {
			// k will be 3, 5, 7 (symmetric values: 13, 11, 9)
			sum_k -= Decider802154Narrow::n_choose_k(16, k) * (exp(dSNRFct * (1.0 / k - 1.0)) + exp(dSNRFct * (1.0 / (16 - k) - 1.0)));
		}
		// for k = 15
		k = 15; sum_k -= Decider802154Narrow::n_choose_k(16, k) * exp(dSNRFct * (1.0 / k - 1.0));
		std::cerr << "BER(opt) = " << ((8.0 / 15) * (1.0 / 16) * sum_k) << std::endl;
		//ConstantSimpleConstMapping*	thermalNoiseS = new ConstantSimpleConstMapping(DimensionSet::timeDomain, FWMath::dBm2mW(-110.0));
		MultiDimMapping<Linear>     NoiseMap(dimSet);
		MultiDimMapping<Linear>     RecvPowerMap(dimSet);

		/*
		NoiseMap:
		Mapping domain: time, frequency(1)
		--------------+-------------------------------
		o\t           | 140000.07 140000.07 140000.12
		--------------+-------------------------------
		5865000000.00 |   -110.00    -95.73    -95.73
		5875000000.00 |   -110.00    -95.75    -95.75
		--------------+-------------------------------
		*/
		NoiseMap.setValue(A(5865000000.00, 140.000070), FWMath::dBm2mW(-110.00));
		NoiseMap.setValue(A(5865000000.00, 140.000071), FWMath::dBm2mW( -95.73));
		NoiseMap.setValue(A(5865000000.00, 140.000120), FWMath::dBm2mW( -95.73));
		NoiseMap.setValue(A(5875000000.00, 140.000070), FWMath::dBm2mW(-110.00));
		NoiseMap.setValue(A(5875000000.00, 140.000071), FWMath::dBm2mW( -95.73));
		NoiseMap.setValue(A(5875000000.00, 140.000120), FWMath::dBm2mW( -95.73));
		std::cerr << "NoiseMap is:" << std::endl << NoiseMap << std::endl;

		/*
		RecvPowerMap:
		Mapping domain: time, frequency(1)
		--------------+---------------------
		o\t           | 140000.07 140000.11
		--------------+---------------------
		5895000000.00 |    -71.28    -71.28
		5905000000.00 |    -71.29    -71.29
		--------------+---------------------
		*/
		RecvPowerMap.setValue(A(5895000000.00, 140.000070), FWMath::dBm2mW(-71.28));
		RecvPowerMap.setValue(A(5905000000.00, 140.000070), FWMath::dBm2mW(-71.29));
		RecvPowerMap.setValue(A(5895000000.00, 140.000111), FWMath::dBm2mW(-71.28));
		RecvPowerMap.setValue(A(5905000000.00, 140.000111), FWMath::dBm2mW(-71.29));
		std::cerr << "RecvPowerMap is:" << std::endl << RecvPowerMap << std::endl;

		/*
		snrMap
		Mapping domain: time, frequency(1)
		--------------+-----------------------------------------
		o\t           | 140000.07 140000.07 140000.11 140000.12
		--------------+-----------------------------------------
		5865000000.00 |     38.72     24.45               24.45
		5875000000.00 |     38.72     24.45               24.45
		5895000000.00 |     38.72               24.45
		5905000000.00 |     38.71               24.44
		--------------+-----------------------------------------
		*/
		Mapping* SnrMap = MappingUtils::divide( RecvPowerMap, NoiseMap, 0 );
		std::cout << "SnrMap ( RecvPowerMap / NoiseMap ) is:" << std::endl << *SnrMap << std::endl;
		delete SnrMap;

		ConstantSimpleConstMapping*  ThermalMap = new ConstantSimpleConstMapping(DimensionSet::timeDomain,  FWMath::dBm2mW(-110));
		Mapping*                     resultMap  = MappingUtils::createMapping(0.0, DimensionSet::timeDomain);
		Mapping*                     RecvMap    = MappingUtils::createMapping(DimensionSet::timeFreqDomain);
		Mapping*                     SignalMap  = MappingUtils::createMapping(DimensionSet::timeFreqDomain);
		Mapping*                     delMap     = NULL;

		ThermalMap->initializeArguments(A(140.000088221202));

		resultMap = MappingUtils::add(*(delMap = resultMap), *ThermalMap, 0);
		delete ThermalMap;
		delete delMap;

		RecvMap->setValue(A(5865000000.00,140.000097221202),FWMath::dBm2mW(-95.73));
		RecvMap->setValue(A(5875000000.00,140.000097221202),FWMath::dBm2mW(-95.73));
		RecvMap->setValue(A(5865000000.00,140.000123332313),FWMath::dBm2mW(-95.73));
		RecvMap->setValue(A(5875000000.00,140.000123332313),FWMath::dBm2mW(-95.73));

		resultMap = MappingUtils::add(*RecvMap, *(delMap = resultMap), 0);
		delete RecvMap;
		delete delMap;

		/*
		Noise Map
		Mapping domain: time, frequency(1)
		--------------+-------------------------------
		o\t           | 140000.09 140000.10 140000.12
		--------------+-------------------------------
		5865000000.00 |    -95.57    -95.57    -95.57
		5875000000.00 |    -95.57    -95.57    -95.57
		--------------+-------------------------------
		*/
		std::cerr<< "Noise Map" << std::endl << *resultMap << std::endl;

		SignalMap->setValue(A(5895000000.00,140.000068221202),FWMath::dBm2mW(-71.28));
		SignalMap->setValue(A(5895000000.00,140.000114332313),FWMath::dBm2mW(-71.28 ));
		SignalMap->setValue(A(5905000000.00,140.000068221202),FWMath::dBm2mW(-71.28));
		SignalMap->setValue(A(5905000000.00,140.000114332313),FWMath::dBm2mW(-71.28 ));

		/*
		Signal Map
		Mapping domain: time, frequency(1)
		--------------+---------------------
		o\t           | 140000.07 140000.11
		--------------+---------------------
		5895000000.00 |    -71.28    -71.28
		5905000000.00 |    -71.28    -71.28
		--------------+---------------------
		*/
		std::cerr << "Signal Map" << std::endl << *SignalMap << std::endl;

		resultMap = MappingUtils::divide(*SignalMap, *(delMap = resultMap), 0);
		delete SignalMap;
		delete delMap;

		/*
		SNR Map
		Mapping domain: time, frequency(1)
		--------------+---------------------------------------------------
		o\t           | 140000.07 140000.09 140000.10 140000.11 140000.12
		--------------+---------------------------------------------------
		5865000000.00 |               24.29     24.29               24.29
		5875000000.00 |               24.29     24.29               24.29
		5895000000.00 |       inf                           inf
		5905000000.00 |       inf                           inf
		--------------+---------------------------------------------------
		*/
		std::cerr << "SNR Map (Signal Map / Noise Map)" << std::endl << *resultMap << std::endl;

		std::cerr << "SNR Map findMin = " << FWMath::mW2dBm( MappingUtils::findMin(*resultMap) ) << std::endl;
		std::cerr << "SNR Map findMax = " << FWMath::mW2dBm( MappingUtils::findMax(*resultMap) ) << std::endl;

		delete resultMap;
	}

	void testMultiFunction() {
		//testMultiFunctionInfinity();
		DimensionSet dimSet(Dimension::time);
		dimSet.addDimension(channel);

		MultiDimMapping<Linear> f(dimSet);
		assertEqualSilent("Dimension of f", channel, f.getDimension());

		assertEqual("Function should be zero initially.", 0.0, f.getValue(a[2][2]));
		assertEqual("Function should be zero initially (with []).", 0.0, f[A(2, 2)]);

		f.setValue(A(2, 2), 5.0);
		for(double i = 1.0; i <= 4.0; i+=1.0) {
			for(double j = 1.0; j <= 4.0; j+=1.0) {
				assertEqual("Function should be constant 5 at "
							+ toString(i) + "," + toString(j), 5.0, f.getValue(A(i, j)));
			}
		}

		f.setValue(A(3, 2), 10.0);
		for(double i = 1.0; i <= 4.0; i+=0.5) {
			for(double j = 1.0; j <= 4.0; j+=0.5) {
				if(i <= 2.0)
					assertEqual("Function should be constant 5 before time 2 at "
								+ toString(i) + "," + toString(j), 5.0, f.getValue(A(i, j)));

				else if(i >= 3.0)
					assertEqual("Function should be constant 10 after time 3 at "
								+ toString(i) + "," + toString(j), 10.0, f.getValue(A(i, j)));

				else
					assertEqual("Function should be constant interpolated between time 2 and 3 at "
								+ toString(i) + "," + toString(j), 7.5, f.getValue(A(i, j)));
			}
		}

		MultiDimMapping<Linear> f2(dimSet);
		f2.setValue(A(2, 2), 5.0);
		f2.setValue(A(2, 3), 10.0);
		for(double i = 1.0; i <= 4.0; i+=0.5) {
			for(double j = 1.0; j <= 4.0; j+=0.5) {
				if(j <= 2.0)
					assertEqual("Function should be constant 5 before time 2 at "
								+ toString(i) + "," + toString(j), 5.0, f2.getValue(A(i, j)));

				else if(j >= 3.0)
					assertEqual("Function should be constant 10 after time 3 at "
								+ toString(i) + "," + toString(j), 10.0, f2.getValue(A(i, j)));

				else
					assertEqual("Function should be constant interpolated between time 2 and 3 at "
								+ toString(i) + "," + toString(j), 7.5, f2.getValue(A(i, j)));
			}
		}

		f2.setValue(A(3, 2), 50.0);
		f2.setValue(A(3, 3), 100.0);
		for(double i = 1.0; i <= 4.0; i+=0.5) {
			for(double j = 1.0; j <= 4.0; j+=0.5) {
				if(j <= 2.0){
					if(i <= 2.0)
						assertEqual("Function should be constant 5"
									+ toString(i) + "," + toString(j), 5.0, f2.getValue(A(i, j)));
					else if(i >= 3.0)
						assertEqual("Function should be constant 50"
									+ toString(i) + "," + toString(j), 50.0, f2.getValue(A(i, j)));
					else
						assertEqual("Function should be constant 27.5 "
									+ toString(i) + "," + toString(j), 27.5, f2.getValue(A(i, j)));

				}else if(j >= 3.0){
					if(i <= 2.0)
						assertEqual("Function should be constant 10"
									+ toString(i) + "," + toString(j), 10.0, f2.getValue(A(i, j)));
					else if(i >= 3.0)
						assertEqual("Function should be constant 100"
									+ toString(i) + "," + toString(j), 100.0, f2.getValue(A(i, j)));
					else
						assertEqual("Function should be constant 55 "
									+ toString(i) + "," + toString(j), 55.0, f2.getValue(A(i, j)));

				}else{
					if(i <= 2.0)
						assertEqual("Function should be constant 7.5"
									+ toString(i) + "," + toString(j), 7.5, f2.getValue(A(i, j)));
					else if(i >= 3.0)
						assertEqual("Function should be constant 75"
									+ toString(i) + "," + toString(j), 75.0, f2.getValue(A(i, j)));
					else
						assertEqual("Function should be constant 41.25 "
									+ toString(i) + "," + toString(j), 41.25, f2.getValue(A(i, j)));
				}
			}
		}

		MultiDimMapping<Linear> fEmpty(dimSet);
		MappingIterator* it = fEmpty.createIterator();

		checkIterator("Empty Iterator initial", *it, false, false, A(0, 0), A(1, 0), 0.0, fEmpty);
		checkNext("First next of Empty iterator", *it, false, false, A(1, 0), A(2, 0), 0.0, fEmpty);

		it->iterateTo(A(2, 3));
		checkIterator("Empty Iterator iterateTo()", *it, false, false, A(2, 3), A(3, 3), 0.0, fEmpty);

		it->jumpTo(A(1, 1));
		checkIterator("Empty Iterator jumpTo(back)", *it, false, false, A(1, 1), A(2, 1), 0.0, fEmpty);

		it->jumpTo(A(3, 4));
		checkIterator("Empty Iterator jumpTo(forward)", *it, false, false, A(3, 4), A(4, 4), 0.0, fEmpty);

		it->setValue(6.0);
		checkIterator("Set empty iterator", *it, false, true, A(3, 4), A(4, 4), 6.0, fEmpty);

		delete it;

		it = f2.createIterator();
		it->jumpTo(A(1.0, 1.0));
		for(double i = 1.0; i <= 4.0; i+=0.5) {
			for(double j = 1.0; j <= 4.0; j+=0.5) {
				it->iterateTo(A(i, j));
				if(j <= 2.0){
					if(i <= 2.0)
						assertEqual("Function should be constant 5"
									+ toString(i) + "," + toString(j), 5.0, it->getValue());
					else if(i >= 3.0)
						assertEqual("Function should be constant 50"
									+ toString(i) + "," + toString(j), 50.0, it->getValue());
					else
						assertEqual("Function should be constant 27.5 "
									+ toString(i) + "," + toString(j), 27.5, it->getValue());

				}else if(j >= 3.0){
					if(i <= 2.0)
						assertEqual("Function should be constant 10"
									+ toString(i) + "," + toString(j), 10.0, it->getValue());
					else if(i >= 3.0)
						assertEqual("Function should be constant 100"
									+ toString(i) + "," + toString(j), 100.0, it->getValue());
					else
						assertEqual("Function should be constant 55 "
									+ toString(i) + "," + toString(j), 55.0, it->getValue());

				}else{
					if(i <= 2.0)
						assertEqual("Function should be constant 7.5"
									+ toString(i) + "," + toString(j), 7.5, it->getValue());
					else if(i >= 3.0)
						assertEqual("Function should be constant 75"
									+ toString(i) + "," + toString(j), 75.0, it->getValue());
					else
						assertEqual("Function should be constant 41.25 "
									+ toString(i) + "," + toString(j), 41.25, it->getValue());
				}
			}
		}

		delete it;

		it = f2.createIterator();
		checkIterator("Check initial MultiDimFunction Iterator22", *it, true, true, A(2, 2), A(2, 3), 5.0, f2);
		it->setValue(6.0);
		checkIterator("Set Initial iterator22", *it, true, true, A(2, 2), A(2, 3), 6.0, f2);

		it->iterateTo(A(2, 2.5));
		checkIterator("First half iterator", *it, true, true, A(2, 2.5), A(2, 3), 8.0, f2);
		it->setValue(7.5);
		checkIterator("Set first half iterator", *it, true, true, A(2, 2.5), A(2, 3), 7.5, f2);

		checkNext("First next of MultiDimFunction23", *it, true, true, A(2, 3), A(3, 2), 10.0, f2);

		it->iterateTo(A(2.5, 1.5));
		checkIterator("second half iterator", *it, true, true, A(2.5, 1.5), A(3, 2), 28.0, f2);
		it->setValue(8.0);
		checkIterator("Set second half iterator", *it, true, true, A(2.5, 1.5), A(3, 2), 8.0, f2);

		it->iterateTo(A(2.75, 2.5));
		checkIterator("Third half iterator", *it, true, true, A(2.75, 2.5), A(3, 2), 41.5, f2);

		checkNext("Second next of MultiDimFunction32", *it, true, true, A(3, 2), A(3, 3), 50.0, f2);
		checkNext("Third next of MultiDimFunction33", *it, false, true, A(3, 3), A(4, 3), 100.0, f2);
		it->next();
		assertEqual("Iterator after end43: hasNext()", false, it->hasNext());
		assertEqual("Iterator after end43: inRange()", false, it->inRange());
		assertTrue("Iterator after end43: currentPos() not smaller last pos", !(it->getPosition() < A(3, 3)));
		assertClose("Iterator after end43: Equal with function", f2.getValue(it->getPosition()), it->getValue());


		it->jumpTo(A(1, 1));
		checkIterator("Prev all", *it, true, false, A(1, 1), A(2, 2), 6.0, f2);

		it->jumpTo(A(1.5, 1));
		checkIterator("Second prev all", *it, true, false, A(1.5, 1), A(2, 2), 6.0, f2);
		it->setValue(3.0);
		checkIterator("Set second prev all", *it, true, true, A(1.5, 1), A(2, 2), 3.0, f2);

		checkNext("Next of Prev all (first)", *it, true, true, A(2, 2), A(2, 2.5), 6.0, f2);

		it->iterateTo(A(3.5, 3.5));
		checkIterator("After all", *it, false, false, A(3.5, 3.5), A(4.5, 3.5), 100.0, f2);
		it->setValue(3.0);
		checkIterator("Set after all", *it, false, true, A(3.5, 3.5), A(4.5, 3.5), 3.0, f2);

		delete it;


		//Multiplikation

		f = MultiDimMapping<Linear>(f2);

		it = f.createIterator();
		checkIterator("Total 1:", *it, true, true, A(1.5, 1), A(2, 2), 3.0, f);
		checkNext("Total 2:", *it, true, true, A(2, 2), A(2, 2.5), 6.0, f);
		checkNext("Total 3:", *it, true, true, A(2, 2.5), A(2, 3), 7.5, f);
		checkNext("Total 4:", *it, true, true, A(2, 3), A(2.5, 1.5), 10.0, f);
		checkNext("Total 5:", *it, true, true, A(2.5, 1.5), A(3, 2), 8.0, f);
		checkNext("Total 6:", *it, true, true, A(3, 2), A(3, 3), 50.0, f);
		checkNext("Total 7:", *it, true, true, A(3, 3), A(3.5, 3.5), 100.0, f);
		checkNext("Total 8:", *it, false, true, A(3.5, 3.5), A(4.5, 3.5), 3.0, f);
		checkNext("Total 9:", *it, false, false, A(4.5, 3.5), A(5.5, 3.5), 3.0, f);
		delete it;



		TimeMapping<Linear> simpleConstTime;
		simpleConstTime.setValue(Argument(1.0), 1.0);

		Mapping* res = f * simpleConstTime;

		//displayPassed = false;

		it = res->createIterator();
		checkIterator("f*1 1:", *it, true, true, A(1.5, 1), A(2, 1), 3.0, *res);
		checkNext("f*1 1.5:", *it, true, true, A(2, 1), A(2, 2), 6.0, *res);
		checkNext("f*1 2:", *it, true, true, A(2, 2), A(2, 2.5), 6.0, *res);
		checkNext("f*1 3:", *it, true, true, A(2, 2.5), A(2, 3), 7.5, *res);
		checkNext("f*1 4:", *it, true, true, A(2, 3), A(2.5, 1), 10.0, *res);
		checkNext("f*1 4.5:", *it, true, true, A(2.5, 1), A(2.5, 1.5), 8.0, *res);
		checkNext("f*1 5:", *it, true, true, A(2.5, 1.5), A(3, 1), 8.0, *res);
		checkNext("f*1 5.5:", *it, true, true, A(3, 1), A(3, 2), 50.0, *res);
		checkNext("f*1 6:", *it, true, true, A(3, 2), A(3, 3), 50.0, *res);
		checkNext("f*1 7:", *it, true, true, A(3, 3), A(3.5, 1), 100.0, *res);
		checkNext("f*1 7.5:", *it, true, true, A(3.5, 1), A(3.5, 3.5), 3.0, *res);
		checkNext("f*1 8:", *it, false, true, A(3.5, 3.5), A(4.5, 3.5), 3.0, *res);
		checkNext("f*1 9:", *it, false, false, A(4.5, 3.5), A(5.5, 3.5), 3.0, *res);
		delete it;
		delete res;

		//displayPassed = false;


		simpleConstTime.setValue(Argument(1.0), 2.0);

		res = f * simpleConstTime;

		it = res->createIterator();
		checkIterator("f*2 1:", *it, true, true, A(1.5, 1), A(2, 1), 6.0, *res);
		checkNext("f*2 1.5:", *it, true, true, A(2, 1), A(2, 2), 12.0, *res);
		checkNext("f*2 2:", *it, true, true, A(2, 2), A(2, 2.5), 12.0, *res);
		checkNext("f*2 3:", *it, true, true, A(2, 2.5), A(2, 3), 15.0, *res);
		checkNext("f*2 4:", *it, true, true, A(2, 3), A(2.5, 1), 20.0, *res);
		checkNext("f*2 4.5:", *it, true, true, A(2.5, 1), A(2.5, 1.5), 16.0, *res);
		checkNext("f*2 5:", *it, true, true, A(2.5, 1.5), A(3, 1), 16.0, *res);
		checkNext("f*2 5.5:", *it, true, true, A(3, 1), A(3, 2), 100.0, *res);
		checkNext("f*2 6:", *it, true, true, A(3, 2), A(3, 3), 100.0, *res);
		checkNext("f*2 7:", *it, true, true, A(3, 3), A(3.5, 1), 200.0, *res);
		checkNext("f*2 7.5:", *it, true, true, A(3.5, 1), A(3.5, 3.5), 6.0, *res);
		checkNext("f*2 8:", *it, false, true, A(3.5, 3.5), A(4.5, 3.5), 6.0, *res);
		checkNext("f*2 9:", *it, false, false, A(4.5, 3.5), A(5.5, 3.5), 6.0, *res);
		delete it;
		delete res;

		assertEqualSilent("Dimension of f", channel, f.getDimension());
		MultiDimMapping<Linear> f3(f);
		assertEqualSilent("Dimension of f", channel, f.getDimension());
		assertEqualSilent("Dimension of f3", channel, f3.getDimension());

		res = f * f3;

		it = res->createIterator();
		checkIterator("f^2 1:", *it, true, true, A(1.5, 1), A(2, 2), 3.0 * 3.0, *res);
		checkNext("f^2 2:", *it, true, true, A(2, 2), A(2, 2.5), 6.0 * 6.0, *res);
		checkNext("f^2 3:", *it, true, true, A(2, 2.5), A(2, 3), 7.5 * 7.5, *res);
		checkNext("f^2 4:", *it, true, true, A(2, 3), A(2.5, 1.5), 10.0 * 10.0, *res);
		checkNext("f^2 5:", *it, true, true, A(2.5, 1.5), A(3, 2), 8.0 * 8.0, *res);
		checkNext("f^2 6:", *it, true, true, A(3, 2), A(3, 3), 50.0 * 50.0, *res);
		checkNext("f^2 7:", *it, true, true, A(3, 3), A(3.5, 3.5), 100.0 * 100.0, *res);
		checkNext("f^2 8:", *it, false, true, A(3.5, 3.5), A(4.5, 3.5), 3.0 * 3.0, *res);
		checkNext("f^2 9:", *it, false, false, A(4.5, 3.5), A(5.5, 3.5), 3.0 * 3.0, *res);
		delete it;
		delete res;


		TimeMapping<Linear> f4;

		f4.setValue(Argument(1.0), 1.0);
		f4.setValue(Argument(3.5), 1.5);
		f4.setValue(Argument(6.0), 2.0);

		assertEqualSilent("Dimension of f3", channel, f3.getDimension());
		res = f3 * f4;
		double fak = 1.0 / 5.0;
		it = res->createIterator();
		checkIterator("f*x 1:", *it, true, true, A(1.5, 1), A(1.5, 3.5), 3.0  * (1.0 + 0 * fak), *res);
		checkNext("f*x 1.1:", *it, true, true, A(1.5, 3.5), A(1.5, 6), 3.0  * (1.0 + 2.5 * fak), *res);
		checkNext("f*x 1.2:", *it, true, true, A(1.5, 6), A(2, 1), 3.0  * (1.0 + 5.0 * fak), *res);

		checkNext("f*x 1.3:", *it, true, true, A(2, 1), A(2, 2), 6.0  * (1.0 + 0.0 * fak), *res);
		checkNext("f*x 2:", *it, true, true, A(2, 2), A(2, 2.5), 6.0  * (1.0 + 1.0 * fak), *res);
		checkNext("f*x 3:", *it, true, true, A(2, 2.5), A(2, 3), 7.5  * (1.0 + 1.5 * fak), *res);
		checkNext("f*x 4:", *it, true, true, A(2, 3), A(2, 3.5), 10.0  * (1.0 + 2.0 * fak), *res);
		checkNext("f*x 4.1:", *it, true, true, A(2, 3.5), A(2, 6), 10.0  * (1.0 + 2.5 * fak), *res);
		checkNext("f*x 4.2:", *it, true, true, A(2, 6), A(2.5, 1), 10.0  * (1.0 + 5.0 * fak), *res);

		checkNext("f*x 4.3:", *it, true, true, A(2.5, 1), A(2.5, 1.5), 8.0  * (1.0 + 0.0 * fak), *res);
		checkNext("f*x 5:", *it, true, true, A(2.5, 1.5), A(2.5, 3.5), 8.0  * (1.0 + 0.5 * fak), *res);
		checkNext("f*x 5.1:", *it, true, true, A(2.5, 3.5), A(2.5, 6), 8.0  * (1.0 + 2.5 * fak), *res);
		checkNext("f*x 5.2:", *it, true, true, A(2.5, 6), A(3, 1), 8.0  * (1.0 + 5 * fak), *res);

		checkNext("f*x 5.3:", *it, true, true, A(3, 1), A(3, 2), 50.0  * (1.0 + 0.0 * fak), *res);
		checkNext("f*x 6:", *it, true, true, A(3, 2), A(3, 3), 50.0  * (1.0 + 1.0 * fak), *res);
		checkNext("f*x 7:", *it, true, true, A(3, 3), A(3, 3.5), 100.0  * (1.0 + 2.0 * fak), *res);
		checkNext("f*x 7.1:", *it, true, true, A(3, 3.5), A(3, 6), 100.0  * (1.0 + 2.5 * fak), *res);
		checkNext("f*x 7.2:", *it, true, true, A(3, 6), A(3.5, 1), 100.0  * (1.0 + 5.0 * fak), *res);

		checkNext("f*x 7.3:", *it, true, true, A(3.5, 1), A(3.5, 3.5), 3.0  * (1.0 + 0.0 * fak), *res);
		checkNext("f*x 8:", *it, true, true, A(3.5, 3.5), A(3.5, 6), 3.0  * (1.0 + 2.5 * fak), *res);
		checkNext("f*x 8.1:", *it, false, true, A(3.5, 6), A(4.5, 6), 3.0  * (1.0 + 5.0 * fak), *res);

		checkNext("f*x 9:", *it, false, false, A(4.5, 6), A(5.5, 6), 3.0  * (1.0 + 5.0 * fak), *res);
		delete it;
		delete res;

		MultiDimMapping<Linear> sig(dimSet);
		sig.setValue(A(1, 1), 1);
		sig.setValue(A(1, 1.5), 1.5);
		sig.setValue(A(1, 2), 2);
		sig.setValue(A(1, 2.5), 2.5);
		sig.setValue(A(2, 1), 3);
		sig.setValue(A(2, 1.5), 3.5);
		sig.setValue(A(2, 2), 4);
		sig.setValue(A(2, 2.5), 4.5);

		MultiDimMapping<Linear> att(dimSet);
		att.setValue(A(1, 1.5), 0.0);
		att.setValue(A(1, 1.75), 1.0);
		att.setValue(A(1, 2.0), 1.0);
		att.setValue(A(1, 2.25), 0.0);
		att.setValue(A(2, 1.0), 1.0);

		res = sig * att;

		for(double i = 1; i <= 2.0; i+=1.0) {
			for(double j = 1; j <= 2.5; j+=0.5) {
				if(i == 2) {
					assertEqualSilent("Not attenuated part." + toString(i) + "," + toString(j), j + 2, res->getValue(A(i, j)));
				} else {
					if(j <= 1.6 || j >=2.2)
						assertEqualSilent("To zero attenuated part" + toString(i) + "," + toString(j), 0.0, res->getValue(A(i, j)));
					else
						assertEqualSilent("Not attenuated part" + toString(i) + "," + toString(j), j, res->getValue(A(i, j)));
				}
			}
		}
		delete res;


	}


	void testSteps() {
		Mapping* t1 = MappingUtils::createMapping(DimensionSet(Dimension::time), Mapping::STEPS);

		assertEqual("Interpolation method STEPS - empty time A.", 0, t1->getValue(A(0)));

		ConstMappingIterator* it = t1->createConstIterator();
		checkIteratorHard("Interpolation method STEPS - empty time A - Iterator", *it, false, false, A(0), A(0), 0, *t1);
		delete it;

		t1->setValue(A(1), 2.0);

		assertEqual("Interpolation method STEPS - single before time A.", 2.0, t1->getValue(A(0)));
		assertEqual("Interpolation method STEPS - single at time A.", 2.0, t1->getValue(A(1)));
		assertEqual("Interpolation method STEPS - single after time A.", 2.0, t1->getValue(A(2)));

		it = t1->createConstIterator(A(0));
		checkIteratorHard("Interpolation method STEPS - single before time A - Iterator", *it, true, false, A(0), A[1], 2.0, *t1);
		checkNextHard("Interpolation method STEPS - single at pretime A - Iterator", *it, true, false, A[1], A(1), 2.0, *t1);
		checkNextHard("Interpolation method STEPS - single at time A - Iterator", *it, false, true, A(1), A[2], 2.0, *t1);
		checkNextHard("Interpolation method STEPS - single after time A - Iterator", *it, false, false, A[2], A(2), 2.0, *t1);
		delete it;

		t1->setValue(A(2), 3.0);

		assertEqual("Interpolation method STEPS - 2 before time A.", 2.0, t1->getValue(A(0)));
		assertEqual("Interpolation method STEPS - 2 at begin time A.", 2.0, t1->getValue(A(1)));
		assertEqual("Interpolation method STEPS - 2 between time A.", 2.0, t1->getValue(A(1.9)));
		assertEqual("Interpolation method STEPS - 2 at end time A.", 3.0, t1->getValue(A(2)));
		assertEqual("Interpolation method STEPS - 2 after time A.", 3.0, t1->getValue(A(2.5)));

		it = t1->createConstIterator(A(0));
		checkIteratorHard("Interpolation method STEPS - 2 before time A - Iterator.", *it, true, false, A(0), A[1], 2.0, *t1);
		checkNextHard("Interpolation method STEPS - 2 at prebegin time A - Iterator.", *it, true, false, A[1], A(1), 2.0, *t1);
		checkNextHard("Interpolation method STEPS - 2 at begin time A - Iterator.", *it, true, true, A(1), A[2], 2.0, *t1);
		checkNextHard("Interpolation method STEPS - 2 at preend time A - Iterator.", *it, true, true, A[2], A(2), 2.0, *t1);
		checkNextHard("Interpolation method STEPS - 2 at end time A - Iterator.", *it, false, true, A(2), A[3], 3.0, *t1);
		checkNextHard("Interpolation method STEPS - 2 after end time A - Iterator.", *it, false, false, A[3], A(3), 3.0, *t1);
		delete it;

		delete t1;

		t1 = MappingUtils::createMapping(1.0, DimensionSet(Dimension::time), Mapping::STEPS);

		assertEqual("Interpolation method STEPS - empty time B.", 1.0, t1->getValue(A(0)));

		it = t1->createConstIterator();
		checkIteratorHard("Interpolation method STEPS - empty time B - Iterator", *it, false, false, A(0), A(0), 1.0, *t1);
		delete it;

		t1->setValue(A(1), 2.0);

		assertEqual("Interpolation method STEPS - single before time B.", 1.0, t1->getValue(A(0)));
		assertEqual("Interpolation method STEPS - single at time B.", 2.0, t1->getValue(A(1)));
		assertEqual("Interpolation method STEPS - single after time B.", 2.0, t1->getValue(A(2)));

		it = t1->createConstIterator(A(0));
		checkIteratorHard("Interpolation method STEPS - single before time B - Iterator", *it, true, false, A(0), A[1], 1.0, *t1);
		checkNextHard("Interpolation method STEPS - single at pretime B - Iterator", *it, true, false, A[1], A(1), 1.0, *t1);
		checkNextHard("Interpolation method STEPS - single at time B - Iterator", *it, false, true, A(1), A[2], 2.0, *t1);
		checkNextHard("Interpolation method STEPS - single after time B - Iterator", *it, false, false, A[2], A(2), 2.0, *t1);
		delete it;

		t1->setValue(A(2), 3.0);

		assertEqual("Interpolation method STEPS - 2 before time B.", 1.0, t1->getValue(A(0)));
		assertEqual("Interpolation method STEPS - 2 at begin time B.", 2.0, t1->getValue(A(1)));
		assertEqual("Interpolation method STEPS - 2 between time B.", 2.0, t1->getValue(A(1.9)));
		assertEqual("Interpolation method STEPS - 2 at end time B.", 3.0, t1->getValue(A(2)));
		assertEqual("Interpolation method STEPS - 2 after time B.", 3.0, t1->getValue(A(2.5)));

		it = t1->createConstIterator(A(0));
		checkIteratorHard("Interpolation method STEPS - 2 before time B - Iterator.", *it, true, false, A(0), A[1], 1.0, *t1);
		checkNextHard("Interpolation method STEPS - 2 at prebegin time B - Iterator.", *it, true, false, A[1], A(1), 1.0, *t1);
		checkNextHard("Interpolation method STEPS - 2 at begin time B - Iterator.", *it, true, true, A(1), A[2], 2.0, *t1);
		checkNextHard("Interpolation method STEPS - 2 at preend time B - Iterator.", *it, true, true, A[2], A(2), 2.0, *t1);
		checkNextHard("Interpolation method STEPS - 2 at end time B - Iterator.", *it, false, true, A(2), A[3], 3.0, *t1);
		checkNextHard("Interpolation method STEPS - 2 after end time B - Iterator.", *it, false, false, A[3], A(3), 3.0, *t1);
		delete it;

		delete t1;

		Mapping* m1 = MappingUtils::createMapping(DimensionSet(time, freq), Mapping::STEPS);

		assertEqual("Interpolation method STEPS - empty time-freq A.", 0.0, m1->getValue(A(0, 0)));

		m1->setValue(A(1, 1), 2.0);

		assertEqual("Interpolation method STEPS - single before time at freq A.", 2.0, m1->getValue(A(1, 0)));
		assertEqual("Interpolation method STEPS - single at time at freq A.", 2.0, m1->getValue(A(1, 1)));
		assertEqual("Interpolation method STEPS - single after time at freq A.", 2.0, m1->getValue(A(1, 2)));

		assertEqual("Interpolation method STEPS - single before time before freq A.", 2.0, m1->getValue(A(0, 0)));
		assertEqual("Interpolation method STEPS - single at time before freq A.", 2.0, m1->getValue(A(0, 1)));
		assertEqual("Interpolation method STEPS - single after time before freq A.", 2.0, m1->getValue(A(0, 2)));

		assertEqual("Interpolation method STEPS - single before time after freq A.", 2.0, m1->getValue(A(2, 0)));
		assertEqual("Interpolation method STEPS - single at time after freq A.", 2.0, m1->getValue(A(2, 1)));
		assertEqual("Interpolation method STEPS - single after time after freq A.", 2.0, m1->getValue(A(2, 2)));

		m1->setValue(A(1, 2), 3.0);

		assertEqual("Interpolation method STEPS - 1,2 before time at freq A.", 2.0, m1->getValue(A(1, 0)));
		assertEqual("Interpolation method STEPS - 1,2 at time begin at freq A.", 2.0, m1->getValue(A(1, 1)));
		assertEqual("Interpolation method STEPS - 1,2 between time at freq A.", 2.0, m1->getValue(A(1, 1.9)));
		assertEqual("Interpolation method STEPS - 1,2 at time end at freq A.", 3.0, m1->getValue(A(1, 2)));
		assertEqual("Interpolation method STEPS - 1,2 after time end at freq A.", 3.0, m1->getValue(A(1, 3)));

		assertEqual("Interpolation method STEPS - 1,2 before time before freq A.", 2.0, m1->getValue(A(0, 0)));
		assertEqual("Interpolation method STEPS - 1,2 at time begin before freq A.", 2.0, m1->getValue(A(0, 1)));
		assertEqual("Interpolation method STEPS - 1,2 between time before freq A.", 2.0, m1->getValue(A(0, 1.9)));
		assertEqual("Interpolation method STEPS - 1,2 at time end before freq A.", 3.0, m1->getValue(A(0, 2)));
		assertEqual("Interpolation method STEPS - 1,2 after time end before freq A.", 3.0, m1->getValue(A(0, 3)));

		assertEqual("Interpolation method STEPS - 1,2 before time after freq A.", 2.0, m1->getValue(A(2, 0)));
		assertEqual("Interpolation method STEPS - 1,2 at time begin after freq A.", 2.0, m1->getValue(A(2, 1)));
		assertEqual("Interpolation method STEPS - 1,2 between time after freq A.", 2.0, m1->getValue(A(2, 1.9)));
		assertEqual("Interpolation method STEPS - 1,2 at time end after freq A.", 3.0, m1->getValue(A(2, 2)));
		assertEqual("Interpolation method STEPS - 1,2 after time end after freq A.", 3.0, m1->getValue(A(2, 3)));

		m1->setValue(A(2, 2), 4.0);

		assertEqual("Interpolation method STEPS - 2,2 before time at freq begin A.", 2.0, m1->getValue(A(1, 0)));
		assertEqual("Interpolation method STEPS - 2,2 at time begin at freq begin A.", 2.0, m1->getValue(A(1, 1)));
		assertEqual("Interpolation method STEPS - 2,2 between time at freq begin A.", 2.0, m1->getValue(A(1, 1.9)));
		assertEqual("Interpolation method STEPS - 2,2 at time end at freq begin A.", 3.0, m1->getValue(A(1, 2)));
		assertEqual("Interpolation method STEPS - 2,2 after time end at freq begin A.", 3.0, m1->getValue(A(1, 3)));

		assertEqual("Interpolation method STEPS - 2,2 before time before freq A.", 2.0, m1->getValue(A(0, 0)));
		assertEqual("Interpolation method STEPS - 2,2 at time begin before freq A.", 2.0, m1->getValue(A(0, 1)));
		assertEqual("Interpolation method STEPS - 2,2 between time before freq A.", 2.0, m1->getValue(A(0, 1.9)));
		assertEqual("Interpolation method STEPS - 2,2 at time end before freq A.", 3.0, m1->getValue(A(0, 2)));
		assertEqual("Interpolation method STEPS - 2,2 after time end before freq A.", 3.0, m1->getValue(A(0, 3)));

		assertEqual("Interpolation method STEPS - 2,2 before time between freq A.", 2.0, m1->getValue(A(1.9, 0)));
		assertEqual("Interpolation method STEPS - 2,2 at time begin between freq A.", 2.0, m1->getValue(A(1.9, 1)));
		assertEqual("Interpolation method STEPS - 2,2 between time between freq A.", 2.0, m1->getValue(A(1.9, 1.9)));
		assertEqual("Interpolation method STEPS - 2,2 at time end between freq A.", 3.0, m1->getValue(A(1.9, 2)));
		assertEqual("Interpolation method STEPS - 2,2 after time end between freq A.", 3.0, m1->getValue(A(1.9, 3)));

		assertEqual("Interpolation method STEPS - 2,2 before time at freq end A.", 4.0, m1->getValue(A(2, 1)));
		assertEqual("Interpolation method STEPS - 2,2 at time at freq end A.", 4.0, m1->getValue(A(2, 2)));
		assertEqual("Interpolation method STEPS - 2,2 after time end at freq end A.", 4.0, m1->getValue(A(2, 3)));

		assertEqual("Interpolation method STEPS - 2,2 before time after freq end A.", 4.0, m1->getValue(A(3, 1)));
		assertEqual("Interpolation method STEPS - 2,2 at time after freq end A.", 4.0, m1->getValue(A(3, 2)));
		assertEqual("Interpolation method STEPS - 2,2 after time end after freq end A.", 4.0, m1->getValue(A(3, 3)));

		delete m1;

		m1 = MappingUtils::createMapping(1.0, DimensionSet(time, freq), Mapping::STEPS);

		assertEqual("Interpolation method STEPS - empty time-freq B.", 1.0, m1->getValue(A(0, 0)));

		m1->setValue(A(1, 1), 2.0);

		assertEqual("Interpolation method STEPS - single before time at freq B.", 1.0, m1->getValue(A(1, 0)));
		assertEqual("Interpolation method STEPS - single at time at freq B.", 2.0, m1->getValue(A(1, 1)));
		assertEqual("Interpolation method STEPS - single after time at freq B.", 2.0, m1->getValue(A(1, 2)));

		assertEqual("Interpolation method STEPS - single before time before freq B.", 1.0, m1->getValue(A(0, 0)));
		assertEqual("Interpolation method STEPS - single at time before freq B.", 1.0, m1->getValue(A(0, 1)));
		assertEqual("Interpolation method STEPS - single after time before freq B.", 1.0, m1->getValue(A(0, 2)));

		assertEqual("Interpolation method STEPS - single before time after freq B.", 1.0, m1->getValue(A(2, 0)));
		assertEqual("Interpolation method STEPS - single at time after freq B.", 2.0, m1->getValue(A(2, 1)));
		assertEqual("Interpolation method STEPS - single after time after freq B.", 2.0, m1->getValue(A(2, 2)));

		m1->setValue(A(1, 2), 3.0);

		assertEqual("Interpolation method STEPS - 1,2 before time at freq B.", 1.0, m1->getValue(A(1, 0)));
		assertEqual("Interpolation method STEPS - 1,2 at time begin at freq B.", 2.0, m1->getValue(A(1, 1)));
		assertEqual("Interpolation method STEPS - 1,2 between time at freq B.", 2.0, m1->getValue(A(1, 1.9)));
		assertEqual("Interpolation method STEPS - 1,2 at time end at freq B.", 3.0, m1->getValue(A(1, 2)));
		assertEqual("Interpolation method STEPS - 1,2 after time end at freq B.", 3.0, m1->getValue(A(1, 3)));

		assertEqual("Interpolation method STEPS - 1,2 before time before freq B.", 1.0, m1->getValue(A(0, 0)));
		assertEqual("Interpolation method STEPS - 1,2 at time begin before freq B.", 1.0, m1->getValue(A(0, 1)));
		assertEqual("Interpolation method STEPS - 1,2 between time before freq B.", 1.0, m1->getValue(A(0, 1.9)));
		assertEqual("Interpolation method STEPS - 1,2 at time end before freq B.", 1.0, m1->getValue(A(0, 2)));
		assertEqual("Interpolation method STEPS - 1,2 after time end before freq B.", 1.0, m1->getValue(A(0, 3)));

		assertEqual("Interpolation method STEPS - 1,2 before time after freq B.", 1.0, m1->getValue(A(2, 0)));
		assertEqual("Interpolation method STEPS - 1,2 at time begin after freq B.", 2.0, m1->getValue(A(2, 1)));
		assertEqual("Interpolation method STEPS - 1,2 between time after freq B.", 2.0, m1->getValue(A(2, 1.9)));
		assertEqual("Interpolation method STEPS - 1,2 at time end after freq B.", 3.0, m1->getValue(A(2, 2)));
		assertEqual("Interpolation method STEPS - 1,2 after time end after freq B.", 3.0, m1->getValue(A(2, 3)));

		m1->setValue(A(2, 2), 4.0);

		assertEqual("Interpolation method STEPS - 2,2 before time at freq begin B.", 1.0, m1->getValue(A(1, 0)));
		assertEqual("Interpolation method STEPS - 2,2 at time begin at freq begin B.", 2.0, m1->getValue(A(1, 1)));
		assertEqual("Interpolation method STEPS - 2,2 between time at freq begin B.", 2.0, m1->getValue(A(1, 1.9)));
		assertEqual("Interpolation method STEPS - 2,2 at time end at freq begin B.", 3.0, m1->getValue(A(1, 2)));
		assertEqual("Interpolation method STEPS - 2,2 after time end at freq begin B.", 3.0, m1->getValue(A(1, 3)));

		assertEqual("Interpolation method STEPS - 2,2 before time before freq B.", 1.0, m1->getValue(A(0, 0)));
		assertEqual("Interpolation method STEPS - 2,2 at time begin before freq B.", 1.0, m1->getValue(A(0, 1)));
		assertEqual("Interpolation method STEPS - 2,2 between time before freq B.", 1.0, m1->getValue(A(0, 1.9)));
		assertEqual("Interpolation method STEPS - 2,2 at time end before freq B.", 1.0, m1->getValue(A(0, 2)));
		assertEqual("Interpolation method STEPS - 2,2 after time end before freq B.", 1.0, m1->getValue(A(0, 3)));

		assertEqual("Interpolation method STEPS - 2,2 before time between freq B.", 1.0, m1->getValue(A(1.9, 0)));
		assertEqual("Interpolation method STEPS - 2,2 at time begin between freq B.", 2.0, m1->getValue(A(1.9, 1)));
		assertEqual("Interpolation method STEPS - 2,2 between time between freq B.", 2.0, m1->getValue(A(1.9, 1.9)));
		assertEqual("Interpolation method STEPS - 2,2 at time end between freq B.", 3.0, m1->getValue(A(1.9, 2)));
		assertEqual("Interpolation method STEPS - 2,2 after time end between freq B.", 3.0, m1->getValue(A(1.9, 3)));

		assertEqual("Interpolation method STEPS - 2,2 before time at freq end B.", 1.0, m1->getValue(A(2, 1)));
		assertEqual("Interpolation method STEPS - 2,2 at time at freq end B.", 4.0, m1->getValue(A(2, 2)));
		assertEqual("Interpolation method STEPS - 2,2 after time end at freq end B.", 4.0, m1->getValue(A(2, 3)));

		assertEqual("Interpolation method STEPS - 2,2 before time after freq end B.", 1.0, m1->getValue(A(3, 1)));
		assertEqual("Interpolation method STEPS - 2,2 at time after freq end B.", 4.0, m1->getValue(A(3, 2)));
		assertEqual("Interpolation method STEPS - 2,2 after time end after freq end B.", 4.0, m1->getValue(A(3, 3)));

		delete m1;
	}

	void testNearest() {
		Mapping* t1 = MappingUtils::createMapping(DimensionSet(Dimension::time), Mapping::NEAREST);

		assertEqual("Interpolation method NEAREST - empty time A.", 0, t1->getValue(A(0)));

		t1->setValue(A(1), 2.0);

		assertEqual("Interpolation method NEAREST - single before time A.", 2.0, t1->getValue(A(0)));
		assertEqual("Interpolation method NEAREST - single at time A.", 2.0, t1->getValue(A(1)));
		assertEqual("Interpolation method NEAREST - single after time A.", 2.0, t1->getValue(A(2)));

		t1->setValue(A(2), 3.0);

		assertEqual("Interpolation method NEAREST - 2 before time A.", 2.0, t1->getValue(A(0)));
		assertEqual("Interpolation method NEAREST - 2 at begin time A.", 2.0, t1->getValue(A(1)));
		assertEqual("Interpolation method NEAREST - 2 between lower time A.", 2.0, t1->getValue(A(1.2)));
		assertEqual("Interpolation method NEAREST - 2 between upper time A.", 3.0, t1->getValue(A(1.9)));
		assertEqual("Interpolation method NEAREST - 2 at end time A.", 3.0, t1->getValue(A(2)));
		assertEqual("Interpolation method NEAREST - 2 after time A.", 3.0, t1->getValue(A(2.5)));

		delete t1;

		t1 = MappingUtils::createMapping(1.0, DimensionSet(Dimension::time), Mapping::NEAREST);

		assertEqual("Interpolation method NEAREST - empty time B.", 1.0, t1->getValue(A(0)));

		t1->setValue(A(1), 2.0);

		assertEqual("Interpolation method NEAREST - single before time B.", 1.0, t1->getValue(A(0)));
		assertEqual("Interpolation method NEAREST - single at time B.", 2.0, t1->getValue(A(1)));
		assertEqual("Interpolation method NEAREST - single after time B.", 1.0, t1->getValue(A(2)));

		t1->setValue(A(2), 3.0);

		assertEqual("Interpolation method NEAREST - 2 before time B.", 1.0, t1->getValue(A(0)));
		assertEqual("Interpolation method NEAREST - 2 at begin time B.", 2.0, t1->getValue(A(1)));
		assertEqual("Interpolation method NEAREST - 2 between lower time B.", 2.0, t1->getValue(A(1.2)));
		assertEqual("Interpolation method NEAREST - 2 between upper time B.", 3.0, t1->getValue(A(1.9)));
		assertEqual("Interpolation method NEAREST - 2 at end time B.", 3.0, t1->getValue(A(2)));
		assertEqual("Interpolation method NEAREST - 2 after time B.", 1.0, t1->getValue(A(2.5)));

		delete t1;

		Mapping* m1 = MappingUtils::createMapping(DimensionSet(time, freq), Mapping::NEAREST);

		assertEqual("Interpolation method NEAREST - empty time-freq A.", 0.0, m1->getValue(A(0, 0)));

		m1->setValue(A(1, 1), 2.0);

		assertEqual("Interpolation method NEAREST - single before time at freq A.", 2.0, m1->getValue(A(1, 0)));
		assertEqual("Interpolation method NEAREST - single at time at freq A.", 2.0, m1->getValue(A(1, 1)));
		assertEqual("Interpolation method NEAREST - single after time at freq A.", 2.0, m1->getValue(A(1, 2)));

		assertEqual("Interpolation method NEAREST - single before time before freq A.", 2.0, m1->getValue(A(0, 0)));
		assertEqual("Interpolation method NEAREST - single at time before freq A.", 2.0, m1->getValue(A(0, 1)));
		assertEqual("Interpolation method NEAREST - single after time before freq A.", 2.0, m1->getValue(A(0, 2)));

		assertEqual("Interpolation method NEAREST - single before time after freq A.", 2.0, m1->getValue(A(2, 0)));
		assertEqual("Interpolation method NEAREST - single at time after freq A.", 2.0, m1->getValue(A(2, 1)));
		assertEqual("Interpolation method NEAREST - single after time after freq A.", 2.0, m1->getValue(A(2, 2)));

		m1->setValue(A(1, 2), 3.0);

		assertEqual("Interpolation method NEAREST - 1,2 before time at freq A.", 2.0, m1->getValue(A(1, 0)));
		assertEqual("Interpolation method NEAREST - 1,2 at time begin at freq A.", 2.0, m1->getValue(A(1, 1)));
		assertEqual("Interpolation method NEAREST - 1,2 between lower time at freq A.", 2.0, m1->getValue(A(1, 1.2)));
		assertEqual("Interpolation method NEAREST - 1,2 between upper time at freq A.", 3.0, m1->getValue(A(1, 1.9)));
		assertEqual("Interpolation method NEAREST - 1,2 at time end at freq A.", 3.0, m1->getValue(A(1, 2)));
		assertEqual("Interpolation method NEAREST - 1,2 after time end at freq A.", 3.0, m1->getValue(A(1, 3)));

		assertEqual("Interpolation method NEAREST - 1,2 before time before freq A.", 2.0, m1->getValue(A(0, 0)));
		assertEqual("Interpolation method NEAREST - 1,2 at time begin before freq A.", 2.0, m1->getValue(A(0, 1)));
		assertEqual("Interpolation method NEAREST - 1,2 between lower time before freq A.", 2.0, m1->getValue(A(0, 1.2)));
		assertEqual("Interpolation method NEAREST - 1,2 between upper time before freq A.", 3.0, m1->getValue(A(0, 1.9)));
		assertEqual("Interpolation method NEAREST - 1,2 at time end before freq A.", 3.0, m1->getValue(A(0, 2)));
		assertEqual("Interpolation method NEAREST - 1,2 after time end before freq A.", 3.0, m1->getValue(A(0, 3)));

		assertEqual("Interpolation method NEAREST - 1,2 before time after freq A.", 2.0, m1->getValue(A(2, 0)));
		assertEqual("Interpolation method NEAREST - 1,2 at time begin after freq A.", 2.0, m1->getValue(A(2, 1)));
		assertEqual("Interpolation method NEAREST - 1,2 between lower time after freq A.", 2.0, m1->getValue(A(2, 1.2)));
		assertEqual("Interpolation method NEAREST - 1,2 between upper time after freq A.", 3.0, m1->getValue(A(2, 1.9)));
		assertEqual("Interpolation method NEAREST - 1,2 at time end after freq A.", 3.0, m1->getValue(A(2, 2)));
		assertEqual("Interpolation method NEAREST - 1,2 after time end after freq A.", 3.0, m1->getValue(A(2, 3)));

		m1->setValue(A(2, 2), 4.0);

		assertEqual("Interpolation method NEAREST - 2,2 before time at freq begin A.", 2.0, m1->getValue(A(1, 0)));
		assertEqual("Interpolation method NEAREST - 2,2 at time begin at freq begin A.", 2.0, m1->getValue(A(1, 1)));
		assertEqual("Interpolation method NEAREST - 2,2 between lower time at freq begin A.", 2.0, m1->getValue(A(1, 1.2)));
		assertEqual("Interpolation method NEAREST - 2,2 between upper time at freq begin A.", 3.0, m1->getValue(A(1, 1.9)));
		assertEqual("Interpolation method NEAREST - 2,2 at time end at freq begin A.", 3.0, m1->getValue(A(1, 2)));
		assertEqual("Interpolation method NEAREST - 2,2 after time end at freq begin A.", 3.0, m1->getValue(A(1, 3)));

		assertEqual("Interpolation method NEAREST - 2,2 before time before freq A.", 2.0, m1->getValue(A(0, 0)));
		assertEqual("Interpolation method NEAREST - 2,2 at time begin before freq A.", 2.0, m1->getValue(A(0, 1)));
		assertEqual("Interpolation method NEAREST - 2,2 between lower time before freq A.", 2.0, m1->getValue(A(0, 1.2)));
		assertEqual("Interpolation method NEAREST - 2,2 between upper time before freq A.", 3.0, m1->getValue(A(0, 1.9)));
		assertEqual("Interpolation method NEAREST - 2,2 at time end before freq A.", 3.0, m1->getValue(A(0, 2)));
		assertEqual("Interpolation method NEAREST - 2,2 after time end before freq A.", 3.0, m1->getValue(A(0, 3)));

		assertEqual("Interpolation method NEAREST - 2,2 before time between lower freq A.", 2.0, m1->getValue(A(1.2, 0)));
		assertEqual("Interpolation method NEAREST - 2,2 at time begin between lower freq A.", 2.0, m1->getValue(A(1.2, 1)));
		assertEqual("Interpolation method NEAREST - 2,2 between lower time between lower freq A.", 2.0, m1->getValue(A(1.2, 1.2)));
		assertEqual("Interpolation method NEAREST - 2,2 between upper time between lower freq A.", 3.0, m1->getValue(A(1.2, 1.9)));
		assertEqual("Interpolation method NEAREST - 2,2 at time end between lower freq A.", 3.0, m1->getValue(A(1.2, 2)));
		assertEqual("Interpolation method NEAREST - 2,2 after time end between lower freq A.", 3.0, m1->getValue(A(1.2, 3)));

		assertEqual("Interpolation method NEAREST - 2,2 before time between upper freq A.", 4.0, m1->getValue(A(1.9, 1)));
		assertEqual("Interpolation method NEAREST - 2,2 at time between upper freq A.", 4.0, m1->getValue(A(1.9, 2)));
		assertEqual("Interpolation method NEAREST - 2,2 after time between upper freq A.", 4.0, m1->getValue(A(1.9, 3)));

		assertEqual("Interpolation method NEAREST - 2,2 before time at freq end A.", 4.0, m1->getValue(A(2, 1)));
		assertEqual("Interpolation method NEAREST - 2,2 at time at freq end A.", 4.0, m1->getValue(A(2, 2)));
		assertEqual("Interpolation method NEAREST - 2,2 after time end at freq end A.", 4.0, m1->getValue(A(2, 3)));

		assertEqual("Interpolation method NEAREST - 2,2 before time after freq end A.", 4.0, m1->getValue(A(3, 1)));
		assertEqual("Interpolation method NEAREST - 2,2 at time after freq end A.", 4.0, m1->getValue(A(3, 2)));
		assertEqual("Interpolation method NEAREST - 2,2 after time end after freq end A.", 4.0, m1->getValue(A(3, 3)));

		delete m1;

		m1 = MappingUtils::createMapping(1.0, DimensionSet(time, freq), Mapping::NEAREST);

		assertEqual("Interpolation method NEAREST - empty time-freq B.", 1.0, m1->getValue(A(0, 0)));

		m1->setValue(A(1, 1), 2.0);

		assertEqual("Interpolation method NEAREST - single before time at freq B.", 1.0, m1->getValue(A(1, 0)));
		assertEqual("Interpolation method NEAREST - single at time at freq B.", 2.0, m1->getValue(A(1, 1)));
		assertEqual("Interpolation method NEAREST - single after time at freq B.", 1.0, m1->getValue(A(1, 2)));

		assertEqual("Interpolation method NEAREST - single before time before freq B.", 1.0, m1->getValue(A(0, 0)));
		assertEqual("Interpolation method NEAREST - single at time before freq B.", 1.0, m1->getValue(A(0, 1)));
		assertEqual("Interpolation method NEAREST - single after time before freq B.", 1.0, m1->getValue(A(0, 2)));

		assertEqual("Interpolation method NEAREST - single before time after freq B.", 1.0, m1->getValue(A(2, 0)));
		assertEqual("Interpolation method NEAREST - single at time after freq B.", 1.0, m1->getValue(A(2, 1)));
		assertEqual("Interpolation method NEAREST - single after time after freq B.", 1.0, m1->getValue(A(2, 2)));

		m1->setValue(A(1, 2), 3.0);

		assertEqual("Interpolation method NEAREST - 1,2 before time at freq B.", 1.0, m1->getValue(A(1, 0)));
		assertEqual("Interpolation method NEAREST - 1,2 at time begin at freq B.", 2.0, m1->getValue(A(1, 1)));
		assertEqual("Interpolation method NEAREST - 1,2 between lower time at freq B.", 2.0, m1->getValue(A(1, 1.2)));
		assertEqual("Interpolation method NEAREST - 1,2 between upper time at freq B.", 3.0, m1->getValue(A(1, 1.9)));
		assertEqual("Interpolation method NEAREST - 1,2 at time end at freq B.", 3.0, m1->getValue(A(1, 2)));
		assertEqual("Interpolation method NEAREST - 1,2 after time end at freq B.", 1.0, m1->getValue(A(1, 3)));

		assertEqual("Interpolation method NEAREST - 1,2 before time before freq B.", 1.0, m1->getValue(A(0, 0)));
		assertEqual("Interpolation method NEAREST - 1,2 at time begin before freq B.", 1.0, m1->getValue(A(0, 1)));
		assertEqual("Interpolation method NEAREST - 1,2 between lower time before freq B.", 1.0, m1->getValue(A(0, 1.2)));
		assertEqual("Interpolation method NEAREST - 1,2 between upper time before freq B.", 1.0, m1->getValue(A(0, 1.9)));
		assertEqual("Interpolation method NEAREST - 1,2 at time end before freq B.", 1.0, m1->getValue(A(0, 2)));
		assertEqual("Interpolation method NEAREST - 1,2 after time end before freq B.", 1.0, m1->getValue(A(0, 3)));

		assertEqual("Interpolation method NEAREST - 1,2 before time after freq B.", 1.0, m1->getValue(A(2, 0)));
		assertEqual("Interpolation method NEAREST - 1,2 at time begin after freq B.", 1.0, m1->getValue(A(2, 1)));
		assertEqual("Interpolation method NEAREST - 1,2 between lower time after freq B.", 1.0, m1->getValue(A(2, 1.2)));
		assertEqual("Interpolation method NEAREST - 1,2 between upper time after freq B.", 1.0, m1->getValue(A(2, 1.9)));
		assertEqual("Interpolation method NEAREST - 1,2 at time end after freq B.", 1.0, m1->getValue(A(2, 2)));
		assertEqual("Interpolation method NEAREST - 1,2 after time end after freq B.", 1.0, m1->getValue(A(2, 3)));

		m1->setValue(A(2, 2), 4.0);

		assertEqual("Interpolation method NEAREST - 2,2 before time at freq begin B.", 1.0, m1->getValue(A(1, 0)));
		assertEqual("Interpolation method NEAREST - 2,2 at time begin at freq begin B.", 2.0, m1->getValue(A(1, 1)));
		assertEqual("Interpolation method NEAREST - 2,2 between lower time at freq begin B.", 2.0, m1->getValue(A(1, 1.2)));
		assertEqual("Interpolation method NEAREST - 2,2 between upper time at freq begin B.", 3.0, m1->getValue(A(1, 1.9)));
		assertEqual("Interpolation method NEAREST - 2,2 at time end at freq begin B.", 3.0, m1->getValue(A(1, 2)));
		assertEqual("Interpolation method NEAREST - 2,2 after time end at freq begin B.", 1.0, m1->getValue(A(1, 3)));

		assertEqual("Interpolation method NEAREST - 2,2 before time before freq B.", 1.0, m1->getValue(A(0, 0)));
		assertEqual("Interpolation method NEAREST - 2,2 at time begin before freq B.", 1.0, m1->getValue(A(0, 1)));
		assertEqual("Interpolation method NEAREST - 2,2 between lower time before freq B.", 1.0, m1->getValue(A(0, 1.2)));
		assertEqual("Interpolation method NEAREST - 2,2 between upper time before freq B.", 1.0, m1->getValue(A(0, 1.9)));
		assertEqual("Interpolation method NEAREST - 2,2 at time end before freq B.", 1.0, m1->getValue(A(0, 2)));
		assertEqual("Interpolation method NEAREST - 2,2 after time end before freq B.", 1.0, m1->getValue(A(0, 3)));

		assertEqual("Interpolation method NEAREST - 2,2 before time between lower freq B.", 1.0, m1->getValue(A(1.2, 0)));
		assertEqual("Interpolation method NEAREST - 2,2 at time begin between lower freq B.", 2.0, m1->getValue(A(1.2, 1)));
		assertEqual("Interpolation method NEAREST - 2,2 between lower time between lower freq B.", 2.0, m1->getValue(A(1.2, 1.2)));
		assertEqual("Interpolation method NEAREST - 2,2 between upper time between lower freq B.", 3.0, m1->getValue(A(1.2, 1.9)));
		assertEqual("Interpolation method NEAREST - 2,2 at time end between lower freq B.", 3.0, m1->getValue(A(1.2, 2)));
		assertEqual("Interpolation method NEAREST - 2,2 after time end between lower freq B.", 1.0, m1->getValue(A(1.2, 3)));

		assertEqual("Interpolation method NEAREST - 2,2 before time between upper freq B.", 1.0, m1->getValue(A(1.9, 1)));
		assertEqual("Interpolation method NEAREST - 2,2 at time between upper freq B.", 4.0, m1->getValue(A(1.9, 2)));
		assertEqual("Interpolation method NEAREST - 2,2 after time between upper freq B.", 1.0, m1->getValue(A(1.9, 3)));

		assertEqual("Interpolation method NEAREST - 2,2 before time at freq end B.", 1.0, m1->getValue(A(2, 1)));
		assertEqual("Interpolation method NEAREST - 2,2 at time at freq end B.", 4.0, m1->getValue(A(2, 2)));
		assertEqual("Interpolation method NEAREST - 2,2 after time end at freq end B.", 1.0, m1->getValue(A(2, 3)));

		assertEqual("Interpolation method NEAREST - 2,2 before time after freq end B.", 1.0, m1->getValue(A(3, 1)));
		assertEqual("Interpolation method NEAREST - 2,2 at time after freq end B.", 1.0, m1->getValue(A(3, 2)));
		assertEqual("Interpolation method NEAREST - 2,2 after time end after freq end B.", 1.0, m1->getValue(A(3, 3)));

		delete m1;
	}

	void testInterpolationMethods() {
		testSteps();
		testNearest();

	}

	class TestSimpleConstMapping: public SimpleConstMapping {
	public:
		TestSimpleConstMapping(const DimensionSet& dims):
			SimpleConstMapping(dims) {}

		TestSimpleConstMapping(const DimensionSet& dims, Argument min, Argument max, Argument interval):
			SimpleConstMapping(dims, min, max, interval) {}

		double getValue(const Argument& pos) const {
			return SIMTIME_DBL(pos.getTime());
		}

		ConstMapping* constClone() const { return new TestSimpleConstMapping(*this); }
	};

	/*void testPerformance() {

		const double chkTime = 1000.0;
		const double factor = 1.0;
		Dimension time("Time");
		Dimension channel("Channel");
		Dimension space("Space");

		DimensionSet chTime(Dimension::time());
		chTime.addDimension(channel);

		DimensionSet spcChTime(chTime);
		spcChTime.addDimension(space);

		Time timer;
		int count = 70000 * factor;
		double el;

		Mapping* res;
		{
			TimeMapping f1;
			TimeMapping f2;

			std::cout << "--------TimeMapping [" << count << " entries]---------------------------------------------" << std::endl;
			std::cout << "Creating f1...\t\t\t\t";
			std::flush(std::cout);

			timer.start();
			for(int j = 0; j < count; j++) {
				f1.setValue(Argument(j * 0.1), j * 0.1);
			}
			el = timer.elapsed();
			std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry)." << std::endl;
			std::cout << "Creating f2 ...\t\t\t\t";
			std::flush(std::cout);
			timer.start();
			for(int j = 0; j < count; j++) {
				f2.setValue(Argument(j * 0.1), j * 0.1);
			}
			el = timer.elapsed();
			std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry)." << std::endl;
			std::cout << "Multiplying f1 with f2...\t\t";
			std::flush(std::cout);
			timer.start();
			res = f1 * f2;
			el = timer.elapsed();
			delete res;
			std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry)." << std::endl;
		}


		count = 35000 * factor;
		int hCount = sqrt(count);
		Argument pos(0.0);
		pos.setArgValue(channel, 0.0);

		{
			std::cout << "--------MultiDimFunction [" << count << " entries]----------------------------------------" << std::endl;
			std::cout << "Creating f7 ...\t\t\t\t";
			MultiDimMapping f7(chTime);
			MultiDimMapping f8(chTime);

			std::flush(std::cout);
			timer.start();
			for(int j = 0; j < count; j++) {
				pos.setTime((j % hCount) * 0.1);
				pos.setArgValue(channel, 0.1 * (int)(j / hCount));
				f7.setValue(pos, j * 0.1);
			}
			el = timer.elapsed();
			std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry)." << std::endl;
			std::cout << "Creating f8 ...\t\t\t\t";
			std::flush(std::cout);
			timer.start();
			for(int j = 0; j < count; j++) {
				pos.setTime((j % hCount) * 0.1);
				pos.setArgValue(channel, 0.1 * (int)(j / hCount));
				f8.setValue(pos, j * 0.1);
			}
			el = timer.elapsed();
			std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry)." << std::endl;
			std::cout << "Multiplying f7 with f8...\t\t";
			std::flush(std::cout);
			timer.start();
			res = f7 * f8;
			el = timer.elapsed();
			delete res;
			std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry)." << std::endl;
		}

		count = 35000 * factor;
		hCount = pow(count, 0.33);
		count = hCount * hCount * hCount;

		{
			std::cout << "--------MultiDimFunction 3D [" << count << " entries]--------------------------------------" << std::endl;
			std::cout << "Creating f9 and f10...\t\t\t";
			MultiDimMapping f9(spcChTime);
			MultiDimMapping f10(spcChTime);

			std::flush(std::cout);
			timer.start();
			int j = 0;
			for(int s = 0; s < hCount; s++) {
				pos.setArgValue(space, s * 0.1);
				for(int c = 0; c < hCount; c++) {
					pos.setArgValue(channel, c * 0.1);
					for(int t = 0; t < hCount; t++) {
						pos.setTime(t * 0.1);
						f10.setValue(pos, s * t * c * 0.1);
						f9.setValue(pos, s * t * c * 0.1);
						j++;
					}
				}
			}
			el = timer.elapsed();
			std::cout << "done. Took " << el << "ms(" << el * 1000.0 / (count * 2) << "us per entry)." << std::endl;

			std::cout << "Multiplying f9 with f10...\t\t";
			std::flush(std::cout);
			timer.start();
			res = f9 * f10;
			el = timer.elapsed();
			std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry)." << std::endl;

			for(int s = 0; s < hCount; s++) {
				pos.setArgValue(space, s * 0.1);
				for(int c = 0; c < hCount; c++) {
					pos.setArgValue(channel, c * 0.1);
					for(int t = 0; t < hCount; t++) {
						pos.setTime(t * 0.1);
						assertClose("3d mult test.",pow(s*t*c*0.1, 2), res->getValue(pos));
					}
				}
			}
			delete res;

		}

		count = 35000 * factor;
		hCount = sqrt(count);
		count = hCount * hCount;

		{
			std::cout << "--------TestSimpleConstMapping [" << count << " entries]----------------------------------" << std::endl;
			std::cout << "Creating f13 ...\t\t\t\t";
			Argument from(0.0);
			from.setArgValue(channel, 0.0);
			Argument to(1.0);
			to.setArgValue(channel, 1.0);
			Argument interval(1.0 / hCount);
			interval.setArgValue(channel, 1.0 / hCount);

			std::flush(std::cout);
			timer.start();
			TestSimpleConstMapping f13(chTime, from, to, interval);
			el = timer.elapsed();
			std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry)." << std::endl;
			std::cout << "Creating f14...\t\t\t\t";
			std::flush(std::cout);
			timer.start();
			TestSimpleConstMapping f14(chTime, from, to, interval);
			el = timer.elapsed();
			std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry)." << std::endl;
			std::cout << "Multiplying f13 with f14...\t\t";
			std::flush(std::cout);
			timer.start();
			res = f13 * f14;
			el = timer.elapsed();
			delete res;
			std::cout << "done. Took " << el << "ms(" << el * 1000.0 / count << "us per entry)." << std::endl;
		}
	}*/

	void checkTestItFromTo(std::string msg, ConstMappingIterator* it,
						   ConstMapping* f, Argument from, Argument to, Argument step,
						   std::map<double, std::map<simtime_t, Argument> >& /*a*/){

		for(double i = from.getArgValue(Dimension("frequency"));
			i <= to.getArgValue(Dimension("frequency"));
			i+=step.getArgValue(Dimension("frequency"))) {
			double in = i;
			simtime_t l = to.getTime();
			if(fabs(i - to.getArgValue(Dimension("frequency"))) < 0.0001)
				l -= 0.0001;
			for(simtime_t j = from.getTime(); j <= l; j+=step.getTime()) {
				simtime_t jn = j + step.getTime();
				if(jn > to.getTime()){
					jn = from.getTime();
					in += step.getArgValue(Dimension("frequency"));
				}
				checkIterator(msg, *it, true, true, A(i, j), A(in, jn), SIMTIME_DBL(j), *f);
				it->next();
			}
		}
	}

	void testSimpleConstmapping(){
		//displayPassed = true;

		SimpleConstMapping* f = new TestSimpleConstMapping(time);

		for(simtime_t t = -2.2; t <= 2.2; t+=0.2)
			assertClose("Get value of not fully initialized time-mapping.", t, f->getValue(Argument(t)));

		f->initializeArguments(Argument(-2.2), Argument(2.2), Argument(0.2));

		for(simtime_t t = -2.2; t <= 2.2; t+=0.2)
			assertClose("Get value of fully initialized time-mapping.", t, f->getValue(Argument(t)));

		ConstMappingIterator* it = f->createConstIterator();

		checkIterator("Initial iterator from begin.", *it, true, true, Argument(-2.2), Argument(-2.0), -2.2, *f);
		for(simtime_t t = -2.0; t < 2.19999; t+=0.2){
			checkNext("Next of iterator from begin.", *it, true, true, Argument(t), Argument(t + 0.2), SIMTIME_DBL(t), *f);
		}
		checkNext("Next of iterator from begin2.", *it, false, true, Argument(2.2), Argument(2.4), 2.2, *f);
		it->iterateTo(Argument(2.4));
		checkIterator("Next of iterator from begin.", *it, false, false, Argument(2.4), Argument(2.6), 2.4, *f);
		it->iterateTo(Argument(2.6));
		checkIterator("Next of iterator from begin.", *it, false, false, Argument(2.6), Argument(2.8), 2.6, *f);

		delete it;

		it = f->createConstIterator(Argument(-2.5));

		checkIterator("Initial iterator from pre-begin.", *it, true, false, Argument(-2.5), Argument(-2.2), -2.5, *f);
		for(simtime_t t = -2.2; t < 2.19999999; t+=0.2){
			checkNext("Next of iterator from pre-begin.", *it, true, true, Argument(t), Argument(t + 0.2), SIMTIME_DBL(t), *f);
		}
		checkNext("Next of iterator from pre-begin.", *it, false, true, Argument(2.2), Argument(2.4), 2.2, *f);
		it->iterateTo(Argument(2.4));
		checkIterator("Next of iterator from pre-begin.", *it, false, false, Argument(2.4), Argument(2.6), 2.4, *f);

		delete it;

		it = f->createConstIterator(Argument(.5));

		checkIterator("Initial iterator from post-begin.", *it, true, true, Argument(.5), Argument(.6), .5, *f);
		for(simtime_t t = .6; t < 2.199999; t+=0.2){
			checkNext("Next of iterator from post-begin.", *it, true, true, Argument(t), Argument(t + 0.2), SIMTIME_DBL(t), *f);
		}
		checkNext("Next of iterator from post-begin.", *it, false, true, Argument(2.2), Argument(2.4), 2.2, *f);
		it->iterateTo(Argument(2.4));
		checkIterator("Next of iterator from post-begin.", *it, false, false, Argument(2.4), Argument(2.6), 2.4, *f);

		delete it;

		it = f->createConstIterator(Argument(2.5));

		checkIterator("Initial iterator from post-end.", *it, false, false, Argument(2.5), Argument(2.7), 2.5, *f);
		it->iterateTo(Argument(2.7));
		checkIterator("Next of iterator from post-end.", *it, false, false, Argument(2.7), Argument(2.9), 2.7, *f);

		delete it;

		delete f;

		//displayPassed =false;

		DimensionSet freqTime(time);
		freqTime.addDimension(freq);

		std::map<double, std::map<simtime_t, Argument> > a;

		for(double i = 0.0; i <= 5.5; i+=0.25) {
			for(simtime_t j = SIMTIME_ZERO; j <= 5.5; j+=0.25) {
				A(i, j).setTime(j);
				A(i, j).setArgValue(freq, i);
			}
		}

		f = new TestSimpleConstMapping(freqTime, A(2, 1.5), A(4, 4), A(1, 0.5));

		for(double i = 0.0; i <= 5.5; i+=0.25) {
			for(simtime_t j = SIMTIME_ZERO; j <= 5.5; j+=0.25) {
				assertEqual("Get value of fully initialized freq-time-mapping.", j, f->getValue(A(i, j)));
			}
		}


		it = f->createConstIterator();

		checkTestItFromTo("Next of iterator from begin2.", it, f, A(2, 1.5), A(4, 4), A(1, 0.5), a);
		checkIterator("Next of iterator from begin2.", *it, false, true, A(4, 4), A(5, 1.5), 4, *f);
		it->iterateTo(A(5, 1.5));
		checkIterator("Next of iterator from begin.", *it, false, false, A(5, 1.5), A(5, 2.0), 1.5, *f);
		it->iterateTo(A(5, 2.0));
		checkIterator("Next of iterator from begin.", *it, false, false, A(5, 2.0), A(5, 2.5), 2.0, *f);
		delete it;

		it = f->createConstIterator(A(1, 1));

		checkIterator("Initial iterator from pre freq, pre time begin.", *it, true, false, A(1, 1), A(2, 1.5), 1, *f);
		it->next();
		checkTestItFromTo("Next of iterator from pre freq, pre time begin.", it, f, A(2, 1.5), A(4, 4), A(1, 0.5), a);
		checkIterator("Next of iterator from pre freq, pre time begin.", *it, false, true, A(4, 4), A(5, 1.5), 4, *f);
		it->iterateTo(A(5, 1.5));
		checkIterator("Next of iterator from pre freq, pre time begin.", *it, false, false, A(5, 1.5), A(5, 2.0), 1.5, *f);
		it->iterateTo(A(5, 2.0));
		checkIterator("Next of iterator from pre freq, pre time begin.", *it, false, false, A(5, 2.0), A(5, 2.5), 2.0, *f);
		delete it;

		it = f->createConstIterator(A(1, 2));

		checkIterator("Initial iterator from pre freq, post time begin.", *it, true, false, A(1, 2), A(2, 1.5), 2, *f);
		it->next();
		checkTestItFromTo("Next of iterator from pre freq, post time begin.", it, f, A(2, 1.5), A(4, 4), A(1, 0.5), a);
		checkIterator("Next of iterator from pre freq, post time begin.", *it, false, true, A(4, 4), A(5, 1.5), 4, *f);
		it->iterateTo(A(5, 1.5));
		checkIterator("Next of iterator from pre freq, post time begin.", *it, false, false, A(5, 1.5), A(5, 2.0), 1.5, *f);
		it->iterateTo(A(5, 2.0));
		checkIterator("Next of iterator from pre freq, post time begin.", *it, false, false, A(5, 2.0), A(5, 2.5), 2.0, *f);
		delete it;

		it = f->createConstIterator(A(3, 0.5));

		checkIterator("Initial iterator from post freq, pre time begin.", *it, true, true, A(3, 0.5), A(3, 1.5), 0.5, *f);
		it->next();
		checkTestItFromTo("Next of iterator from post freq, pre time begin.", it, f, A(3, 1.5), A(4, 4), A(1, 0.5), a);
		checkIterator("Next of iterator from post freq, pre time begin.", *it, false, true, A(4, 4), A(5, 1.5), 4, *f);
		it->iterateTo(A(5, 1.5));
		checkIterator("Next of iterator from post freq, pre time begin.", *it, false, false, A(5, 1.5), A(5, 2.0), 1.5, *f);
		it->iterateTo(A(5, 2.0));
		checkIterator("Next of iterator from post freq, pre time begin.", *it, false, false, A(5, 2.0), A(5, 2.5), 2.0, *f);
		delete it;

		it = f->createConstIterator(A(3, 5));

		checkIterator("Initial iterator from pre freq, post time end.", *it, true, true, A(3, 5), A(4, 1.5), 5, *f);
		it->next();
		checkTestItFromTo("Next of iterator from pre freq, post time end.", it, f, A(4, 1.5), A(4, 4), A(1, 0.5), a);
		checkIterator("Next of iterator from pre freq, post time end.", *it, false, true, A(4, 4), A(5, 1.5), 4, *f);
		it->iterateTo(A(5, 1.5));
		checkIterator("Next of iterator from pre freq, post time end.", *it, false, false, A(5, 1.5), A(5, 2.0), 1.5, *f);
		it->iterateTo(A(5, 2.0));
		checkIterator("Next of iterator from pre freq, post time end.", *it, false, false, A(5, 2.0), A(5, 2.5), 2.0, *f);
		delete it;

		it = f->createConstIterator(A(5, 2));

		checkIterator("Initial iterator from post freq, pre time end.", *it, false, false, A(5, 2), A(4, 1.5), 2, *f);
		it->iterateTo(A(5, 2.5));
		checkIterator("Next of iterator from post freq, pre time end.", *it, false, false, A(5, 2.5), A(5, 2.5), 2.5, *f);
		delete it;

		it = f->createConstIterator(A(5, 4.5));

		checkIterator("Initial iterator from post end.", *it, false, false, A(5, 4.5), A(4, 1.5), 4.5, *f);
		it->iterateTo(A(5, 5));
		checkIterator("Next of iterator from post end.", *it, false, false, A(5, 5), A(5, 2.5), 5, *f);
		delete it;

		delete f;

		//displayPassed = false;
	}

	void testOperatorAgainstInt64Simtime(){
		const char cSaveFill = std::cout.fill();
		std::cout << std::setw(80) << std::setfill('-') << std::internal << " Int64 simtime tests " << std::setw(48) << "" << std::setfill(cSaveFill) << std::endl; std::cout.flush();

		TimeMapping<Linear> time1;
		simtime_t t1 = 3.5;
		simtime_t t2 = 4;
		Argument pos1(t1);
		Argument pos2(t2);

		TimeMapping<Linear> time2;

		time1.setValue(pos1, 2);
		time2.setValue(pos1, 3);
		time2.setValue(pos2, 4);

		Mapping* res = time2 * time1;

		ConstMappingIterator* it = res->createConstIterator();
		checkIterator("time * time with same argument.", *it, true, true, pos1, pos2, 6, *res);
		checkNext("time * time after same argument.", *it, false, true, pos2, pos2, 8, *res);

		delete it;
		delete res;

		MultiDimMapping<Linear> multi1(DimensionSet(Dimension::time, freq));

		Argument pos1f(t1);
		pos1f.setArgValue(freq, 2.0);

		Argument pos2f(t2);
		pos2f.setArgValue(freq, 2.0);

		multi1.setValue(pos1f, 4.0);

		res = multi1 * time2;

		it = res->createConstIterator();
		checkIterator("multi * time with same argument.", *it, true, true, pos1f, pos2f, 12, *res);
		checkNext("multi * time after same argument.", *it, false, true, pos2f, pos2f, 16, *res);

		delete it;
		delete res;

		MultiDimMapping<Linear> multi2(DimensionSet(Dimension::time, freq, space));

		Argument pos1fs(t1);
		pos1fs.setArgValue(freq, 2.0);
		pos1fs.setArgValue(space, 2.5);
		Argument pos1f2s(t1);
		pos1f2s.setArgValue(space, 3.0);
		pos1f2s.setArgValue(freq, 2.0);

		multi2.setValue(pos1fs, 5.0);
		multi2.setValue(pos1f2s, 1.0);

		res = multi2 * multi1;

		it = res->createConstIterator();
		checkIterator("multi * multi with same argument.", *it, true, true, pos1fs, pos1f2s, 20, *res);
		checkNext("multi * multi after same argument.", *it, false, true, pos1f2s, pos1f2s, 4, *res);

		delete it;
		delete res;

		Argument pos0(0.5);

		TestSimpleConstMapping simple1(DimensionSet::timeDomain);

		simple1.initializeArguments(pos0, pos1, pos1);

		it = simple1.createConstIterator();
		checkIterator("simple with two entries.", *it, true, true, pos0, pos1, 0.5, simple1);
		checkNext("simple with two entries.", *it, false, true, pos1, pos1, 3.5, simple1);

		delete it;

		res = time1 * simple1;

		it = res->createConstIterator();
		checkIterator("time * simple with same argument.", *it, true, true, pos0, pos1, 1, *res);
		checkNext("time * simple with same argument.", *it, false, true, pos1, pos1, 7.0, *res);

		delete it;
		delete res;

		Argument pos0f(0.5);
		pos0f.setArgValue(freq, 2.0);

		res = multi1 * simple1;

		it = res->createConstIterator();
		checkIterator("multi * simple with same argument.", *it, true, true, pos0f, pos1f, 2.0, *res);
		checkNext("multi * simple with same argument.", *it, false, true, pos1f, Argument(t1 + 1), 14.0, *res);

		delete it;
		delete res;

		TestSimpleConstMapping simple2(DimensionSet(Dimension::time, freq));

		simple2.initializeArguments(pos1f, pos1f, pos1f);

		res = simple2 * simple1;

		it = res->createConstIterator();
		checkIterator("simple * simple with same argument.", *it, true, true, pos0f, pos1f, 0.25, *res);
		checkNext("simple * simple with same argument.", *it, false, true, pos1f, Argument(t1 + 1), 3.5*3.5, *res);

		delete it;
		delete res;

		std::cout << std::setw(80) << std::setfill('-') << std::internal << " Int64 simtime tests done. " << std::setw(48) << "" << std::setfill(cSaveFill) << std::endl; std::cout.flush();
	}

	template<class Operator>
	void testMappingOperator(std::string opName, Operator op){
		DimensionSet timeFreq(Dimension::time, freq);
		MultiDimMapping<Linear> f1(timeFreq);
		MultiDimMapping<Linear> f2(timeFreq);
		TimeMapping<Linear> f3;

		f3.setValue(A(2), 2);
		f3.setValue(A(3), 3);
		f1.setValue(A(1,2), 2);
		f1.setValue(A(1,3), 3);
		f2.setValue(A(1,2), 4);
		f2.setValue(A(1,3), 5);
		f2.setValue(A(2,1), 2);

		MultiDimMapping<Linear> exp(timeFreq);

		exp.setValue(A(1,2), op(2, 2));
		exp.setValue(A(1,3), op(3, 3));
		Mapping* res = MappingUtils::applyElementWiseOperator(f1, f3, op);
		assertMappingEqual("Operator " + opName + ": freq time 1 with time.", &exp, res);
		delete res;

		exp.setValue(A(1,2), op(4, 2));
		exp.setValue(A(1,3), op(5, 3));
		exp.setValue(A(2,1), op(2, 2));
		res = MappingUtils::applyElementWiseOperator(f2, f1, op);
		assertMappingEqual("Operator " + opName + ": freq time with freq time.", &exp, res);
		delete res;

		exp.setValue(A(2,2), op(2, 2));
		exp.setValue(A(2,3), op(2, 3));
		res = MappingUtils::applyElementWiseOperator(f2, f3, op);
		assertMappingEqual("Operator " + opName + ": freq time 2 with freq time.", &exp, res);
		delete res;

		/*
		TimeMapping exp2;

		exp2.setValue(A(1.5), op(2, 2));
		exp2.setValue(A(2), op(2, 2));
		exp2.setValue(A(2.5), op(2.5, 2.5));
		res = Mapping::applyElementWiseOperator(f3, f3, A(1.5), A(2.5), op);
		assertMappingEqual("Operator " + opName + ": time with time in interval.", &exp2, res);
		delete res;
		*/

	}

	void testAdd(){
		testMappingOperator("Plus", std::plus<double>());
	}



	Mapping* createTestMapping(simtime_t_cref from, simtime_t_cref to, int entries){
		Mapping* result = new TimeMapping<Linear>();

		if(entries > 1){
			simtime_t stepsize = (to - from) / (entries - 1);
			for(int i = 0; i < entries - 1; i++){
				result->setValue(A(from + stepsize * i), SIMTIME_DBL(from + stepsize * i));
			}
		}

		result->setValue(A(to), SIMTIME_DBL(to));

		return result;
	}

	Mapping* createMapping(DimensionSet dims, int keyCount, Argument* keys, bool endWrong = false, bool startWrong = false){
		if(createMappingBuffer)
			delete createMappingBuffer;
		createMappingBuffer = MappingUtils::createMapping(dims, Mapping::LINEAR);

		for(int i = 0; i < keyCount; i++){
			if(i == 0 && startWrong){
				createMappingBuffer->setValue(*keys, SIMTIME_DBL(keys->getTime()) * SIMTIME_DBL((keys + 1)->getTime()));
			} else if(i == keyCount - 1 && endWrong) {
				createMappingBuffer->setValue(*keys, SIMTIME_DBL(keys->getTime()) * SIMTIME_DBL((keys - 1)->getTime()));
			} else
				createMappingBuffer->setValue(*keys, SIMTIME_DBL(keys->getTime()) * SIMTIME_DBL(keys->getTime()));
			++keys;
		}

		return createMappingBuffer;
	}



	Mapping* createMapping(DimensionSet dims, int keyCount, Argument* keys, double value){
		if(createMappingBuffer)
			delete createMappingBuffer;
		createMappingBuffer = MappingUtils::createMapping(dims, Mapping::LINEAR);

		for(int i = 0; i < keyCount; i++){
			createMappingBuffer->setValue(*keys, SIMTIME_DBL(keys->getTime()) * value);
			++keys;
		}

		return createMappingBuffer;
	}

	Mapping* createMapping(DimensionSet dims, int keyCount, Argument* keys, double* other){
		if(createMappingBuffer)
			delete createMappingBuffer;
		createMappingBuffer = MappingUtils::createMapping(dims, Mapping::LINEAR);

		for(int i = 0; i < keyCount; i++){
			createMappingBuffer->setValue(*keys, SIMTIME_DBL(keys->getTime()) * *other);
			++keys;
			++other;
		}

		return createMappingBuffer;
	}



	void assertMultiplied(std::string msg, Mapping* m1, Mapping* m2, Mapping* exp){
		Mapping* res = MappingUtils::multiply(*m1, *m2);
		assertMappingEqual(msg, exp, res);
		delete res;
	}
	void testMulitply(){
		DimensionSet timeSet(time);
		//empty mapping
		TimeMapping<Linear>* empty = new TimeMapping<Linear>();

		//one element mapping
		Mapping* m5to5by1 = createTestMapping(5, 5, 1);

		//more than one element
		//from 3 to 6
		Mapping* m3to6by3 = createTestMapping(3, 6, 3);
		//from 2 to 7
		Mapping* m3to7by5 = createTestMapping(3, 7, 5);
		//from 4 to 5
		Mapping* m3to5by2 = createTestMapping(3, 5, 2);
		//from 3 to 7
		Mapping* m4to6by2 = createTestMapping(4, 6, 2);
		//from 2 to 6
		Mapping* m2to6by5 = createTestMapping(2, 6, 5);

		//1st is empty
		assertMultiplied("empty time * empty time", empty, empty, empty);
		Argument e5to5[] = {A(5)};
		assertMultiplied("empty time * 5to5by1 time", empty, m5to5by1, createMapping(timeSet, 1, e5to5, 0.0));
		Argument e3to6[] = {A(3), A(4.5), A(6)};
		assertMultiplied("empty time * 3to6by3 time", empty, m3to6by3, createMapping(timeSet, 3, e3to6, 0.0));

		//second is empty
		assertMultiplied("5to5by1 time * empty time", m5to5by1, empty, createMapping(timeSet, 1, e5to5, 0.0));
		assertMultiplied("3to6by3 time * empty time", m3to6by3, empty, createMapping(timeSet, 3, e3to6, 0.0));

		//1st has only one element
		assertMultiplied("5to5by1 time * 5to5by1 time", m5to5by1, m5to5by1, createMapping(timeSet, 1, e5to5, 5.0));
		Argument e3to5to6[] = {A(3), A(4.5), A(5), A(6)};
		assertMultiplied("5to5by1 time * 3to6by3 time", m5to5by1, m3to6by3, createMapping(timeSet, 4, e3to5to6, 5.0));
		//second has only one element
		assertMultiplied("3to6by3 time * 5to5by1 time", m3to6by3, m5to5by1, createMapping(timeSet, 4, e3to5to6, 5.0));

		//both have more than one element:
		//start at same
		//end at same
		assertMultiplied("3to6by3 time * 3to6by3 time", m3to6by3, m3to6by3, createMapping(timeSet, 3, e3to6));
		//end first smaller
		Argument e3to6and3to7[] = {A(3), A(4), A(4.5), A(5), A(6), A(7)};
		assertMultiplied("3to6by3 time * 3to7by5 time", m3to6by3, m3to7by5, createMapping(timeSet, 6, e3to6and3to7, true));
		//end first bigger
		Argument e3to6and3to5[] = {A(3), A(4.5), A(5), A(6)};
		assertMultiplied("3to6by3 time * 3to5by2 time", m3to6by3, m3to5by2, createMapping(timeSet, 4, e3to6and3to5, true));

		//1st start before second
		//end at same
		Argument e3to6and2to6[] = {A(2), A(3), A(4), A(4.5), A(5), A(6)};
		assertMultiplied("2to6by5 time * 3to6by3 time", m2to6by5, m3to6by3, createMapping(timeSet, 6, e3to6and2to6, false, true));
		//end first smaller
		Argument e2to6and3to5[] = {A(2), A(3), A(4), A(5), A(6)};
		assertMultiplied("2to6by5 time * 3to5by2 time", m2to6by5, m3to5by2, createMapping(timeSet, 5, e2to6and3to5, true, true));
		//end first bigger
		Argument e2to6and3to7[] = {A(2), A(3), A(4), A(5), A(6), A(7)};
		assertMultiplied("2to6by5 time * 3to7by5 time", m2to6by5, m3to7by5, createMapping(timeSet, 6, e2to6and3to7, true, true));
		//1st starts after second
		//end at same
		Argument e4to6and3to6[] = {A(3), A(4), A(4.5), A(6)};
		assertMultiplied("4to6by2 time * 3to6by3 time", m4to6by2, m3to6by3, createMapping(timeSet, 4, e4to6and3to6, false, true));
		//end first smaller
		Argument e4to6and3to5[] = {A(3), A(4), A(5), A(6)};
		assertMultiplied("4to6by2 time * 3to5by2 time", m4to6by2, m3to5by2, createMapping(timeSet, 4, e4to6and3to5, true, true));
		//end first bigger
		Argument e4to6and3to7[] = {A(3), A(4), A(5), A(6), A(7)};
		assertMultiplied("4to6by2 time * 3to7by5 time", m4to6by2, m3to7by5, createMapping(timeSet, 5, e4to6and3to7, true, true));

		delete empty;
		delete m5to5by1;
		delete m3to6by3;
		delete m4to6by2;
		delete m3to5by2;
		delete m3to7by5;
		delete m2to6by5;

		DimensionSet timeFreq(time, freq);
		DimensionSet timeSpace(time, space);
		DimensionSet timeFreqSpace(time, freq, space);
		Dimension bigDim("bigDim");
		DimensionSet timeFreqSpaceBig(time, freq, space);
		timeFreqSpaceBig.addDimension(bigDim);
		DimensionSet timeBig(time, bigDim);

		Mapping* ts11 = MappingUtils::createMapping(timeSpace);
		ts11->setValue(A(space, 1, 1), 1);
		Mapping* ts12 = MappingUtils::createMapping(timeSpace);
		ts12->setValue(A(space, 1, 2), 2);
		Mapping* ts21 = MappingUtils::createMapping(timeSpace);
		ts21->setValue(A(space, 2, 1), 1);
		Mapping* ts22 = MappingUtils::createMapping(timeSpace);
		ts22->setValue(A(space, 2, 2), 2);

		Mapping* tfs11 = MappingUtils::createMapping(timeFreqSpace);
		tfs11->setValue(A(1, 1, 1), 1);
		Mapping* tfs12 = MappingUtils::createMapping(timeFreqSpace);
		tfs12->setValue(A(1, 1, 2), 2);
		Mapping* tfs21 = MappingUtils::createMapping(timeFreqSpace);
		tfs21->setValue(A(2, 1, 1), 1);
		Mapping* tfs22 = MappingUtils::createMapping(timeFreqSpace);
		tfs22->setValue(A(2, 1, 2), 2);

		Argument e1111[] = {A(1,1,1)};
		assertMultiplied("tfs11 * ts11", tfs11, ts11, createMapping(timeFreqSpace, 1, e1111));
		Argument e1112[] = {A(1,1,1), A(1, 1, 2)};
		double d1112[] = {2, 1};
		assertMultiplied("tfs11 * ts12", tfs11, ts12, createMapping(timeFreqSpace, 2, e1112, d1112));
		Argument e1121[] = {A(1,1,1), A(2,1,1)};
		double d1121[] = {1, 1};
		assertMultiplied("tfs11 * ts21", tfs11, ts21, createMapping(timeFreqSpace, 2, e1121, d1121));
		Argument e1122[] = {A(1,1,1), A(2,1,2)};
		double d1122[] = {2, 1};
		assertMultiplied("tfs11 * ts22", tfs11, ts22, createMapping(timeFreqSpace, 2, e1122, d1122));

		Argument e1211[] = {A(1,1,1), A(1, 1, 2)};
		double d1211[] = {2, 1};
		assertMultiplied("tfs12 * ts11", tfs12, ts11, createMapping(timeFreqSpace, 2, e1211, d1211));
		Argument e2111[] = {A(1,1,1), A(2,1,1)};
		double d2111[] = {1, 1};
		assertMultiplied("tfs21 * ts11", tfs21, ts11, createMapping(timeFreqSpace, 2, e2111, d2111));
		Argument e2211[] = {A(1,1,1), A(2,1,2)};
		double d2211[] = {2, 1};
		assertMultiplied("tfs22 * ts11", tfs22, ts11, createMapping(timeFreqSpace, 2, e2211, d2211));

		Argument e2112[] = {A(1,1,2), A(2,1,1)};
		double d2112[] = {1, 2};
		assertMultiplied("tfs21 * ts12", tfs21, ts12, createMapping(timeFreqSpace, 2, e2112, d2112));

		Argument e1221[] = {A(1,1,2), A(2,1,1)};
		double d1221[] = {1, 2};
		assertMultiplied("tfs12 * ts21", tfs12, ts21, createMapping(timeFreqSpace, 2, e1221, d1221));

		Mapping* tfsD1 = tfs11->clone();
		tfsD1->setValue(A(1,2,3), 3);
		Mapping* tfsD2 = tfs12->clone();
		tfsD2->setValue(A(1,2,2), 2);

		Mapping* ts13 = MappingUtils::createMapping(timeSpace);
		ts13->setValue(A(space, 1, 3), 3);

		Argument eD111[] = {A(1,1,1), A(1,2,1), A(1,2,3)};
		double dD111[] = {1, 3, 1};
		assertMultiplied("tfsD1 * ts11", tfsD1, ts11, createMapping(timeFreqSpace, 3, eD111, dD111));
		Argument eD112[] = {A(1,1,1), A(1,1,2), A(1,2,2), A(1,2,3)};
		double dD112[] = {2, 1, 3, 2};
		assertMultiplied("tfsD1 * ts12", tfsD1, ts12, createMapping(timeFreqSpace, 4, eD112, dD112));
		Argument eD113[] = {A(1,1,1), A(1,1,3), A(1,2,3)};
		double dD113[] = {3, 1, 3};
		assertMultiplied("tfsD1 * ts13", tfsD1, ts13, createMapping(timeFreqSpace, 3, eD113, dD113));

		Argument eD213[] = {A(1,1,2), A(1,1,3), A(1,2,2), A(1,2,3)};
		double dD213[] = {3, 2, 3, 2};
		assertMultiplied("tfsD2 * ts13", tfsD2, ts13, createMapping(timeFreqSpace, 4, eD213, dD213));
		Argument eD211[] = {A(1,1,1), A(1,1,2), A(1,2,1), A(1,2,2)};
		double dD211[] = {2, 1, 2, 1};
		assertMultiplied("tfsD2 * ts11", tfsD2, ts11, createMapping(timeFreqSpace, 4, eD211, dD211));
		Argument eD221[] = {A(1,1,2), A(1,2,2), A(2,1,1), A(2,2,1)};
		double dD221[] = {1, 1, 2, 2};
		assertMultiplied("tfsD2 * ts21", tfsD2, ts21, createMapping(timeFreqSpace, 4, eD221, dD221));

		delete ts11;
		delete ts12;
		delete ts13;
		delete ts21;
		delete ts22;
		delete tfs11;
		delete tfs12;
		delete tfs21;
		delete tfs22;
		delete tfsD1;
		delete tfsD2;

		testMappingOperator("Multiply", std::multiplies<double>());
	}
	void testSubstract(){
		testMappingOperator("Minus", std::minus<double>());
	}
	void testDivide(){
		testMappingOperator("Divide", std::divides<double>());
	}

	/**
	 * @brief Creates a quadratic mapping with the passed domain and size at the
	 * passed offset.
	 *
	 * "Quadratic" in the sense of the number of dimensions of the passed domain
	 * not limited to 2D. In 3D it would be a cube in 4D a whatever.
	 * The value of each entry is the product of its coordinates (without the
	 * offset).
	 *
	 * @param domain The domain of the mapping to create.
	 * @param size The number of entries in each dimension to create.
	 * @param offset The position to start the entries at.
	 * @return A mapping with size^|domain| entries.
	 */
	Mapping* createTestMapping(	const DimensionSet& domain,
								unsigned int size = 3,
								double offset = 1.0)
	{
		Mapping* res = MappingUtils::createMapping(domain);
		unsigned int numDims = domain.size();

		//for each entry i
		for(int i = 0; i < pow((double)size, numDims); ++i) {
			Argument pos(domain);
			int d = numDims; 	//the current dimension counter
			int remain = i; //stores the remainder used to convert the entry
							//number into a coordinate in the domain
			double val = 1; //accumulates the value for the entry

			//iterate over dimensions and calculate coordinate in them
			for(DimensionSet::iterator it = domain.begin(); it != domain.end(); ++it)
			{
				--d;
				unsigned int coord = floor(remain / pow((double)size, d));
				val *= coord + 1;
				if(it == domain.begin())
					pos.setTime((double)coord + offset);
				else
					pos.setArgValue(*it, (double)coord + offset);
				remain = remain % (int)pow((double)size, d);
			}
			res->setValue(pos, val);
		}

		return res;
	}

	void checkMappingAddition(Mapping* m1, Mapping* m2, Mapping* result)
	{
		ConstMappingIterator* it = result->createConstIterator();

		while(true) {
			const Argument& pos = it->getPosition();
			assertEqual("@" + toString(pos)
						+ ":\tM1(" + toString(m1->getValue(pos))
						+ ")\t+ M2(" + toString(m2->getValue(pos))
						+ ")\t= " + toString(m1->getValue(pos)
											+ m2->getValue(pos)),
						m1->getValue(pos) + m2->getValue(pos), it->getValue());

			if(!it->hasNext())
				break;

			it->next();
		}

		delete it;
	}

	void testMappingMulBF(DimensionSet d1, DimensionSet d2) {
		Mapping* m1 = createTestMapping(d1, 2, 1.0);
		Mapping* m2 = createTestMapping(d2, 3, 0.5);

		Mapping* m3 = MappingUtils::add(*m1, *m2);
		checkMappingAddition(m1, m2, m3);
		delete m3;

		m3 = MappingUtils::add(*m2, *m1);
		checkMappingAddition(m2, m1, m3);
		delete m3;

		delete m1;
		delete m2;
	}

	void testOperatorBruteForce()
	{
		//displayPassed = true;

		DimensionSet timeFreq(time, freq);
		DimensionSet timeSpace(time, space);
		DimensionSet timeFreqSpace(time, freq, space);
		Dimension bigDim("bigDim");
		DimensionSet timeFreqSpaceBig(time, freq, space);
		timeFreqSpaceBig.addDimension(bigDim);
		DimensionSet timeBig(time, bigDim);
		const char   cSaveFill = std::cout.fill();

		std::cout << std::setw(80) << std::setfill('-') << std::internal << " TimeFreq^2. " << std::setw(48) << "" << std::setfill(cSaveFill) << std::endl; std::cout.flush();
		testMappingMulBF(timeFreq, timeFreq);
		std::cout << std::setw(80) << std::setfill('-') << std::internal << " Time * TimeBig. " << std::setw(48) << "" << std::setfill(cSaveFill) << std::endl; std::cout.flush();
		testMappingMulBF(timeBig, DimensionSet(time));
		std::cout << std::setw(80) << std::setfill('-') << std::internal << " timeSpace * timeFreqSpaceBig. " << std::setw(48) << "" << std::setfill(cSaveFill) << std::endl; std::cout.flush();
		testMappingMulBF(timeSpace, timeFreqSpaceBig);

		//displayPassed = false;
	}

	void testOperators(){
		testMulitply();

	#ifndef USE_DOUBLE_SIMTIME
		testOperatorAgainstInt64Simtime();
	#endif

		testAdd();

		testSubstract();
		testDivide();

		testOperatorBruteForce();
	}

	void testOutOfRange() {
		Mapping* f1 = MappingUtils::createMapping(0.2);

		assertEqual("Out of range empty",0.2, f1->getValue(A(0.5)));

		MappingIterator* it = f1->createIterator(A(0.5));
		assertEqual("Out of range it empty",0.2, it->getValue());
		delete it;

		f1->setValue(A(0.5), 2.0);
		assertEqual("Out of range single prev.",0.2, f1->getValue(A(0.3)));
		assertEqual("Out of range single after.",0.2, f1->getValue(A(0.7)));

		it = f1->createIterator(A(0.3));
		assertEqual("Out of range single it prev.",0.2, it->getValue());
		it->iterateTo(A(0.7));
		assertEqual("Out of range single it after.",0.2, it->getValue());
		delete it;

		Mapping* f2 = MappingUtils::createMapping(0.3, DimensionSet (time, freq));

		assertEqual("Out of range empty multi",0.3, f2->getValue(A(1.0, 0.5)));

		it = f2->createIterator(A(1.0, 0.5));
		assertEqual("Out of range it empty multi",0.3, it->getValue());
		delete it;

		f2->setValue(A(1,0, 0.5), 2.0);
		assertEqual("Out of range single prev multi.",0.3, f2->getValue(A(1.0, 0.3)));
		assertEqual("Out of range single after multi.",0.3, f2->getValue(A(1.0, 0.7)));

		assertEqual("Out of range single prev multi.",0.3, f2->getValue(A(0.5, 0.5)));
		assertEqual("Out of range single after multi.",0.3, f2->getValue(A(1.5, 0.5)));

		it = f2->createIterator(A(0.5, 0.5));
		assertEqual("Out of range single it prev multi.",0.3, it->getValue());
		it->iterateTo(A(1.0, 0.3));
		assertEqual("Out of range single it after multi.",0.3, it->getValue());
		it->iterateTo(A(1.0, 0.7));
		assertEqual("Out of range single it prev multi.",0.3, it->getValue());
		it->iterateTo(A(1.5, 0.5));
		assertEqual("Out of range single it after multi.",0.3, it->getValue());
		delete it;

		Mapping* res = MappingUtils::add(*f2, *f1, 0.1);

		assertEqual("Out of range added prev multi.",0.1, res->getValue(A(1.0, 0.3)));
		assertEqual("Out of range added after multi.",0.1, res->getValue(A(1.0, 0.7)));

		assertEqual("Out of range added prev multi.",0.1, res->getValue(A(0.5, 0.5)));
		assertEqual("Out of range added after multi.",0.1, res->getValue(A(1.5, 0.5)));

		delete res;
		delete f1;
		delete f2;
	}

	/**
	 * Unit tests for the MappingUtils::FindMin/-Max() methods.
	 * Every test with one dimensional and multidimensional mappings
	 * - test global min and max
	 * -- empty mapping
	 * -- one element mapping
	 * -- five element mapping with min/max at
	 * --- first entry
	 * --- last entry
	 * --- somewhere in the middle
	 *
	 * - test lokal min and max
	 * -- empty mapping
	 * -- one element mapping with element
	 * --- before range
	 * --- after range
	 * --- in range
	 * -- six element mapping with global min/max out range and local min/max at
	 * --- first entry
	 * --- last entry
	 * --- somewhere in the middle
	 *
	 */
	void testFindMinMax() {
		DimensionSet timeFreqSpace(time, freq, space);

		//- empty time mapping
		Mapping* timed1 = MappingUtils::createMapping(0.0);
		Mapping* multi1 = MappingUtils::createMapping(timeFreqSpace);

		//global
		assertEqual("Empty timed mapping max(global).", MappingUtils::cMaxNotFound, MappingUtils::findMax(*timed1));
		assertEqual("Empty timed mapping min(global).", MappingUtils::cMinNotFound, MappingUtils::findMin(*timed1));
		//local
		assertEqual("Empty timed mapping max(local).", MappingUtils::cMaxNotFound, MappingUtils::findMax(*timed1, A(1), A(2)));
		assertEqual("Empty timed mapping min(local).", MappingUtils::cMinNotFound, MappingUtils::findMin(*timed1, A(1), A(2)));

		//global
		assertEqual("Empty multidim mapping max(global).", MappingUtils::cMaxNotFound, MappingUtils::findMax(*multi1));
		assertEqual("Empty multidim mapping min(global).", MappingUtils::cMinNotFound, MappingUtils::findMin(*multi1));
		//local - is not yet implemented
		assertEqual("Empty multidim mapping max(local).", MappingUtils::cMaxNotFound, MappingUtils::findMax(*multi1, A(1,1,1), A(2,2,2)));
		assertEqual("Empty multidim mapping min(local).", MappingUtils::cMinNotFound, MappingUtils::findMin(*multi1, A(1,1,1), A(2,2,2)));

		//- one element mapping
		timed1->setValue(A(1), 2);
		multi1->setValue(A(1,1,1), 2);

		//global
		assertEqual("One element timed mapping max(global).", 2, MappingUtils::findMax(*timed1));
		assertEqual("One element timed mapping min(global).", 2, MappingUtils::findMin(*timed1));
		//local
		assertEqual("One element timed mapping max(local) before element.", MappingUtils::cMaxNotFound, MappingUtils::findMax(*timed1, A(0), A(0.5)));
		assertEqual("One element timed mapping min(local) before element.", 0.0, MappingUtils::findMin(*timed1, A(0), A(0.5), 0.0));
		assertEqual("One element timed mapping max(local) around element.", 2, MappingUtils::findMax(*timed1, A(1), A(1)));
		assertEqual("One element timed mapping min(local) around element.", 2, MappingUtils::findMin(*timed1, A(1), A(1)));
		assertEqual("One element timed mapping max(local) after element.", MappingUtils::cMaxNotFound, MappingUtils::findMax(*timed1, A(2), A(3)));
		assertEqual("One element timed mapping min(local) after element.", MappingUtils::cMinNotFound, MappingUtils::findMin(*timed1, A(2), A(3)));

		//global
		assertEqual("One element multidim mapping max(global).", 2, MappingUtils::findMax(*multi1));
		assertEqual("One element multidim mapping min(global).", 2, MappingUtils::findMin(*multi1));
		//local - is not yet implemented
		assertEqual("One element multidim mapping max(local) before element.", MappingUtils::cMaxNotFound, MappingUtils::findMax(*multi1, A(0,0,0), A(0.5,2,2)));
		assertEqual("One element multidim mapping min(local) before element.", MappingUtils::cMinNotFound, MappingUtils::findMin(*multi1, A(0,0,0), A(0.5,2,2)));
		assertEqual("One element multidim mapping max(local) around element.", 2, MappingUtils::findMax(*multi1, A(1,1,1), A(1,1,1)));
		assertEqual("One element multidim mapping min(local) around element.", 2, MappingUtils::findMin(*multi1, A(1,1,0), A(1,1,1)));
		assertEqual("One element multidim mapping max(local) after element.", MappingUtils::cMaxNotFound, MappingUtils::findMax(*multi1, A(2,0,0), A(3,2,2)));
		assertEqual("One element multidim mapping min(local) after element.", MappingUtils::cMinNotFound, MappingUtils::findMin(*multi1, A(2,0,0), A(3,2,2)));

		//Timed mapping multi element tests
		timed1->setValue(A(2), 2);  /**/ timed1->setValue(A(3), 2);
		timed1->setValue(A(4), 2);  /**/ timed1->setValue(A(5), 2);

		Mapping* timed2 = timed1->clone();
		Mapping* timed3 = timed1->clone();

		//-- max at first entry min at last
		timed1->setValue(A(1),3); /**/ timed1->setValue(A(5), 1);
		//-- max at last entry min at first
		timed2->setValue(A(1),1); /**/ timed2->setValue(A(5), 3);
		//-- min and max somewhere in the middle
		timed3->setValue(A(2),3); /**/ timed3->setValue(A(3), 1);

		//global
		assertEqual("Multi element timed mapping max(global) at first element.", 3, MappingUtils::findMax(*timed1));
		assertEqual("Multi element timed mapping min(global) at first element.", 1, MappingUtils::findMin(*timed2));
		assertEqual("Multi element timed mapping max(global) at last element.", 3, MappingUtils::findMax(*timed2));
		assertEqual("Multi element timed mapping min(global) at last element.", 1, MappingUtils::findMin(*timed1));
		assertEqual("Multi element timed mapping max(global) somewhere between.", 3, MappingUtils::findMax(*timed3));
		assertEqual("Multi element timed mapping min(global) somewhere between.", 1, MappingUtils::findMin(*timed3));

		//local
		//add global min and max out of range
		timed1->setValue(A(6), 10); /**/ timed1->setValue(A(7), 0);

		assertEqual("Multi element timed mapping max(local) at first element.", 3, MappingUtils::findMax(*timed1, A(1), A(5)));
		assertEqual("Multi element timed mapping min(local) at first element.", 1, MappingUtils::findMin(*timed2, A(1), A(5)));
		assertEqual("Multi element timed mapping max(local) at last element.", 3, MappingUtils::findMax(*timed2, A(1), A(5)));
		assertEqual("Multi element timed mapping min(local) at last element.", 1, MappingUtils::findMin(*timed1, A(1), A(5)));
		assertEqual("Multi element timed mapping max(local) somewhere between.", 3, MappingUtils::findMax(*timed3, A(1), A(5)));
		assertEqual("Multi element timed mapping min(local) somewhere between.", 1, MappingUtils::findMin(*timed3, A(1), A(5)));


		//Multidim mapping multi element tests
		for(int t=1; t < 5; ++t){
			for(int f=1; f < 5; ++f){
				for(int s=1; s < 5; ++s){
					multi1->setValue(A(t,f,s), 2);
				}
			}
		}

		for(int t=1; t < 5; ++t){
		for(int f=1; f < 5; ++f){
		for(int s=1; s < 5; ++s){
			Mapping* tmp = multi1->clone();
			//set min and max
			tmp->setValue(A(t,f,s),3);
			tmp->setValue(A(5-t,5-f,5-s),1);

			assertEqual("Multi element multidim mapping max(global) at (" +
						toString(t)+","+toString(f)+","+toString(s)+").",
						3, MappingUtils::findMax(*tmp));
			assertEqual("Multi element multidim mapping min(global) at (" +
						toString(t)+","+toString(f)+","+toString(s)+").",
						1, MappingUtils::findMin(*tmp));

			for(int t1=1; t1 < 5; ++t1){
			for(int f1=1; f1 < 5; ++f1){
			for(int s1=1; s1 < 5; ++s1){

				for(int t2=t1; t2 < 5; ++t2){
				for(int f2=f1; f2 < 5; ++f2){
				for(int s2=s1; s2 < 5; ++s2){
					double expMax = 2;
					double expMin = 2;

					if(t1==t2 && f1==f2 && s1==s2 && t1==5-t && f1==5-f && s1==5-s) {
						expMax = 1;
					} else if(t >= t1 && t <= t2 && f >= f1 && f <= f2 && s >= s1 && s <= s2) {
						expMax = 3;
					}
					if(t1==t2 && f1==f2 && s1==s2 && t1==t && f1==f && s1==s) {
						expMin=3;
					} else if(5-t >= t1 && 5-t <= t2 && 5-f >= f1 && 5-f <= f2 && 5-s >= s1 && 5-s <= s2) {
						expMin=1;
					}

					assertEqual("min at (" + toString(5-t)+","+toString(5-f)+","+toString(5-s)+
								") at local (" + toString(t1)+","+toString(f1)+","+toString(s1)+
								") to (" + toString(t2)+","+toString(f2)+","+toString(s2)+").",
								expMin, MappingUtils::findMin(*tmp, A(t1,f1,s1), A(t2,f2,s2)));
					assertEqual("max at (" + toString(t)+","+toString(f)+","+toString(s)+
								") at local (" + toString(t1)+","+toString(f1)+","+toString(s1)+
								") to (" + toString(t2)+","+toString(f2)+","+toString(s2)+").",
								expMax, MappingUtils::findMax(*tmp, A(t1,f1,s1), A(t2,f2,s2)));

				}
				}
				}
			}
			}
			}

			delete tmp;
		}
		}
		}


		delete timed1;
		delete timed2;
		delete timed3;
		delete multi1;
	}

	void testMappingUtils() {
		testFindMinMax();
	}

	void runTests() {
		displayPassed = false;
		testDimension();
		testArg();
		//testDoubleCompareLess();

	    testSimpleFunction<TimeMapping<Linear> >();
	    const char cSaveFill = std::cout.fill();

	    std::cout << std::setw(80) << std::setfill('-') << std::internal << " TimeMapping tests done. " << std::setw(48) << "" << std::setfill(cSaveFill) << std::endl; std::cout.flush();

	    testMultiFunction();
	    std::cout << std::setw(80) << std::setfill('-') << std::internal << " MultiDimMapping tests done. " << std::setw(48) << "" << std::setfill(cSaveFill) << std::endl; std::cout.flush();

	    testInterpolationMethods();
	    std::cout << std::setw(80) << std::setfill('-') << std::internal << " Interpolation methods tests done. " << std::setw(48) << "" << std::setfill(cSaveFill) << std::endl; std::cout.flush();

	    testSimpleConstmapping();
	    std::cout << std::setw(80) << std::setfill('-') << std::internal << " SimpleConstMapping tests done. " << std::setw(48) << "" << std::setfill(cSaveFill) << std::endl; std::cout.flush();

	    testOperators();
	    std::cout << std::setw(80) << std::setfill('-') << std::internal << " Operator tests done. " << std::setw(48) << "" << std::setfill(cSaveFill) << std::endl; std::cout.flush();

	    testOutOfRange();
	    std::cout << std::setw(80) << std::setfill('-') << std::internal << " Out of range tests done. " << std::setw(48) << "" << std::setfill(cSaveFill) << std::endl; std::cout.flush();

	    std::cout << std::setw(80) << std::setfill('-') << std::internal << " Various MappingUtils tests (may take a while) " << std::setw(48) << "" << std::setfill(cSaveFill) << std::endl; std::cout.flush();
	    testMappingUtils();
		std::cout << std::setw(80) << std::setfill('-') << std::internal << " Various MappingUtils tests done. " << std::setw(48) << "" << std::setfill(cSaveFill) << std::endl; std::cout.flush();

	    //std::cout << std::setw(80) << std::setfill('=') << std::internal << " Performance tests " << std::setw(48) << "" << std::setfill(cSaveFill) << std::endl; std::cout.flush();
	    //testPerformance();
		testsExecuted = true;
	}
};

Define_Module(MappingTest);
