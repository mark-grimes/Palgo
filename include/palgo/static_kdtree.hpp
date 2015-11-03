#ifndef INCLUDEGUARD_palgo_static_kdtree_hpp
#define INCLUDEGUARD_palgo_static_kdtree_hpp

/** @file
 * @brief A test of specifying the paritioners statically using the template arguments.
 *
 * Still in alpha.
 */

#include <array>
#include <vector>

// TODO - Remove this include once fully tested
#include <iostream>

//
// Unnamed namespace for things only used in this file
//
namespace
{
	/** @brief Function to return the number of elements in a container.
	 *
	 * Required because cl::sycl::accessor doesn't have a "size" method; it has a "get_count"
	 * method instead ("get_size" is get_count*sizeof(T)). This is the default for normal STL
	 * containers, the SYCL accessor special case will be in a specialisation. */
	template<class T>
	inline size_t dataSize( const T& container )
	{
		return container.size();
	}

	/** @brief Info struct to get the type of the containers elements.
	 *
	 * Required because cl::sycl::accessor doesn't have a value_type. This is the default for
	 * normal STL containers, the SYCL accessor special case will be in a specialisation. */
	template<class T_container>
	struct ContainerTypeInfo
	{
		typedef typename T_container::value_type value_type;
	};

// If ComputeCpp is being used, add template specialisations
#ifdef RUNTIME_INCLUDE_SYCL_ACCESSOR_H_ // This is the include guard for SYCL/accessor.h
	template<class T_dataType,int dimensions,cl::sycl::access::mode accessMode,cl::sycl::access::target target>
	inline size_t dataSize( const cl::sycl::accessor<T_dataType,dimensions,accessMode,target>& container )
	{
		return container.get_count();
	}

	/** @brief Template specialisation to retrieve the type of a SYCL accessor's elements. */
	template<class T_dataType,int dimensions,cl::sycl::access::mode accessMode,cl::sycl::access::target target>
	struct ContainerTypeInfo<cl::sycl::accessor<T_dataType,dimensions,accessMode,target> >
	{
		typedef T_dataType value_type;
	};
#endif

} // end of the unnamed namespace

namespace palgo
{

	template<typename T_data, float (*... V_partitioners)(const T_data&)>
	class test_tree
	{
	public:
		template<typename T_iterator>
		test_tree( T_iterator iBegin, T_iterator iEnd )
		{
			for( ; iBegin!=iEnd; ++iBegin ) items_.emplace_back( *iBegin );
		}
		std::vector<float> testMethod()
		{
			std::vector<float> returnValue;
			for( size_t index=0; index<items_.size(); ++index )
			{
				returnValue.push_back( partitioners_[index%partitioners_.size()](items_[index]) );
			}
			return returnValue;
		}
	private:
		std::vector<T_data> items_;
		static const std::array<float (*)(const T_data&),sizeof...(V_partitioners)> partitioners_;
	//	std::tuple<T_partitioners...> partitioners_;
	};

	template<typename T_data, float (*... V_partitioners)(const T_data&)>
	const std::array<float (*)(const T_data&),sizeof...(V_partitioners)> test_tree<T_data,V_partitioners...>::partitioners_={V_partitioners...};

	//template<typename T_data, float (*... Vs_function)(const T_data&)> struct FunctionList {};
	//
	//template<typename T_data, float (*V_function)(const T_data&), float (*... Vs_function)(const T_data&)>
	//struct FunctionList : FunctionList<T_data,Vs_function>
	//{
	//	FunctionList( float (*V_function)(const T_data&), float (*... Vs_function)(const T_data&) )
	//		: FunctionList<T_data,Vs_function>(Vs_function), tail(V_function)
	//	{
	//	}
	//	float (*V_function)(const T_data&) tail;
	//};

	template<typename T_data, typename T_sort, float (*... V_functions)(const T_data&)>
	class test_tree2
	{
	public:
		template<typename T_iterator>
		test_tree2( T_iterator iBegin, T_iterator iEnd )
		{
			for( ; iBegin!=iEnd; ++iBegin ) items_.emplace_back( *iBegin );
		}
		bool testMethod()
		{
			return true;
		}
	private:
		std::vector<T_data> items_;
		static const std::array<float (*)(const T_data&),sizeof...(V_functions)> partitioners_;
	};

	template<typename T_data, typename T_sort, float (*... V_functions)(const T_data&)>
	const std::array<float (*)(const T_data&),sizeof...(V_functions)> test_tree2<T_data,T_sort,V_functions...>::partitioners_={V_functions...};

	template<typename... T_functions>
	struct FunctionList
	{
		// intentionally empty base class
	};

	template<typename T,typename... Ts>
	struct FunctionList<T,Ts...> : public FunctionList<Ts...>
	{
		typedef T func;
	};

	//-------------------------------------------

	template <size_t,class T, class... Ts>
	struct TypeHolder;

	template <class T, class... Ts>
	struct TypeHolder<0, FunctionList<T, Ts...> >
	{
		typedef T type;
	};

	template <size_t k, class T, class... Ts>
	struct TypeHolder<k, FunctionList<T, Ts...> >
	{
		typedef typename TypeHolder<k - 1, FunctionList<Ts...>>::type type;
	};

	//-------------------------------------------
	template <size_t,class T, class... Ts>
	struct ListCycle;

	template <class T, class... Ts>
	struct ListCycle<0, FunctionList<T, Ts...> >
	{
		typedef T type;
		static constexpr size_t next=1;
		static constexpr size_t previous=sizeof...(Ts);
	};

	template <class T>
	struct ListCycle<0, FunctionList<T> > // Special case of a single element list
	{
		typedef T type;
		static constexpr size_t next=0;
		static constexpr size_t previous=0;
	};

	template <size_t k, class T, class... Ts>
	struct ListCycle<k, FunctionList<T, Ts...> >
	{
		typedef typename TypeHolder<k - 1, FunctionList<Ts...>>::type type;
		static constexpr size_t next=k+1;
		static constexpr size_t previous=k-1;
	};

	template <class T, class... Ts>
	struct ListCycle<sizeof...(Ts), FunctionList<T, Ts...> >
	{
		typedef typename TypeHolder<sizeof...(Ts)-1, FunctionList<Ts...>>::type type;
		static constexpr size_t next=0;
		static constexpr size_t previous=sizeof...(Ts)-1;
	};

	//-------------------------------------------
	/** @brief Helper template that can statically cycle through a list of types.
	 * @author Mark Grimes
	 * @date 18/Oct/2015
	 */
	template <size_t k,class T>
	struct CyclicList
	{
		typedef typename ListCycle<k%3,T>::type type;
	};

	/** @brief A kd tree where the partitioning functor is calculated statically.
	 *
	 * SYCL does not allow function pointers, so the partitioning functor for a given
	 * level of the tree has to be known at compile time.
	 *
	 * @author Mark Grimes
	 * @date 18/Oct/2015
	 */
	template<class T_datastore,class T_functionList>
	class static_kdtree
	{
	public:
		class Iterator
		{
		public:
			typedef typename ContainerTypeInfo<T_datastore>::value_type value_type;
		public:
			Iterator( size_t leftLimit, size_t rightLimit, const T_datastore& store ) : subTreeFarLeft_(leftLimit), subTreeFarRight_(rightLimit), pStore_(&store) {}
			bool operator==( const Iterator& otherIterator ) { return subTreeFarLeft_==otherIterator.subTreeFarLeft_ && subTreeFarRight_==otherIterator.subTreeFarRight_; }
			bool operator!=( const Iterator& otherIterator ) { return !(*this==otherIterator); }
			value_type operator*() { return (*pStore_)[currentIndex()]; }
			const value_type* operator->() const { return &((*pStore_)[currentIndex()]); }
			Iterator& goto_parent()
			{
				// Pretty poor algorithm, but I just want to get something working. I'll look at
				// optimisations later. Start with a root Iterator and traverse down until the step
				// before the current state, then copy that position.
				// TODO - optimise with a decent algorithm
				size_t current=currentIndex();
				Iterator stepBefore( 0, pStore_->size(), *pStore_ ); // start with an iterator to the root node
				while( stepBefore.currentIndex()!=current )
				{
					subTreeFarLeft_=stepBefore.subTreeFarLeft_;
					subTreeFarRight_=stepBefore.subTreeFarRight_;
					if( stepBefore.currentIndex()<current ) stepBefore.goto_right_child();
					else stepBefore.goto_left_child();
				}
				return *this;
			}
			Iterator& goto_left_child() { subTreeFarRight_=currentIndex(); return *this; }
			Iterator& goto_right_child() { subTreeFarLeft_=currentIndex()+1; return *this; }
			bool has_parent() const { if( subTreeFarLeft_==0 && subTreeFarRight_==pStore_->size() ) return false; else return true; }
			bool has_left_child() const { return currentIndex()!=subTreeFarLeft_; }
			bool has_right_child() const { return currentIndex()+1!=subTreeFarRight_; }
		private:
			size_t subTreeFarLeft_,subTreeFarRight_;
			inline size_t currentIndex() const { return (subTreeFarLeft_+subTreeFarRight_)/2; }
			const T_datastore* pStore_;
		};
	public:
		typedef typename ContainerTypeInfo<T_datastore>::value_type value_type; // Can't use T_datastore::value_type because sycl::accessor doesn't have it
		typedef Iterator const_iterator;
	public:
		static_kdtree( T_datastore data, bool presorted=false ) : data_(data), size_( ::dataSize(data_) )
		{
			if( presorted ) return; // If the data is already in the correct order, no need to do anything else

			// SYCL doesn't allow exceptions, but while I'm testing in host code I'll put this in for now.
			throw std::logic_error( "Construction of static_kdtrees where the input is not already sorted has not been implemented yet" );
		}
		Iterator root() const { return Iterator( 0, size_, data_ ); }//{ return Iterator( 0, ::dataSize(data_), data_ ); }
		Iterator end() const { return Iterator( size_, size_, data_ ); }
		Iterator nearest_neighbour( const value_type& query ) const
		{
			return end();
		}
	protected:
		T_datastore data_;
		size_t size_; // Need to store this because ComputeCpp currently segfaults if accessor size is queried in the kernel. ToDo - remove this once ComputeCpp is fixed.
	};
}

#endif
