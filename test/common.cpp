#include "common.h"
#include <random>

//
// Unnamed namespace for things only used in this file
//
namespace
{
	/** @brief a global pseudo-random number generator so that I don't have repeats of number sequences
	 *
	 * Gets set in the first call to "nextRandomNumber". */
	std::function<float()> globalRandomGenerator;
} // end of the unnamed namespace

float test::nextRandomNumber()
{
	if( ! ::globalRandomGenerator )
	{
		std::default_random_engine engine;
		typename std::uniform_real_distribution<float> random(0,100);
		::globalRandomGenerator=std::bind( random, engine );
	}

	return ::globalRandomGenerator();
}
