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

bool test::Point3D::operator==( const Point3D& other )
{
	return x==other.x && y==other.y && z==other.z;
}

template<>
void test::setRandom( test::Point3D& point )
{
	point.x=nextRandomNumber();
	point.y=nextRandomNumber();
	point.z=nextRandomNumber();
}

std::ostream& test::operator<<( std::ostream& output, const test::Point3D& point )
{
	output << "[" << point.x << "," << point.y << "," << point.z << "]";
	return output;
}
