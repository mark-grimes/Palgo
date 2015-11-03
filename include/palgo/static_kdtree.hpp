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
			Iterator( size_t index, const T_datastore& store ) : index_(index), pStore_(&store) {}
			bool operator==( const Iterator& otherIterator ) { return index_==otherIterator.index_; }
			bool operator!=( const Iterator& otherIterator ) { return !(*this==otherIterator); }
			value_type operator*() { return (*pStore_)[index_]; }
			const value_type* operator->() const { return &((*pStore_)[index_]); }
		private:
			size_t index_;
			const T_datastore* pStore_;
		};
	public:
		typedef typename ContainerTypeInfo<T_datastore>::value_type value_type; // Can't use T_datastore::value_type because sycl::accessor doesn't have it
		typedef Iterator const_iterator;
	public:
		static_kdtree( T_datastore data, bool presorted=false ) : data_(data)
		{
			if( presorted ) return; // If the data is already in the correct order, no need to do anything else

			// SYCL doesn't allow exceptions, but while I'm testing in host code I'll put this in for now.
			throw std::logic_error( "Construction of static_kdtrees where the input is not already sorted has not been implemented yet" );
		}
		Iterator nearest_neighbour( const value_type& query ) const
		{
			return Iterator(0,data_);
		}
	protected:
		T_datastore data_;
	};
}

#endif
