#include <palgo/kdtree.hpp>
#include "catch.hpp"

#include <chrono>
#include <random>
#include <iostream>

template<class T_Rep,class T_Period>
double durationInSeconds( std::chrono::duration<T_Rep,T_Period> time )
{
	double count=time.count();
	return (count*T_Period::num)/T_Period::den;
}

SCENARIO( "Running nearest neighbour checks repeatedly to test runtime", "[speed][hide]" )
{
	GIVEN( "A kdtree" )
	{
		constexpr size_t datasetSize=1000000, testsetSize=300, numIterations=100;

		typedef std::vector< std::pair<float,float> > TestData;
		std::uniform_real_distribution<float> uniformDistribution( 0, 500 );
		std::default_random_engine random;

		TestData dataset;
		for( size_t index=0; index<datasetSize; ++index ) dataset.push_back( {uniformDistribution(random),uniformDistribution(random)} );

		std::vector< std::function<float(const TestData::value_type&)> > partitioners;
		partitioners.push_back( [](const TestData::value_type& point){ return point.first; } );
		partitioners.push_back( [](const TestData::value_type& point){ return point.second; } );

		auto startTime=std::chrono::high_resolution_clock::now();
		auto myTree=palgo::make_fixed_kdtree( dataset.begin(), dataset.end(), partitioners );
		auto finishTime=std::chrono::high_resolution_clock::now();
		std::cout << "Building tree took        " << durationInSeconds(finishTime-startTime) << "s" << std::endl;

		TestData testset;
		for( size_t index=0; index<testsetSize; ++index ) testset.push_back( {uniformDistribution(random),uniformDistribution(random)} );

		TestData recursiveResult(testsetSize);
		TestData nonrecursiveResult(testsetSize);
		std::vector<decltype(myTree)::const_iterator> batchResult;

		WHEN( "Running non recursive searches" )
		{
			auto startTime=std::chrono::high_resolution_clock::now();
			for( size_t indexA=0; indexA<numIterations; ++indexA )
			{
				for( size_t indexB=0; indexB<testsetSize; ++indexB ) nonrecursiveResult[indexB]=*myTree.nearest_neighbour_nonrecursive( testset[indexB] );
			}

			auto finishTime=std::chrono::high_resolution_clock::now();
			std::cout << "Non recursive search took " << durationInSeconds(finishTime-startTime) << "s" << std::endl;
		}
		WHEN( "Running recursive searches" )
		{
			auto startTime=std::chrono::high_resolution_clock::now();
			for( size_t indexA=0; indexA<numIterations; ++indexA )
			{
				for( size_t indexB=0; indexB<testsetSize; ++indexB ) recursiveResult[indexB]=*myTree.nearest_neighbour( testset[indexB] );
			}

			auto finishTime=std::chrono::high_resolution_clock::now();
			std::cout << "Recursive search took     " << durationInSeconds(finishTime-startTime) << "s" << std::endl;
		}
		WHEN( "Running batch searches" )
		{
			auto startTime=std::chrono::high_resolution_clock::now();
			for( size_t indexA=0; indexA<numIterations; ++indexA )
			{
				batchResult=myTree.nearest_neighbour( testset );
			}

			auto finishTime=std::chrono::high_resolution_clock::now();
			std::cout << "Batch search took         " << durationInSeconds(finishTime-startTime) << "s" << std::endl;
		}
		WHEN( "Making sure both algorithms get the same result" )
		{
			for( size_t index=0; index<testsetSize; ++index )
			{
				// Have to run the algorithms again, since other "WHEN" cases are in a different scope.
				nonrecursiveResult[index]=*myTree.nearest_neighbour_nonrecursive( testset[index] );
				recursiveResult[index]=*myTree.nearest_neighbour( testset[index] );
				CHECK( recursiveResult[index]==nonrecursiveResult[index] );
			}
		}
		WHEN( "Making sure batch gets the same result" )
		{
			// Have to run the algorithms again, since other "WHEN" cases are in a different scope.
			batchResult=myTree.nearest_neighbour( testset );
			for( size_t indexB=0; indexB<testsetSize; ++indexB ) recursiveResult[indexB]=*myTree.nearest_neighbour( testset[indexB] );

			CHECK( batchResult.size()==testsetSize );
			for( size_t index=0; index<recursiveResult.size() && index<batchResult.size() ; ++index ) CHECK( recursiveResult[index]==*batchResult[index] );
		}
	}
}
