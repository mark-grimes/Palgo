#ifndef INCLUDEGUARD_palgo_impl_CyclicIterator_hpp
#define INCLUDEGUARD_palgo_impl_CyclicIterator_hpp

#include <iterator>

namespace palgo
{
	namespace impl
	{
		/** @brief Iterator that returns to the start if it runs of the end, or vice versa.
		 *
		 * @author Mark Grimes
		 * @date 07/Sep/2015
		 */
		template<class T>
		class CyclicIterator
		{
		public:
			typedef typename T::difference_type difference_type;
			typedef typename T::value_type value_type;
			typedef value_type* pointer;
			typedef value_type& reference;
			typedef typename T::iterator_category iterator_category;
		public:
			CyclicIterator( T begin, T end ) : current_(begin), begin_(begin), end_(end) {}
			CyclicIterator( T current, T begin, T end ) : current_(current), begin_(begin), end_(end) {}
			bool operator==( const CyclicIterator& otherIterator ) { return current_==otherIterator.current_; }
			bool operator!=( const CyclicIterator& otherIterator ) { return !(*this==otherIterator); }
			value_type operator*() { return *current_; }
			const value_type* operator->() { return &(*current_); }
			CyclicIterator& operator++() { ++current_; if( current_==end_ ) current_=begin_; return *this; }
			CyclicIterator& operator--() { if( current_==begin_ ) current_=end_; --current_; return *this; }
			CyclicIterator operator++(int) { auto result=*this; ++(*this); return result; }
			CyclicIterator operator--(int) { auto result=*this; --(*this); return result; }
			CyclicIterator& operator+=( difference_type value )
			{
				value=value%std::distance(begin_,end_);
				if( current_+value >= end_ ) current_=begin_+(value-std::distance(current_,end_));
				else current_+=value;
				return *this;
			}
			CyclicIterator& operator-=( difference_type value )
			{
				value=value%std::distance(begin_,end_);
				if( begin_+value > current_ ) current_=end_-(value-std::distance(begin_,current_));
				else current_-=value;
				return *this;
			}
			CyclicIterator operator+( difference_type value ) { return CyclicIterator<T>( *this )+=value; }
			CyclicIterator operator-( difference_type value ) { return CyclicIterator<T>( *this )-=value; }
		private:
			T current_,begin_,end_;
		};

	} // end of namespace impl

} // end of namespace palgo

#endif
