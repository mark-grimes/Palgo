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
		void testMethod()
		{
	//		std::cout << "argument is " << std::get<0>(partitioners_) << std::endl;
			std::cout << "Function gives ";
			for( const auto& element : items_ ) std::cout << std::get<0>(partitioners_)(element) << ", ";
			std::cout << std::endl;
			for( size_t index=0; index<partitioners_.size(); ++index ) std::cout << partitioners_[index](items_.front()) << ", ";
			std::cout << std::endl;
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
		void testMethod()
		{
			std::cout << "blah" << std::endl;
		}
	private:
		std::vector<T_data> items_;
		static const std::array<float (*)(const T_data&),sizeof...(V_functions)> partitioners_;
	};

	template<typename T_data, typename T_sort, float (*... V_functions)(const T_data&)>
	const std::array<float (*)(const T_data&),sizeof...(V_functions)> test_tree2<T_data,T_sort,V_functions...>::partitioners_={V_functions...};


}

#endif
