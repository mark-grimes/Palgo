#ifndef INCLUDEGUARD_test_common_h
#define INCLUDEGUARD_test_common_h

/** @file
*
* Common functions, types and utilities used for testing.
*/

#include <functional>
#include <vector>
#include <cmath>
#include <iostream>
#include <sstream>

//
// Unnamed namespace for things that shouldn't be used outside this file
//
namespace
{
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

} // end of the unnamed namespace

namespace test
{

	/** @brief Returns a pseudo-random number, initialising the global generator if required. */
	float nextRandomNumber();

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

	template<class T>
	void dumpTree( const T& tree )
	{
		std::vector< std::vector< std::pair<std::string,std::string> > > treeLevels;

		auto iRootNode=tree.root();
		if( iRootNode!=tree.end() ) ::dumpTreeNode( iRootNode, 0, 0, false, treeLevels );

		for( const auto& vector : treeLevels )
		{
			for( const auto& pair : vector ) std::cout << pair.first << "-" << pair.second << "  ";
			std::cout << "\n";
		}
	}
} // end of namespace test

#endif
