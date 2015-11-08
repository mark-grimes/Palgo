#include <palgo/static_kdtree.hpp>
#include <palgo/kdtree.hpp> // required to build trees until I fully implement the static_kdtree constructor. ToDo take this out.
#include "catch.hpp"
#include "common.h"

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

	struct Functor1
	{
		static float value( const int& datapoint )
		{
			return 1;
		}
	};

	struct Functor2
	{
		static int value( const int& datapoint )
		{
			return 2;
		}
	};

	struct Functor3
	{
		static float value( const int& datapoint )
		{
			return 3;
		}
	};

	/** @brief Just returns the input exactly as received */
	template<class T>
	struct ValueFunctor
	{
		static T value( const T& datapoint )
		{
			return datapoint;
		}
	};

	/** @brief Functor to partition kd-trees using the "x" data member. */
	template<class T_struct>
	struct XFunctor
	{
		static decltype(T_struct::x) value( const T_struct& datapoint )
		{
			return datapoint.x;
		}
	};

	template<class T_struct>
	struct YFunctor
	{
		static decltype(T_struct::y) value( const T_struct& datapoint )
		{
			return datapoint.y;
		}
	};

	template<class T_struct>
	struct ZFunctor
	{
		static decltype(T_struct::z) value( const T_struct& datapoint )
		{
			return datapoint.z;
		}
	};

	struct TestDataStructure
	{
		friend struct XFunctor<TestDataStructure>;
		friend struct YFunctor<TestDataStructure>;
		friend struct ZFunctor<TestDataStructure>;
	private:
		float x;
		float y;
		float z;
	public:
		TestDataStructure( float x, float y, float z ) : x(x), y(y), z(z) {}
		TestDataStructure() : x(0), y(0), z(0) {}
		bool operator==( const TestDataStructure& other ) { return x==other.x && y==other.y && z==other.z; }
		inline float getX() const { return x; }
		inline float getY() const { return y; }
		inline float getZ() const { return z; }
		inline void setX( float newValue ) { x=newValue; }
		inline void setY( float newValue ) { y=newValue; }
		inline void setZ( float newValue ) { z=newValue; }
		static inline float staticGetX( const TestDataStructure& data ) { return data.x; }
		static inline float staticGetY( const TestDataStructure& data ) { return data.y; }
		static inline float staticGetZ( const TestDataStructure& data ) { return data.z; }
	};

	std::ostream& operator<<( std::ostream& output, const TestDataStructure& data )
	{
		output << "[" << data.getX() << "," << data.getY() << "," << data.getZ() << "]";
		return output;
	}
} // end of the unnamed namespace

/** @brief Template specialisation of a function in "common.h" to set TestDataStructure instances to random values */
template<>
void test::setRandom( TestDataStructure& variable )
{
	variable.setX(nextRandomNumber());
	variable.setY(nextRandomNumber());
	variable.setZ(nextRandomNumber());
}



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
	WHEN( "Testing FunctionList variadic data structure with a single entry" )
	{
		typedef palgo::FunctionList< ::Functor1> T_FunctionList;

		// Don't know why, but putting the template directly in the CHECK call messes up the macro
		//CHECK( (palgo::ListCycle<1,T_FunctionList>::previous)==0 );
		//CHECK( palgo::ListCycle<2,T_FunctionList>::previous==1 );

		size_t test;
		test=palgo::ListCycle<0,T_FunctionList>::previous; CHECK( test==0 );
		test=palgo::ListCycle<0,T_FunctionList>::next; CHECK( test==0 );

		auto test1=palgo::ListCycle<0,T_FunctionList>::type::value(4); CHECK( test1==1 );

		float testFloat;
		testFloat=palgo::CyclicList<0,T_FunctionList>::type::value(4); CHECK( testFloat==1 );
		testFloat=palgo::CyclicList<1,T_FunctionList>::type::value(4); CHECK( testFloat==1 );
		testFloat=palgo::CyclicList<2,T_FunctionList>::type::value(4); CHECK( testFloat==1 );
		testFloat=palgo::CyclicList<3,T_FunctionList>::type::value(4); CHECK( testFloat==1 );
		testFloat=palgo::CyclicList<4,T_FunctionList>::type::value(4); CHECK( testFloat==1 );
		testFloat=palgo::CyclicList<5,T_FunctionList>::type::value(4); CHECK( testFloat==1 );
		testFloat=palgo::CyclicList<6,T_FunctionList>::type::value(4); CHECK( testFloat==1 );
		testFloat=palgo::CyclicList<7,T_FunctionList>::type::value(4); CHECK( testFloat==1 );
		testFloat=palgo::CyclicList<8,T_FunctionList>::type::value(4); CHECK( testFloat==1 );
	}
	WHEN( "Testing FunctionList variadic data structure with two entries" )
	{
		typedef palgo::FunctionList< ::Functor1,::Functor2> T_FunctionList;

		// Don't know why, but putting the template directly in the CHECK call messes up the macro
		//CHECK( (palgo::ListCycle<1,T_FunctionList>::previous)==0 );
		//CHECK( palgo::ListCycle<2,T_FunctionList>::previous==1 );

		size_t test;
		test=palgo::ListCycle<0,T_FunctionList>::previous; CHECK( test==1 );
		test=palgo::ListCycle<1,T_FunctionList>::previous; CHECK( test==0 );

		test=palgo::ListCycle<0,T_FunctionList>::next; CHECK( test==1 );
		test=palgo::ListCycle<1,T_FunctionList>::next; CHECK( test==0 );

		auto test1=palgo::ListCycle<0,T_FunctionList>::type::value(4); CHECK( test1==1 );
		auto test2=palgo::ListCycle<1,T_FunctionList>::type::value(4); CHECK( test2==2 );

		float testFloat;
		testFloat=palgo::CyclicList<0,T_FunctionList>::type::value(4); CHECK( testFloat==1 );
		testFloat=palgo::CyclicList<1,T_FunctionList>::type::value(4); CHECK( testFloat==2 );
		testFloat=palgo::CyclicList<2,T_FunctionList>::type::value(4); CHECK( testFloat==1 );
		testFloat=palgo::CyclicList<3,T_FunctionList>::type::value(4); CHECK( testFloat==2 );
		testFloat=palgo::CyclicList<4,T_FunctionList>::type::value(4); CHECK( testFloat==1 );
		testFloat=palgo::CyclicList<5,T_FunctionList>::type::value(4); CHECK( testFloat==2 );
		testFloat=palgo::CyclicList<6,T_FunctionList>::type::value(4); CHECK( testFloat==1 );
		testFloat=palgo::CyclicList<7,T_FunctionList>::type::value(4); CHECK( testFloat==2 );
		testFloat=palgo::CyclicList<8,T_FunctionList>::type::value(4); CHECK( testFloat==1 );
	}
	WHEN( "Testing FunctionList variadic data structure with three entries" )
	{
		typedef palgo::FunctionList< ::Functor1,::Functor2,::Functor3> T_FunctionList;

		// Don't know why, but putting the template directly in the CHECK call messes up the macro
		//CHECK( (palgo::ListCycle<1,T_FunctionList>::previous)==0 );
		//CHECK( palgo::ListCycle<2,T_FunctionList>::previous==1 );

		size_t test;
		test=palgo::ListCycle<0,T_FunctionList>::previous; CHECK( test==2 );
		test=palgo::ListCycle<1,T_FunctionList>::previous; CHECK( test==0 );
		test=palgo::ListCycle<2,T_FunctionList>::previous; CHECK( test==1 );

		test=palgo::ListCycle<0,T_FunctionList>::next; CHECK( test==1 );
		test=palgo::ListCycle<1,T_FunctionList>::next; CHECK( test==2 );
		test=palgo::ListCycle<2,T_FunctionList>::next; CHECK( test==0 );


		auto test1=palgo::ListCycle<0,T_FunctionList>::type::value(4); CHECK( test1==1 );
		auto test2=palgo::ListCycle<1,T_FunctionList>::type::value(4); CHECK( test2==2 );
		auto test3=palgo::ListCycle<2,T_FunctionList>::type::value(4); CHECK( test3==3 );

		float testFloat;
		testFloat=palgo::CyclicList<0,T_FunctionList>::type::value(4); CHECK( testFloat==1 );
		testFloat=palgo::CyclicList<1,T_FunctionList>::type::value(4); CHECK( testFloat==2 );
		testFloat=palgo::CyclicList<2,T_FunctionList>::type::value(4); CHECK( testFloat==3 );
		testFloat=palgo::CyclicList<3,T_FunctionList>::type::value(4); CHECK( testFloat==1 );
		testFloat=palgo::CyclicList<4,T_FunctionList>::type::value(4); CHECK( testFloat==2 );
		testFloat=palgo::CyclicList<5,T_FunctionList>::type::value(4); CHECK( testFloat==3 );
		testFloat=palgo::CyclicList<6,T_FunctionList>::type::value(4); CHECK( testFloat==1 );
		testFloat=palgo::CyclicList<7,T_FunctionList>::type::value(4); CHECK( testFloat==2 );
		testFloat=palgo::CyclicList<8,T_FunctionList>::type::value(4); CHECK( testFloat==3 );
	}
	WHEN( "Testing the SumOfSquares distance functor" )
	{
		typedef palgo::FunctionList< ::XFunctor<TestDataStructure>,::YFunctor<TestDataStructure>,::ZFunctor<TestDataStructure> > T_FunctionList;
		typedef palgo::SumOfSquares<T_FunctionList,TestDataStructure,float> T_DistanceFunctor;

		// Note the function SumOfSquares returns the squared distance. No point in taking the
		// sqrt if all I care about is the order.
		CHECK( T_DistanceFunctor::distance(TestDataStructure(0,0,0),TestDataStructure(1,0,0))==1 );
		CHECK( T_DistanceFunctor::distance(TestDataStructure(0,0,0),TestDataStructure(3,4,0))==25 ); // makes a 3-4-5 triangle
		CHECK( T_DistanceFunctor::distance(TestDataStructure(5,6,7),TestDataStructure(5,9,11))==25 ); // makes a 3-4-5 triangle
	}
}

SCENARIO( "Check that a static_kdtree can be traversed properly" )
{
	GIVEN( "Datatype of single int, partitioned value" )
	{
		typedef std::vector<int> TestData;
		typedef palgo::FunctionList< ::ValueFunctor<TestData::value_type> > T_FunctionList;

		WHEN( "Traversing a tree of 13 entries" )
		{
			TestData input(13);
			// Fill with 0,1,2,3,4,5,...
			size_t generator=0;
			std::generate( input.begin(), input.end(), [&](){return generator++;} );

			// Quick check to make sure I did it properly
			REQUIRE( input[0]==0 ); REQUIRE( input[7]==7 ); REQUIRE( input[11]==11 );

			palgo::static_kdtree<TestData,T_FunctionList> myTree(input,true);

			auto iNode=myTree.root();
			CHECK( *iNode==6 );
			iNode.goto_right_child();
			CHECK( *iNode==10 );
			iNode.goto_right_child();
			CHECK( *iNode==12 );
			iNode.goto_left_child();
			CHECK( *iNode==11 );

			// At the end of the tree, so this should not be allowed to happen.
			// I'm in two minds as to whether this should make the iterator invalid
			// or not move. The implementation is a little easier if it doesn't move
			// so I'll do that for now.
			iNode.goto_left_child();
			//CHECK( iNode==myTree.end() );
			CHECK( *iNode==11 );

			iNode=myTree.root().goto_left_child().goto_right_child();
			CHECK( *iNode==5 );
			iNode.goto_parent();
			CHECK( *iNode==3 );
			iNode.goto_parent();
			CHECK( *iNode==6 ); // Should be back at the root node

			iNode=myTree.root();
			CHECK( iNode.has_parent()==false );
			CHECK( iNode.has_left_child()==true );
			CHECK( iNode.has_right_child()==true );

			iNode.goto_right_child();
			CHECK( *iNode==10 );
			CHECK( iNode.has_parent()==true );
			CHECK( iNode.has_left_child()==true );
			CHECK( iNode.has_right_child()==true );

			iNode.goto_right_child();
			CHECK( *iNode==12 );
			CHECK( iNode.has_parent()==true );
			CHECK( iNode.has_left_child()==true );
			CHECK( iNode.has_right_child()==false );

			iNode.goto_left_child();
			CHECK( *iNode==11 );
			CHECK( iNode.has_parent()==true );
			CHECK( iNode.has_left_child()==false );
			CHECK( iNode.has_right_child()==false );

			iNode.goto_parent();
			CHECK( *iNode==12 );
			CHECK( iNode.has_parent()==true );
			CHECK( iNode.has_left_child()==true );
			CHECK( iNode.has_right_child()==false );

			iNode.goto_parent();
			CHECK( *iNode==10 );
			CHECK( iNode.has_parent()==true );
			CHECK( iNode.has_left_child()==true );
			CHECK( iNode.has_right_child()==true );
		}

		WHEN( "Traversing a tree of 14 entries" )
		{
			TestData input(14);
			// Fill with 0,1,2,3,4,5,...
			size_t generator=0;
			std::generate( input.begin(), input.end(), [&](){return generator++;} );

			// Quick check to make sure I did it properly
			REQUIRE( input[0]==0 ); REQUIRE( input[7]==7 ); REQUIRE( input[11]==11 );

			palgo::static_kdtree<TestData,T_FunctionList> myTree(input,true);

			auto iNode=myTree.root();
			CHECK( *iNode==7 );
			iNode.goto_right_child();
			CHECK( *iNode==11 );
			iNode.goto_right_child();
			CHECK( *iNode==13 );
			iNode.goto_right_child();
			CHECK( iNode==myTree.end() );
		}
	}
}

SCENARIO( "Test the nearest neighbours found are the same as fixed_kdtree" )
{
	GIVEN( "A three dimensional fixed_kdtree and some random data queries" )
	{
		std::vector<TestDataStructure> data(500);
		std::vector<TestDataStructure> queries(100);
		test::fillWithRandoms(data);
		test::fillWithRandoms(queries);

		// The way fixed_kdtree and static_kdtree specify their partitioner functions
		// is different, but the result needs to be the same. Until I implement building
		// static_kdtree properly I need to build first with fixed_kdtree; then take the
		// sorted data and give it to static_kdtree.
		typedef palgo::FunctionList< ::XFunctor<TestDataStructure>,::YFunctor<TestDataStructure>,::ZFunctor<TestDataStructure> > T_FunctionList;

		// Until I implement building the tree in the static_kdtree constructor, I'll
		// use a fixed_kdtree to sort the items into the correct order and re-copy
		// into the original input vector.
		// ToDo - take this out when static_kdtree construction fully implemented.
		std::vector< std::function<float(const TestDataStructure&)> > partitioners;
		partitioners.push_back( XFunctor<TestDataStructure>::value );
		partitioners.push_back( YFunctor<TestDataStructure>::value );
		partitioners.push_back( ZFunctor<TestDataStructure>::value );
		auto correctTree=palgo::make_fixed_kdtree( data.begin(), data.end(), partitioners );
		// Re-copy the ordering back to the original input so that I can create the
		// static_kdtree on top of that.
		for( size_t index=0; index<data.size(); ++index ) data[index]=correctTree.rawData()[index];

		WHEN( "Running nearest neighbour searches serially" )
		{
			// This creates a static_kdtree that uses the treeAccess accessor as the backing store. The
			// data is already sorted into the correct order by fixed_kdtree (hence the second constructor
			// argument that tells static_tree it can skip building).
			palgo::static_kdtree<decltype(data),T_FunctionList> myTree(data,true);
			std::vector<TestDataStructure> results;

			//test::dumpTree(myTree);

			for( const auto query : queries )
			{
				results.push_back( *myTree.nearest_neighbour(query) );
			}

			//
			// Check the results
			//
			REQUIRE( results.size()==queries.size() );
			for( size_t index=0; index<queries.size(); ++index )
			{
				typedef palgo::static_kdtree<decltype(data),T_FunctionList>::distance_functor distance_functor;
				//TestDataStructure correctAnswer=*correctTree.nearest_neighbour_nonrecursive( queries[index] );
				TestDataStructure correctAnswer=*test::bruteForceNearestNeightbour<distance_functor>( data, queries[index] );
				INFO( "Query was " << queries[index] << ". Distance for correct answer=" << distance_functor::distance(correctAnswer,queries[index]) << "; for this answer " << distance_functor::distance(results[index],queries[index]) );
				CHECK( correctAnswer==results[index] );
				//CHECK( correctAnswer.getX()==results[index].getX() );
				//CHECK( correctAnswer.getY()==results[index].getY() );
				//CHECK( correctAnswer.getZ()==results[index].getZ() );
			}
		}
	} // end of "GIVEN three dimensional tree"
}
