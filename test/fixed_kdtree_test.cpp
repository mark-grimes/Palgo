#include <palgo/kdtree.hpp>
#include "catch.hpp"
#include <array>

// TODO remove this include
#include <random>
// TODO remove this include
#include <iostream>

//
// Unnamed namespace for things only used in this file
//
namespace
{
	/** @brief Recursively tests each level of the tree to make sure items are on the correct branch
	 *
	 * Assumes the root node is the one in the middle (or on the right of the centre line if even
	 * number of elements). Checks that everything to the left of that has lower values and everything
	 * to the right has higher values.
	 *
	 * After that, treats everything on the left has a separate tree; and everything on the right as
	 * as separate tree; and calls itself for each of those subtrees.
	 */
	template<class T_iterator,class T_functorIterator>
	void testTree( T_iterator dataBegin, T_iterator dataEnd, T_functorIterator partitionersBegin, T_functorIterator partitionersEnd, T_functorIterator currentPartitioner, std::string branch="" )
	{
		if( std::distance(dataBegin,dataEnd)<2 ) return;

		T_iterator rootNode=dataBegin+(std::distance(dataBegin,dataEnd))/2;

		INFO( "Branch " << branch << ", with root node [" << std::distance(dataBegin,rootNode) << "]"
				<< "{" << (*rootNode)->first << "," << (*rootNode)->second << "}"
				<< std::distance(dataBegin,dataEnd) );
		// Make sure everything in left branch has smaller values than root.
		// Note that the comparison is "<=" because they're sorted with "<", so the root node
		// could have values equal.
		for( T_iterator data=dataBegin; data!=rootNode; ++data ) REQUIRE( (*currentPartitioner)(**data) <= (*currentPartitioner)(**rootNode) );
		// Make sure everything in right branch has larger values than root
		for( T_iterator data=rootNode+1; data!=dataEnd; ++data ) REQUIRE( (*currentPartitioner)(**rootNode) <= (*currentPartitioner)(**data) );

		// Change to the next partitioner ready for the next iteration
		++currentPartitioner;
		if( currentPartitioner==partitionersEnd ) currentPartitioner=partitionersBegin;

		// Now test left subtree. Note that the rootNode is not part of this check since check is on [dataBegin,rootNode)
		testTree( dataBegin, rootNode, partitionersBegin, partitionersEnd, currentPartitioner, branch+"L" );
		// And right subtree
		testTree( rootNode+1, dataEnd, partitionersBegin, partitionersEnd, currentPartitioner, branch+"R" );
	}

	/** @brief Dummy class just to automatically pick between std::uniform_real_distribution and std::uniform_int_distribution */
	template<class T> struct uniform_auto_distribution{ typedef std::uniform_real_distribution<T> type; static const int precisionLimit=3; static constexpr T defaultLow=-5; static constexpr T defaultHigh=5; };
	template<> struct uniform_auto_distribution<int>{ typedef std::uniform_int_distribution<int> type; static const int precisionLimit=-1; static constexpr int defaultLow=-5; static constexpr int defaultHigh=5; };
	template<> struct uniform_auto_distribution<size_t>{ typedef std::uniform_int_distribution<size_t> type; static const int precisionLimit=-1; static constexpr size_t defaultLow=0; static constexpr size_t defaultHigh=5; };

	/** @brief Prints an initialiser list of the given length.
	 *
	 * I Just use this to copy and paste into the hard coded test values. Not used for actual testing.
	 */
	template<class T>
	void printRandomInitialiserList( size_t length, size_t rank, T low, T high, std::ostream& output=std::cout )
	{
		std::default_random_engine engine;
		typename uniform_auto_distribution<T>::type random(low,high);
		output << "TestData data={ ";
		for( size_t index=0; index<length; ++index )
		{
			if( index!=0 ) output << ", ";
			if( rank>1 ) output << "{";
			if( uniform_auto_distribution<T>::precisionLimit>0 ) output << std::setprecision(uniform_auto_distribution<T>::precisionLimit);
			for( size_t rankIndex=0; rankIndex<rank; ++rankIndex )
			{
				if( rankIndex!=0 ) output << ",";
				output << random(engine);
			}
			if( rank>1 ) output << "}";
		}
		output << " };\n";
	}

	template<class T>
	void printRandomInitialiserList( size_t length, size_t rank, std::ostream& output=std::cout )
	{
		printRandomInitialiserList<T>( length, rank, uniform_auto_distribution<T>::defaultLow, uniform_auto_distribution<T>::defaultHigh, std::cout );
	}

	std::ostream& operator<<( std::ostream& output, const std::pair<float,float>& item )
	{
		output << "{" << item.first << "," << item.second << "}";
		return output;
	}

	template<class T_iterator>
	void dumpTreeNode( T_iterator node, size_t level, size_t pairNumber, bool isRightSibling, std::vector< std::vector< std::pair<std::string,std::string> > >& treeLevels )
	{
		if( treeLevels.size()<=level )
		{
			if( level==0 ) treeLevels.push_back( std::vector< std::pair<std::string,std::string> >(1) );
			else treeLevels.push_back( std::vector< std::pair<std::string,std::string> >( std::pow(2,level-1) ) );
		}

		std::stringstream converter;
		converter << *node;

		if( isRightSibling ) treeLevels[level][pairNumber].second=converter.str();
		else treeLevels[level][pairNumber].first=converter.str();

		if( node.has_left_child() ) dumpTreeNode( node.left_child(), level+1, pairNumber*2+isRightSibling, false, treeLevels );
		if( node.has_right_child() ) dumpTreeNode( node.right_child(), level+1, pairNumber*2+isRightSibling, true, treeLevels );
	}

	template<class T>
	void dumpTree( const T& tree )
	{
		std::vector< std::vector< std::pair<std::string,std::string> > > treeLevels;

		auto iRootNode=tree.root();
		if( iRootNode!=tree.end() ) dumpTreeNode( iRootNode, 0, 0, false, treeLevels );

//		// The tree is going to be printed out upside down, since it's easier to work out the spacing.
//		// The last row has no special spacing
//		for( auto& pair : treeLevels.back() )
//		{
//			if( pair.first.empty() ) pair.first="*";
//			if( pair.second.empty() ) pair.second="*";
//			std::cout << pair.first << "-" << pair.second << "  ";
//		}
//		std::cout << "\n";
//		// Then every other row takes it's spacing from the one above
//		for( size_t index=treeLevels.size()-2; index>=0; --index )
//		{
//
//		}

		for( const auto& vector : treeLevels )
		{
			for( const auto& pair : vector ) std::cout << pair.first << "-" << pair.second << "  ";
			std::cout << "\n";
		}
	}
}

SCENARIO( "Test that a kdtree can be instantiated" )
{
	GIVEN( "A std::vector of [x,y] points" )
	{
		typedef std::vector< std::pair<float,float> > TestData;
		TestData input={ {0,0}, {0,1} };

		std::vector< std::function<float(const TestData::value_type&)> > partitioners;
		partitioners.push_back( [](const TestData::value_type& point){ return point.first; } );
		partitioners.push_back( [](const TestData::value_type& point){ return point.second; } );

		//palgo::fixed_kdtree<TestData::const_iterator,> myTree;
		auto myTree=palgo::make_fixed_kdtree( input.begin(), input.end(), partitioners );

		WHEN( "I make sure the test system can fail" )
		{
			// TODO Write some proper tests
			CHECK( 1==1 );
		}
	}
}

SCENARIO( "Check that the constexpr functions work as expected" )
{
	WHEN( "Testing constexpr power function" )
	{
		CHECK( palgo::powerOfTwo(0)==1 );
		CHECK( palgo::powerOfTwo(1)==2 );
		CHECK( palgo::powerOfTwo(2)==4 );
		CHECK( palgo::powerOfTwo(3)==8 );
		CHECK( palgo::powerOfTwo(4)==16 );
		CHECK( palgo::powerOfTwo(5)==32 );
		CHECK( palgo::powerOfTwo(12)==4096 );
	}
	WHEN( "Testing constexpr numberOfElements function" )
	{
		CHECK( palgo::numberOfElements(0)==0 );
		CHECK( palgo::numberOfElements(1)==1 );
		CHECK( palgo::numberOfElements(2)==3 );
		CHECK( palgo::numberOfElements(3)==7 );
		CHECK( palgo::numberOfElements(4)==15 );
		CHECK( palgo::numberOfElements(5)==31 );
		CHECK( palgo::numberOfElements(6)==63 );
		CHECK( palgo::numberOfElements(7)==127 );
		CHECK( palgo::numberOfElements(8)==255 );
		CHECK( palgo::numberOfElements(9)==511 );
		CHECK( palgo::numberOfElements(10)==1023 );
	}
	WHEN( "Testing constexpr numberOfElementsInLevel function" )
	{
		CHECK( palgo::numberOfElementsInLevel(0)==1 );
		CHECK( palgo::numberOfElementsInLevel(1)==2 );
		CHECK( palgo::numberOfElementsInLevel(2)==4 );
		CHECK( palgo::numberOfElementsInLevel(3)==8 );
		CHECK( palgo::numberOfElementsInLevel(4)==16 );
		CHECK( palgo::numberOfElementsInLevel(5)==32 );
		CHECK( palgo::numberOfElementsInLevel(6)==64 );
		CHECK( palgo::numberOfElementsInLevel(7)==128 );
		CHECK( palgo::numberOfElementsInLevel(8)==256 );
		CHECK( palgo::numberOfElementsInLevel(9)==512 );
		CHECK( palgo::numberOfElementsInLevel(10)==1024 );
	}
	WHEN( "Testing constexpr numberOfLevels function" )
	{
		CHECK( palgo::numberOfLevels(0)==0 );
		CHECK( palgo::numberOfLevels(1)==1 );
		CHECK( palgo::numberOfLevels(2)==2 );
		CHECK( palgo::numberOfLevels(3)==2 );
		CHECK( palgo::numberOfLevels(4)==3 );
		CHECK( palgo::numberOfLevels(5)==3 );
		CHECK( palgo::numberOfLevels(6)==3 );
		CHECK( palgo::numberOfLevels(7)==3 );
		CHECK( palgo::numberOfLevels(8)==4 );
		CHECK( palgo::numberOfLevels(15)==4 );
		CHECK( palgo::numberOfLevels(16)==5 );
		CHECK( palgo::numberOfLevels(31)==5 );
		CHECK( palgo::numberOfLevels(32)==6 );
		CHECK( palgo::numberOfLevels(63)==6 );
		CHECK( palgo::numberOfLevels(64)==7 );
	}
	WHEN( "Testing constexpr partitionerIndex function" )
	{
		CHECK( palgo::partitionerIndex(0,1)==0 );
		CHECK( palgo::partitionerIndex(1,1)==0 );
		CHECK( palgo::partitionerIndex(2,1)==0 );
		CHECK( palgo::partitionerIndex(3,1)==0 );

		CHECK( palgo::partitionerIndex(0,3)==0 );
		CHECK( palgo::partitionerIndex(1,3)==1 );
		CHECK( palgo::partitionerIndex(2,3)==2 );
		CHECK( palgo::partitionerIndex(3,3)==0 );
		CHECK( palgo::partitionerIndex(4,3)==1 );
		CHECK( palgo::partitionerIndex(5,3)==2 );
		CHECK( palgo::partitionerIndex(6,3)==0 );
		CHECK( palgo::partitionerIndex(7,3)==1 );
		CHECK( palgo::partitionerIndex(8,3)==2 );
		CHECK( palgo::partitionerIndex(9,3)==0 );

		CHECK( palgo::partitionerIndex(0,4)==0 );
		CHECK( palgo::partitionerIndex(1,4)==1 );
		CHECK( palgo::partitionerIndex(2,4)==2 );
		CHECK( palgo::partitionerIndex(3,4)==3 );
		CHECK( palgo::partitionerIndex(4,4)==0 );
	}
	WHEN( "Test that the constexpr functions are indeed evaluated at compile time" )
	{
		// template arguments must be resolved at compile time, so these force
		// a check that the functions are actually staticly calculated. If not
		// these won't compile.
		std::array<char,palgo::numberOfElements(3)> temp1;
		std::array<char,palgo::numberOfElementsInLevel(3)> temp2;
		std::array<char,palgo::numberOfLevels(3)> temp3;
		std::array<char,palgo::partitionerIndex(2,3)> temp4;
	}
}


SCENARIO( "Check that a kdtree can be traversed properly" )
{
	GIVEN( "Datatype of single int, partitioned value" )
	{
		typedef std::vector<size_t> TestData;

		std::vector< std::function<size_t(const TestData::value_type&)> > partitioners;
		partitioners.push_back( [](const TestData::value_type& point){ return point; } );

		WHEN( "Traversing a tree of 13 entries" )
		{
			TestData input(13);
			// Fill with 0,1,2,3,4,5,...
			size_t generator=0;
			std::generate( input.begin(), input.end(), [&](){return generator++;} );

			// Quick check to make sure I did it properly
			REQUIRE( input[0]==0 ); REQUIRE( input[7]==7 ); REQUIRE( input[11]==11 );

			auto myTree=palgo::make_fixed_kdtree( input.begin(), input.end(), partitioners );

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

			auto myTree=palgo::make_fixed_kdtree( input.begin(), input.end(), partitioners );

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

SCENARIO( "Check that a kdtree subtree iterator works correctly" )
{
	GIVEN( "A tree of (x,y) points" )
	{
		typedef std::vector< std::pair<int,int> > TestData;
		TestData input={ {142,69}, {124,34}, {80,88}, {194,126}, {34,110},
		                 {170,76}, {168,2}, {40,49}, {142,96}, {131,86},
		                 {184,83}, {57,115}, {146,26}, {132,154}, {189,55} };

		std::vector< std::function<float(const TestData::value_type&)> > partitioners;
		partitioners.push_back( [](const TestData::value_type& point){ return point.first; } );
		partitioners.push_back( [](const TestData::value_type& point){ return point.second; } );

		auto myTree=palgo::make_fixed_kdtree( input.begin(), input.end(), partitioners );

		WHEN( "Searching for datapoints" )
		{
			auto iNode=myTree.root().goto_left_child().goto_right_child();
			auto iSubTree=iNode.subtree();
			CHECK( iSubTree->first == iNode->first );
			CHECK( iSubTree->second == iNode->second );
			CHECK( iSubTree.has_left_child() == iNode.has_left_child() );
			CHECK( iSubTree.has_right_child() == iNode.has_right_child() );
			CHECK( iSubTree.has_parent() == false );

			iNode.goto_left_child();
			iSubTree.goto_left_child();
			CHECK( iSubTree->first == iNode->first );
			CHECK( iSubTree->second == iNode->second );
			CHECK( iSubTree.has_left_child() == iNode.has_left_child() );
			CHECK( iSubTree.has_right_child() == iNode.has_right_child() );
			CHECK( iSubTree.has_parent() == iNode.has_parent() );

			// Already checked this, but check again to make sure it traversed properly
			iNode.goto_parent();
			iSubTree.goto_parent();
			CHECK( iSubTree->first == iNode->first );
			CHECK( iSubTree->second == iNode->second );
			CHECK( iSubTree.has_left_child() == iNode.has_left_child() );
			CHECK( iSubTree.has_right_child() == iNode.has_right_child() );
			CHECK( iSubTree.has_parent() == false );

			iNode.goto_right_child();
			iSubTree.goto_right_child();
			CHECK( iSubTree->first == iNode->first );
			CHECK( iSubTree->second == iNode->second );
			CHECK( iSubTree.has_left_child() == iNode.has_left_child() );
			CHECK( iSubTree.has_right_child() == iNode.has_right_child() );
			CHECK( iSubTree.has_parent() == iNode.has_parent() );

			// iSubTree should not be allowed to make the second traversal to the
			// parent. The call should have no effect.
			iNode.goto_parent();
			iNode.goto_parent();
			iSubTree.goto_parent();
			iSubTree.goto_parent();
			CHECK( iSubTree->first != iNode->first );
			CHECK( iSubTree->second != iNode->second );
			CHECK( iSubTree.has_parent() == false );
		}
	}
}

SCENARIO( "Test some kdtree internal functions" )
{
	GIVEN( "Datatype pairs of floats, partitioned first by 'first' then by 'second'" )
	{
		typedef std::vector< std::pair<float,float> > TestData;

		std::vector< std::function<float(const TestData::value_type&)> > partitioners;
		partitioners.push_back( [](const TestData::value_type& point){ return point.first; } );
		partitioners.push_back( [](const TestData::value_type& point){ return point.second; } );

		WHEN( "::sort_kdelements is called with simple input" )
		{
			TestData input={ {0,3}, {1,1}, {1.5,0.5}, {0.6,0} };

			std::vector<TestData::const_iterator> inputIterators;
			for( auto iInput=input.begin(); iInput!=input.end(); ++iInput ) inputIterators.push_back(iInput);

			::sort_kdelements( inputIterators.begin(), inputIterators.end(), partitioners.begin(), partitioners.end() );

			REQUIRE( inputIterators.size() == input.size() );
			REQUIRE( *inputIterators[2] == input[1] ); // root node
			REQUIRE( *inputIterators[1] == input[0] ); // left branch
			REQUIRE( *inputIterators[3] == input[2] ); // right branch
			REQUIRE( *inputIterators[0] == input[3] ); // left branch of next level

			// Recursively test the tree to make sure each left branch has smaller values than the root,
			// and the right branch has larger values.
			INFO( "Calling testTree at line " << __LINE__ )
			::testTree( inputIterators.begin(), inputIterators.end(), partitioners.begin(), partitioners.end(), partitioners.begin() );
		}

		WHEN( "::sort_kdelements is called with simple input that evenly divides branches" )
		{
			TestData input={ {0,3}, {1,1}, {1.5,0.5}, {0.6,0}, {1.2,1.8}, {1.9,0.38}, {1,0.8} };

			std::vector<TestData::const_iterator> inputIterators;
			for( auto iInput=input.begin(); iInput!=input.end(); ++iInput ) inputIterators.push_back(iInput);

			::sort_kdelements( inputIterators.begin(), inputIterators.end(), partitioners.begin(), partitioners.end() );

			REQUIRE( inputIterators.size() == input.size() );
			REQUIRE( *inputIterators[3] == input[6] ); // root node
			REQUIRE( *inputIterators[1] == input[1] ); // left branch
			REQUIRE( *inputIterators[5] == input[2] ); // right branch
			REQUIRE( *inputIterators[0] == input[3] ); // left left branch
			REQUIRE( *inputIterators[2] == input[0] ); // left right branch
			REQUIRE( *inputIterators[4] == input[5] ); // right left branch
			REQUIRE( *inputIterators[6] == input[4] ); // right right branch

			// The tests above have already made sure everything is okay, but I'll
			// also check the ::testTree method, more to test that
			// Recursively test the tree to make sure each left branch has smaller values than the root,
			// and the right branch has larger values.
			INFO( "Calling testTree at line " << __LINE__ )
			::testTree( inputIterators.begin(), inputIterators.end(), partitioners.begin(), partitioners.end(), partitioners.begin() );
		}

		WHEN( "::sort_kdelements is called with power of 2 input" )
		{
			TestData input={ {-5,-4.15},   {1.01,3.92},  {4.68,-3.1},    {0.15,-1.02},
			                 {-2.37,2.44}, {-4.1,0.604}, {0.822,3.1},    {0.919,0.117},
			                 {3.77,4.95},  {2.26,4.67},  {-2.03,-0.739}, {3.99,1.53},
			                 {4.02,4.62},  {-3.35,3.58}, {4.07,-2.06},   {4.36,-0.854} };

			std::vector<TestData::const_iterator> inputIterators;
			for( auto iInput=input.begin(); iInput!=input.end(); ++iInput ) inputIterators.push_back(iInput);

			::sort_kdelements( inputIterators.begin(), inputIterators.end(), partitioners.begin(), partitioners.end() );
			REQUIRE( inputIterators.size() == input.size() );

			// Recursively test the tree to make sure each left branch has smaller values than the root,
			// and the right branch has larger values.
			INFO( "Calling testTree at line " << __LINE__ )
			::testTree( inputIterators.begin(), inputIterators.end(), partitioners.begin(), partitioners.end(), partitioners.begin() );
		}
	}
}

SCENARIO( "Check that a kdtree can find insertion points correctly" )
{
	GIVEN( "A tree of (x,y) points" )
	{
		typedef std::vector< std::pair<float,float> > TestData;
		TestData input={ {-5,-4.15}, {1.01,3.92}, {4.68,-3.1},
		                 {0.15,-1.02}, {-2.37,2.44}, {-4.1,0.604},
		                 {0.822,3.1}, {0.919,0.117}, {3.77,4.95},
		                 {2.26,4.67}, {-2.03,-0.739}, {3.99,1.53},
		                 {4.02,4.62}, {-3.35,3.58}, {4.07,-2.06} };

		std::vector< std::function<float(const TestData::value_type&)> > partitioners;
		partitioners.push_back( [](const TestData::value_type& point){ return point.first; } );
		partitioners.push_back( [](const TestData::value_type& point){ return point.second; } );

		auto myTree=palgo::make_fixed_kdtree( input.begin(), input.end(), partitioners );

		WHEN( "Searching for datapoints" )
		{
			auto iNearest=myTree.lowest_neighbour( std::make_pair(0,0) );
			CHECK( iNearest->first == Approx(0.15) );
			CHECK( iNearest->second == Approx(-1.02) );

			iNearest=myTree.lowest_neighbour( std::make_pair(2.35,-4.2) );
			CHECK( iNearest->first == Approx(3.99) );
			CHECK( iNearest->second == Approx(1.53) );

			iNearest=myTree.lowest_neighbour( std::make_pair(0.3,1.13) );
			CHECK( iNearest->first == Approx(0.822) );
			CHECK( iNearest->second == Approx(3.1) );
		}
	}

	GIVEN( "A tree of (x,y) points evenly distributed around a square" )
	{
		typedef std::vector< std::pair<float,float> > TestData;
		TestData input={ {-5,-5}, {-5,5}, {5,-5}, {5,5},
		                 {-3,-3}, {-3,3}, {3,-3}, {3,3},
		                 {0,0} };

		std::vector< std::function<float(const TestData::value_type&)> > partitioners;
		partitioners.push_back( [](const TestData::value_type& point){ return point.first; } );
		partitioners.push_back( [](const TestData::value_type& point){ return point.second; } );

		auto myTree=palgo::make_fixed_kdtree( input.begin(), input.end(), partitioners );

		WHEN( "Searching for datapoints" )
		{
			auto iNearest=myTree.lowest_neighbour( std::make_pair(1,1) );
			CHECK( iNearest->first == Approx(3) );
			CHECK( iNearest->second == Approx(-3) );

			iNearest=myTree.lowest_neighbour( std::make_pair(-1,1) );
			CHECK( iNearest->first == Approx(-3) );
			CHECK( iNearest->second == Approx(-3) );
		}
	}
}

SCENARIO( "Check that the kdtree distance_squared method works correctly" )
{
	GIVEN( "A tree of (x,y) points" )
	{
		typedef std::vector< std::pair<float,float> > TestData;
		TestData input={ {-5,-4.15}, {1.01,3.92} }; // Doesn't really matter about the input, it's not used for this test

		std::vector< std::function<float(const TestData::value_type&)> > partitioners;
		partitioners.push_back( [](const TestData::value_type& point){ return point.first; } );
		partitioners.push_back( [](const TestData::value_type& point){ return point.second; } );

		auto myTree=palgo::make_fixed_kdtree( input.begin(), input.end(), partitioners );

		WHEN( "Querying distances between two points" )
		{
			CHECK( myTree.distance_squared( std::make_pair(0,0), std::make_pair(0,0) ) == 0 );
			CHECK( myTree.distance_squared( std::make_pair(0,1), std::make_pair(0,0) ) == 1 );
			CHECK( myTree.distance_squared( std::make_pair(0,2), std::make_pair(0,0) ) == 4 );
			CHECK( myTree.distance_squared( std::make_pair(1,0), std::make_pair(0,0) ) == 1 );
			CHECK( myTree.distance_squared( std::make_pair(2,0), std::make_pair(0,0) ) == 4 );
			CHECK( myTree.distance_squared( std::make_pair(1,1), std::make_pair(0,0) ) == 2 );
			CHECK( myTree.distance_squared( std::make_pair(2,2), std::make_pair(0,0) ) == 8 );
			CHECK( myTree.distance_squared( std::make_pair(-2,2), std::make_pair(0,0) ) == 8 );
			CHECK( myTree.distance_squared( std::make_pair(2,-2), std::make_pair(0,0) ) == 8 );
			CHECK( myTree.distance_squared( std::make_pair(-2,-2), std::make_pair(0,0) ) == 8 );

			CHECK( myTree.distance_squared( std::make_pair(-1.45,9.23), std::make_pair(4.23,-2.4) ) == Approx(167.5193) );
		}
	}
}

SCENARIO( "Check that a kdtree can find nearest neighbours correctly" )
{
	GIVEN( "A tree of (x) points" )
	{
		typedef std::vector<int> TestData;
		TestData input={ 14, 33, 5, 48, 34, 16, 24, 2, 34, 46, 42, 12 };
		auto myTree=palgo::make_fixed_kdtree( input.begin(), input.end() );

		WHEN( "Searching for datapoints" )
		{
			auto iNearest=myTree.nearest_neighbour(0);
			CHECK( *iNearest == 2 );

			iNearest=myTree.nearest_neighbour(2);
			CHECK( *iNearest == 2 );

			iNearest=myTree.nearest_neighbour(4);
			CHECK( *iNearest == 5 );

			iNearest=myTree.nearest_neighbour(15);
			CHECK( *iNearest == 16 ); // This is a bit ambiguous. 14 would also be valid.
		}
	}

	GIVEN( "A tree of (x,y) points" )
	{
		//printRandomInitialiserList<int>( 15, 2, 0, 200 );
		typedef std::vector< std::pair<int,int> > TestData;

		TestData input={ {142,69}, {124,34}, {80,88}, {194,126}, {34,110}, {170,76}, {168,2}, {40,49}, {142,96}, {131,86}, {184,83}, {57,115} };

		std::vector< std::function<float(const TestData::value_type&)> > partitioners;
		partitioners.push_back( [](const TestData::value_type& point){ return point.first; } );
		partitioners.push_back( [](const TestData::value_type& point){ return point.second; } );

		auto myTree=palgo::make_fixed_kdtree( input.begin(), input.end(), partitioners );

		WHEN( "Searching for datapoints that exactly match the input finds the correct point" )
		{
			// This just makes sure that all branches of the tree can be traversed down
			for( const auto& inputPair : input )
			{
				INFO( "Checking for exact match of {" << inputPair.first << "," << inputPair.second << "}" );
				auto iNearest=myTree.nearest_neighbour( inputPair );
				CHECK( iNearest->first == inputPair.first );
				CHECK( iNearest->second == inputPair.second );
			}
		}

		WHEN( "Searching for datapoints with approximate matches" )
		{
			auto iNearest=myTree.nearest_neighbour( std::make_pair(41,49) );
			CHECK( iNearest->first == 40 );
			CHECK( iNearest->second == 49 );

			iNearest=myTree.nearest_neighbour( std::make_pair(58,115) );
			CHECK( iNearest->first == 57 );
			CHECK( iNearest->second == 115 );

			iNearest=myTree.nearest_neighbour( std::make_pair(146,60) );
			CHECK( iNearest->first == 142 );
			CHECK( iNearest->second == 69 );

			iNearest=myTree.nearest_neighbour( std::make_pair(141,97) );
			CHECK( iNearest->first == 142 );
			CHECK( iNearest->second == 96 );
		}
	}

}
