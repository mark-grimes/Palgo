/** @file
 *
 * Tests using ComputeCpp to run nearest neighbour searches on GPUs or accelerators; or the CPU
 * if none of those are available. If ComputeCpp could not be found during the cmake configuration
 * then this file does nothing. If ComputeCpp was found, then PALGO_MAKE_SYCL_TESTS will be defined.
 */
#ifdef PALGO_MAKE_SYCL_TESTS
#	include <CL/sycl.hpp>
#endif

#include <palgo/kdtree.hpp> // Haven't implemented building in static_kdtree yet, so use fixed_kdtree for building only
#include <palgo/static_kdtree.hpp>
#include "catch.hpp"

#include <random>
#include <iostream>

namespace // unnamed namespace
{
#ifdef PALGO_MAKE_SYCL_TESTS
	/** @brief SYCL device selector that will choose Xeon Phis */
	class XeonPhiDeviceSelector : public cl::sycl::device_selector
	{
	public:
		virtual int operator()( const cl::sycl::device& device ) const override
		{
			// If it doesn't support the cl_khr_spir extension, can't use it at all
			if( !device.has_extension("cl_khr_spir") ) return -1; // Negative numbers mean it will never be used

			auto type=device.get_info<cl::sycl::info::device::device_type>();
			if( type==cl::sycl::info::device_type::accelerator ) return 100;
			else if( type==cl::sycl::info::device_type::gpu ) return 90;
			else if( type==cl::sycl::info::device_type::cpu )
			{
				// On my system I have the AMD OpenCL implementation that can run on Intel chips,
				// and I also have the Intel OpenCL implementation. I've heard the Intel implementation
				// is much faster (no proof of that though). It also reports larger work group sizes.
				std::string vendor=device.get_platform().get_info<cl::sycl::info::platform::vendor>();
				if( vendor.substr(0,5)=="Intel" ) return 80;
				else return 70;
			}
			else return 1;
		}
	};
#endif // Of "#ifdef PALGO_MAKE_SYCL_TESTS"

	/** @brief a global pseudo-random number generator so that I don't have repeats of number sequences
	 *
	 * Gets set in the first call to "nextRandomNumber". */
	std::function<float()> globalRandomGenerator;

	/** @brief Returns a pseudo-random number, initialising the global generator if required. */
	float nextRandomNumber()
	{
		if( !globalRandomGenerator )
		{
			std::default_random_engine engine;
			typename std::uniform_real_distribution<float> random(0,100);
			globalRandomGenerator=std::bind( random, engine );
		}

		return globalRandomGenerator();
	}

	/** @brief Set the supplied variable to a random number. Specialise the template for custom types. */
	template<class T_value>
	void setRandom( T_value& variable )
	{
		variable=nextRandomNumber();
	}

	/** @brief Overwrites the contents of the supplied container with random values. */
	template<template<typename,typename...> class T_container,class T_value,class... T_args>
	void fillWithRandoms( T_container<T_value,T_args...>& container )
	{
		for( auto& element : container ) setRandom(element);
	}

	/** @brief Just returns the input exactly as received */
	template<class T>
	struct ValueFunctor
	{
		static T value( const T& datapoint )
		{
			return datapoint;
		}
	};

	struct TestData
	{
		float x,y,z;
	};

	/** @brief Template specialisation to set TestData instances to random values */
	template<>
	void setRandom( TestData& variable )
	{
		variable.x=nextRandomNumber();
		variable.y=nextRandomNumber();
		variable.z=nextRandomNumber();
	}

	/** @brief Functor to partition kd-trees using the "x" data member. */
	template<class T_struct,class T_var>
	struct XFunctor
	{
		static T_var value( const T_struct& datapoint )
		{
			return datapoint.x;
		}
	};

	template<class T_struct,class T_var>
	struct YFunctor
	{
		static T_var value( const T_struct& datapoint )
		{
			return datapoint.y;
		}
	};

	template<class T_struct,class T_var>
	struct ZFunctor
	{
		static T_var value( const T_struct& datapoint )
		{
			return datapoint.z;
		}
	};

} // end of the unnamed namespace

SCENARIO( "Attempting to use ComputeCpp to run nearest neighbour searches" )
{
#ifndef PALGO_MAKE_SYCL_TESTS
	std::cerr << "NOTE: SYCL tests not run because it was not configured by cmake to do so (" << __FILE__ << ")." << std::endl;
#else
	GIVEN( "A one dimensional fixed_kdtree and some random data queries" )
	{
		std::vector<float> data(50);
		std::vector<float> queries(10);
		fillWithRandoms(data);
		fillWithRandoms(queries);

		std::vector< std::function<float(const float)> > partitioners;
		partitioners.push_back( [](const float point){ return point; } );
		// The way fixed_kdtree and static_kdtree specify their partitioner functions
		// is different, but the result needs to be the same. Until I implement building
		// static_kdtree properly I need to build first with fixed_kdtree; then take the
		// sorted data and give it to static_kdtree.
		typedef palgo::FunctionList< ::ValueFunctor<float> > T_FunctionList;

		auto myTree=palgo::make_fixed_kdtree( data.begin(), data.end() );

		WHEN( "Running nearest neighbour searches with SYCL and checking the answers against fixed_kdtree" )
		{
			cl::sycl::buffer<float> outputBuffer( cl::sycl::range<1>(queries.size()) );

			{ // Block to make sure the myQueue destructor blocks until all queued items have finished
				cl::sycl::buffer<float> treeBuffer( myTree.rawData(), cl::sycl::range<1>(myTree.size()) );
				cl::sycl::buffer<float> queryBuffer( queries.data(), cl::sycl::range<1>(queries.size()) );

				::XeonPhiDeviceSelector selector;
				cl::sycl::queue myQueue(selector);

				myQueue.submit( [&]( cl::sycl::handler& myHandler )
				{
					auto queryAccess=queryBuffer.get_access<cl::sycl::access::mode::read>(myHandler);
					auto treeAccess=treeBuffer.get_access<cl::sycl::access::mode::read>(myHandler);
					auto outputAccess=outputBuffer.get_access<cl::sycl::access::mode::write>(myHandler);

					// This creates a static_kdtree that uses the treeAccess accessor as the backing store. The
					// data is already sorted into the correct order by fixed_kdtree (hence the second constructor
					// argument that tells static_tree it can skip building).
					palgo::static_kdtree<decltype(treeAccess),T_FunctionList> treeView(treeAccess,true);

					myHandler.parallel_for<class NeighbourSearch>( cl::sycl::range<1>(queryBuffer.get_count()), [=](cl::sycl::item<1> range)
					{
						outputAccess[range]=*treeView.nearest_neighbour( queryAccess[range] );
					} );
				});
			} // End of block to limit scope of myQueue

			//
			// Check the results
			//
			auto outputCheck=outputBuffer.get_access<cl::sycl::access::mode::read,cl::sycl::access::target::host_buffer>();
			size_t correctAnswers=0;
			for( size_t index=0; index<queries.size(); ++index )
			{
				float correctAnswer=*myTree.nearest_neighbour_nonrecursive( queries[index] );
				CHECK( correctAnswer==outputCheck[index] );
			}
		}
	} // end of "GIVEN one dimensional tree"
#endif // Of "#ifndef PALGO_MAKE_SYCL_TESTS"
}
