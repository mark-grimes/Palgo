#ifndef INCLUDEGUARD_palgo_kdtree_hpp
#define INCLUDEGUARD_palgo_kdtree_hpp

#include <vector>
#include <cmath>

// TODO remove once fully tested
#include <iostream>

namespace palgo
{
	/** @brief A kd-tree that does not allow insertion after initial creation.
	 * @author Mark Grimes
	 * @date 25/Aug/2015
	 */
	template<class T_iterator,class T_functor>
	class fixed_kdtree
	{
	private:
		typedef std::vector<typename T_iterator::value_type> container_type;
	public:
		class Iterator
		{
			typedef typename std::iterator_traits<typename container_type::const_iterator>::difference_type difference_type;
			typedef typename container_type::value_type value_type;
			typedef value_type* pointer;
			typedef value_type& reference;
			typedef typename std::iterator_traits<typename container_type::const_iterator>::iterator_category iterator_category;
		public:
			bool operator==( const Iterator& otherIterator ) { return iterator_==otherIterator.iterator_; }
			bool operator!=( const Iterator& otherIterator ) { return !(*this==otherIterator); }
			Iterator& goto_parent()
			{
				if( parentStack_.empty() ) return *this;
				if( parentStack_.back() > subTreeFarRight_ ) subTreeFarRight_=parentStack_.back();
				else subTreeFarLeft_=parentStack_.back();
				parentStack_.pop_back();
				setIterator();
				return *this;
			}
			Iterator& goto_left_child() { parentStack_.push_back(subTreeFarRight_); subTreeFarRight_=iterator_; setIterator(); return *this; }
			Iterator& goto_right_child() { parentStack_.push_back(subTreeFarLeft_); subTreeFarLeft_=iterator_+1; setIterator(); return *this; }
			Iterator parent() { return Iterator(*this).goto_parent(); }
			Iterator left_child() { return Iterator(*this).goto_left_child(); }
			Iterator right_child() { return Iterator(*this).goto_right_child(); }
			Iterator subtree() { Iterator returnValue(*this); returnValue.parentStack_.clear(); return returnValue; }
			value_type operator*() { return *iterator_; }
			const value_type* operator->() { return &(*iterator_); }
			bool has_parent() { return !parentStack_.empty(); }
			bool has_left_child() { return iterator_!=subTreeFarLeft_; }
			bool has_right_child() { return iterator_+1!=subTreeFarRight_; }
		public:
			Iterator& operator=( const Iterator& other )
			{
				subTreeFarLeft_=other.subTreeFarLeft_;
				subTreeFarRight_=other.subTreeFarRight_;
				parentStack_=other.parentStack_;
				setIterator();
				return *this;
			}
			Iterator& operator=( Iterator&& other ) = default;
			Iterator( const Iterator& other ) = default;
			Iterator( Iterator&& other ) = default;
		public:
			Iterator( typename container_type::const_iterator leftLimit, typename container_type::const_iterator rightLimit, const container_type& container )
				: container_(container), subTreeFarLeft_(leftLimit), subTreeFarRight_(rightLimit) { setIterator(); }
			void dumpPosition( std::string prefix="" )
			{
				std::cout << prefix << "(" << *subTreeFarLeft_ << ","
						<< *iterator_ << ",";
				if( subTreeFarRight_==container_.end() ) std::cout << "*" << ")\n";
				else std::cout << *subTreeFarRight_ << ")\n";
			}
		private:
			inline void setIterator() { iterator_=subTreeFarLeft_+std::distance(subTreeFarLeft_,subTreeFarRight_)/2; }
			inline Iterator current() { return subTreeFarLeft_+std::distance(subTreeFarLeft_,subTreeFarRight_)/2; }
			typename container_type::const_iterator iterator_;
			typename container_type::const_iterator subTreeFarLeft_,subTreeFarRight_;
			const std::vector<typename T_iterator::value_type>& container_;
			std::vector<typename container_type::const_iterator> parentStack_;
		};
	public:
		typedef Iterator const_iterator;
		typedef typename T_iterator::value_type value_type;
	public:
		/** @brief Constructor from data iterators and a vector of functors that provide the metric to partition
		 *
		 * The dimensionality of the tree is the number of functors passed.
		 */
		fixed_kdtree( T_iterator begin, T_iterator end, std::vector<T_functor> partitioners );
		Iterator root() const { return Iterator( data_.begin(), data_.end(), data_ ); }
		Iterator end() const { return Iterator( data_.end(), data_.end(), data_ ); }
		Iterator nearest_neighbour( value_type );
		Iterator lowest_neighbour( value_type );
		typename T_functor::result_type distance_squared( value_type pointA, value_type pointB );
	private:
		Iterator nearest_neighbour_subSearch( value_type searchData, Iterator iSubTree, typename T_functor::result_type currentBest );
		std::vector<typename T_iterator::value_type> data_;
		std::vector<T_functor> partitioners_;
	};

	/** @brief Helper function so that template parameters can be automatically deduced
	 * @author Mark Grimes
	 * @date 25/Aug/2015
	 */
	template<class T_iterator,class T_functor>
	fixed_kdtree<T_iterator,T_functor> make_fixed_kdtree( T_iterator begin, T_iterator end, std::vector<T_functor> partitioners );

	/** @brief Special overload where items are only sorted on their value (i.e. 1D tree)
	 * @author Mark Grimes
	 * @date 02/Sep/2015
	 */
	template<class T_iterator>
	fixed_kdtree<T_iterator,std::function<typename T_iterator::value_type(const typename T_iterator::value_type&)> > make_fixed_kdtree( T_iterator begin, T_iterator end );



	/** @brief Helper function. Returns 2 to the given power. */
	constexpr size_t powerOfTwo( size_t power ) { return power==0 ? 1 : 2*powerOfTwo(power-1); }
	/** @brief Returns the maximum number of elements that can be contained in a tree with the given number of levels.
	 *
	 * Note that counting starts at 1, so that a tree with 1 level just has a single root node. */
	constexpr size_t numberOfElements( size_t levels ) { return powerOfTwo(levels)-1; }
	/** @brief Returns the maximum number of elements that can be contained in a particular level of the tree.
	 *
	 * Note that counting starts at zero, so the root level is 0. */
	constexpr size_t numberOfElementsInLevel( size_t level ) { return powerOfTwo(level); }
	/** @brief Returns the number of levels required to hold the given number of elements */
	constexpr size_t numberOfLevels( size_t elements ) { return elements==0 ? 0 : 1+numberOfLevels( elements/2 ); }
} // end of namespace palgo


//
// Implementations of all the definitions above
//


namespace
{
	template<class T_iterator,class T_functor>
	void split( std::vector<T_iterator>& input, std::vector<T_iterator>& leftOutput, std::vector<T_iterator>& rightOutput, T_functor partitioner )
	{
		std::sort( input.begin(), input.end(), [=](T_iterator A, T_iterator B){ return partitioner(*A)<partitioner(*B); } );

		typename std::vector<T_iterator>::iterator iLeftStart=input.begin();
		typename std::vector<T_iterator>::iterator iLeftEnd=input.end()-input.size()/2;
		leftOutput.insert( leftOutput.end(), iLeftStart, iLeftEnd );
		rightOutput.insert( rightOutput.end(), iLeftEnd, input.end() );
	}

	template<class T_iterIterator,class T_functorIter>
	void sort_kdelements( const T_iterIterator& inputBegin, const T_iterIterator& inputEnd, const T_functorIter& partitionersBegin, const T_functorIter& partitionersEnd, T_functorIter currentPartitioner )
	{
		//T_functorIter currentPartitioner=partitionersBegin; // current partitioner
		T_iterIterator iCurrentEnd=inputEnd;

		while( inputBegin!=iCurrentEnd )
		{
			typedef typename T_iterIterator::value_type T_iter;
			std::sort( inputBegin, iCurrentEnd, [=](T_iter A,T_iter B){return (*currentPartitioner)(*A)<(*currentPartitioner)(*B);} );

			// This level has been partitioned, so the start of the container has the
			// left branch of the tree, and the end the right. Now I need to descend
			// and partition the next level. I'll do the left branch in this function,
			// the right branch needs a recursive call.
			auto iOldEnd=iCurrentEnd;
			iCurrentEnd=inputBegin+std::distance(inputBegin,iCurrentEnd)/2;
			++currentPartitioner;
			if( currentPartitioner==partitionersEnd ) currentPartitioner=partitionersBegin;
			// recursive call on [iCurrentEnd+1,inputEnd) to do the right branch (+1 to skip over root node)
			// TODO - find a way to allow frameworks to run this call concurrently
			sort_kdelements( iCurrentEnd+1, iOldEnd, partitionersBegin, partitionersEnd, currentPartitioner );
		}
	}

	template<class T_iterIterator,class T_functorIter>
	void sort_kdelements( const T_iterIterator& inputBegin, const T_iterIterator& inputEnd, const T_functorIter& partitionersBegin, const T_functorIter& partitionersEnd )
	{
		// Forward the call, just with currentPartitioner set to partitionersBegin
		sort_kdelements( inputBegin, inputEnd, partitionersBegin, partitionersEnd, partitionersBegin );
	}

} // end of the unnamed namespace

template<class T_iterator,class T_functor>
palgo::fixed_kdtree<T_iterator,T_functor>::fixed_kdtree( T_iterator begin, T_iterator end, std::vector<T_functor> partitioners )
	: partitioners_(partitioners)
{
	if( partitioners.empty() ) throw std::runtime_error("palgo::fixed_kdtree constructed with empty partitioners");

	auto iPartitioner=partitioners.begin();

	// Create a vector of iterators to the original data
	std::vector<T_iterator> inputData;
	for( ; begin!=end; ++begin ) inputData.push_back( begin );

	sort_kdelements( inputData.begin(), inputData.end(), partitioners.begin(), partitioners.end() );

	// Now I have iterators to the data in the correct order, I'll use these to copy
	// these elements into the class in the correct place.
	data_.reserve( std::distance(begin,end) );
	for( const auto& iData : inputData ) data_.emplace_back( *iData );
}

template<class T_iterator,class T_functor>
palgo::fixed_kdtree<T_iterator,T_functor> palgo::make_fixed_kdtree( T_iterator begin, T_iterator end, std::vector<T_functor> partitioners )
{
	return palgo::fixed_kdtree<T_iterator,T_functor>(begin,end,partitioners);
}

template<class T_iterator>
palgo::fixed_kdtree<T_iterator,std::function<typename T_iterator::value_type(const typename T_iterator::value_type&)> > palgo::make_fixed_kdtree( T_iterator begin, T_iterator end )
{
	typedef std::function<typename T_iterator::value_type(const typename T_iterator::value_type&)> T_functor;
	std::vector<T_functor> partitioners(1,[](const typename T_iterator::value_type& data){ return data; } );

	return palgo::fixed_kdtree<T_iterator,T_functor>(begin,end,partitioners);
}

template<class T_iterator,class T_functor>
typename palgo::fixed_kdtree<T_iterator,T_functor>::const_iterator palgo::fixed_kdtree<T_iterator,T_functor>::nearest_neighbour( typename palgo::fixed_kdtree<T_iterator,T_functor>::value_type datapoint )
{
	typedef typename T_functor::result_type T_distance;
	//std::cout << "Checking for " << datapoint.first << "," << datapoint.second << std::endl;

	Iterator iCurrent=root();
	std::vector<bool> isLeftChild;
	auto iCurrentPartitioner=partitioners_.begin();

	while( true )
	{
		if( (*iCurrentPartitioner)(datapoint) < (*iCurrentPartitioner)(*iCurrent) )
		{
			if( !iCurrent.has_left_child() ) break;
			iCurrent.goto_left_child();
			isLeftChild.push_back(true);
		}
		else
		{
			if( !iCurrent.has_right_child() ) break;
			iCurrent.goto_right_child();
			isLeftChild.push_back(false);
		}

		++iCurrentPartitioner;
		if( iCurrentPartitioner==partitioners_.end() ) iCurrentPartitioner=partitioners_.begin();
	}


	std::pair<Iterator,T_distance> bestMatch=std::make_pair( iCurrent, distance_squared( datapoint, *iCurrent ) );

	while( iCurrent.has_parent() )
	{
		iCurrent.goto_parent();
		if( iCurrentPartitioner==partitioners_.begin() ) iCurrentPartitioner=partitioners_.end();
		--iCurrentPartitioner;

		T_distance distance=distance_squared( datapoint, *iCurrent );

		if( distance<bestMatch.second )
		{
			bestMatch.first=iCurrent;
			bestMatch.second=distance;
		}

		auto iOtherSplittingPlane=iCurrent;
		if( isLeftChild.back() )
		{
			if( !iOtherSplittingPlane.has_right_child() ) continue;
			iOtherSplittingPlane.goto_right_child();
		}
		else
		{
			if( !iOtherSplittingPlane.has_left_child() ) continue;
			iOtherSplittingPlane.goto_left_child();
		}

		if( std::pow( (*iCurrentPartitioner)(*iOtherSplittingPlane), 2 )<bestMatch.second )
		{
			auto iSubResult=nearest_neighbour_subSearch( datapoint, iOtherSplittingPlane, bestMatch.second );
			if( iSubResult!=end() )
			{
				bestMatch.first=iSubResult;
				bestMatch.second=distance_squared( datapoint, *iSubResult );
			}
		}
	}

	return bestMatch.first; // return the iterator to the best match
}

template<class T_iterator,class T_functor>
typename palgo::fixed_kdtree<T_iterator,T_functor>::const_iterator palgo::fixed_kdtree<T_iterator,T_functor>::nearest_neighbour_subSearch( value_type searchData, typename palgo::fixed_kdtree<T_iterator,T_functor>::const_iterator iSubTree, typename T_functor::result_type currentBest )
{
	return end();
}

template<class T_iterator,class T_functor>
typename palgo::fixed_kdtree<T_iterator,T_functor>::const_iterator palgo::fixed_kdtree<T_iterator,T_functor>::lowest_neighbour( typename palgo::fixed_kdtree<T_iterator,T_functor>::value_type datapoint )
{
	Iterator returnValue=root();
	auto iCurrentPartitioner=partitioners_.begin();

	while( true )
	{
		if( (*iCurrentPartitioner)(datapoint) < (*iCurrentPartitioner)(*returnValue) )
		{
			if( returnValue.has_left_child() ) returnValue.goto_left_child();
			else return returnValue;
		}
		else
		{
			if( returnValue.has_right_child() ) returnValue.goto_right_child();
			else return returnValue;
		}

		++iCurrentPartitioner;
		if( iCurrentPartitioner==partitioners_.end() ) iCurrentPartitioner=partitioners_.begin();
	}
}

template<class T_iterator,class T_functor>
typename T_functor::result_type palgo::fixed_kdtree<T_iterator,T_functor>::distance_squared( typename palgo::fixed_kdtree<T_iterator,T_functor>::value_type pointA, typename palgo::fixed_kdtree<T_iterator,T_functor>::value_type pointB )
{
	typename T_functor::result_type distanceSquared=0;
	for( const auto& partitioner : partitioners_ )
	{
		distanceSquared+=( std::pow(partitioner(pointA)-partitioner(pointB),2) );
	}

	return distanceSquared;
}

#endif
