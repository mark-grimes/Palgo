#include <palgo/static_kdtree.hpp>
#include "catch.hpp"


//
// Unnamed namespace for things only held in this file
//
namespace
{
	inline float staticFunction1( const int& datapoint )
	{
		return datapoint*2;
	}

	inline float staticFunction2( const int& datapoint )
	{
		return std::sqrt(datapoint);
	}

	struct TestDataStructure
	{
	private:
		float x;
		float y;
		float z;
	public:
		TestDataStructure( float x, float y, float z ) : x(x), y(y), z(z) {}
		inline float getX() { return x; }
		inline float getY() { return y; }
		inline float getZ() { return z; }
		static inline float staticGetX( const TestDataStructure& data ) { return data.x; }
		static inline float staticGetY( const TestDataStructure& data ) { return data.y; }
		static inline float staticGetZ( const TestDataStructure& data ) { return data.z; }
	};

} // end of the unnamed namespace


SCENARIO( "Check that a test_tree works correctly" )
{
	WHEN( "Constructing from a vector" )
	{
		const auto func1=[](const int& datapoint)->float{ return datapoint*2; };
		std::vector<int> input={ 4,54,2,54,256,345,4 };
		palgo::test_tree<int,&staticFunction1,&staticFunction2> foo( input.begin(), input.end() );
		auto results=foo.testMethod();
		CHECK( results[0]==staticFunction1(input[0]) );
		CHECK( results[1]==staticFunction2(input[1]) );
		CHECK( results[2]==staticFunction1(input[2]) );
		CHECK( results[3]==staticFunction2(input[3]) );
		CHECK( results[4]==staticFunction1(input[4]) );
		CHECK( results[5]==staticFunction2(input[5]) );
		CHECK( results[6]==staticFunction1(input[6]) );
	}
	WHEN( "Using methods as the partitioners" )
	{
		std::vector<TestDataStructure> input={ {3.4,6,'r'}, {23.2,6,'r'}, {23.56,6,'r'}, {654.1,6,'r'}, };
		palgo::test_tree<TestDataStructure,&TestDataStructure::staticGetX,&TestDataStructure::staticGetY,&TestDataStructure::staticGetZ> foo( input.begin(), input.end() );
		foo.testMethod();
		auto results=foo.testMethod();
		CHECK( results[0]==TestDataStructure::staticGetX(input[0]) );
		CHECK( results[1]==TestDataStructure::staticGetY(input[1]) );
		CHECK( results[2]==TestDataStructure::staticGetZ(input[2]) );
		CHECK( results[3]==TestDataStructure::staticGetX(input[3]) );
	}
	WHEN( "Constructing test_tree2" )
	{
		std::function<float(const int& datapoint)> func1=[](const int& datapoint)->float{ return datapoint*2; };
		std::function<float(const int& datapoint)> func2=staticFunction2;
//		auto pointer=func2.target();
//		std::cout << func2.target() << std::endl;

		std::vector<int> input={ 4,54,2,54,256,345,4 };
		palgo::test_tree2<int,float,&staticFunction1> foo( input.begin(), input.end() );
		CHECK( foo.testMethod() == true );
	}
}
