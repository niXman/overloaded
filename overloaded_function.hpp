
#ifndef _overloaded_function_hpp
#define _overloaded_function_hpp

#include <boost/mpl/vector.hpp>

#include <boost/fusion/container/map.hpp>
#include <boost/fusion/include/map.hpp>
#include <boost/fusion/container/map/map_fwd.hpp>
#include <boost/fusion/include/map_fwd.hpp>
#include <boost/fusion/container/map/convert.hpp>
#include <boost/fusion/include/as_map.hpp>
#include <boost/fusion/support/pair.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/fusion/sequence/intrinsic/has_key.hpp>
#include <boost/fusion/include/has_key.hpp>
#include <boost/fusion/sequence/intrinsic/at_key.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/fusion/algorithm/transformation/erase_key.hpp>
#include <boost/fusion/include/erase_key.hpp>
#include <boost/fusion/algorithm/transformation/push_back.hpp>
#include <boost/fusion/include/push_back.hpp>

namespace boost {
namespace detail {

/***************************************************************************/

template<typename Map, typename K>
struct check_if_contains {
	static_assert(
		 boost::fusion::result_of::has_key<Map, K>::value
		,"K is not in Map"
	);

	using unused = K;
};

/***************************************************************************/

template<typename...>
struct callable_parameters;

template<typename Ret, typename... Args>
struct callable_parameters<Ret(Args...)> {
	using result_type = Ret;
	using type = boost::mpl::vector<
		typename std::remove_const<
			typename std::remove_reference<Args>::type
		>::type...
	>;
};

template<typename Ret>
struct callable_parameters<Ret(void)> {
	using result_type = Ret;
	using type = boost::mpl::vector<void>;
};

/****************************************************/

template<typename... Args>
struct transform_parameters {
	using type = typename std::conditional<
		 (sizeof...(Args) > 0)
		,boost::mpl::vector<
			typename std::conditional<
				 std::is_array<Args>::value
				,typename std::add_pointer<
					typename std::add_const<
						typename std::remove_all_extents<Args>::type
					>::type
				>::type
				,Args
			>::type...
		>
		,boost::mpl::vector<void>
	>::type;
};

/***************************************************************************/

template<bool>
struct at_key_check_helper;

template<>
struct at_key_check_helper<true> {
	template<typename Map, typename F>
	static bool apply(const Map &map, F f) {
		return f == boost::fusion::at_key<
			typename callable_parameters<
				typename std::remove_pointer<F>::type
			>::type
		>(map);
	}
};

template<>
struct at_key_check_helper<false> {
	template<typename Map, typename F>
	static bool apply(const Map &, F) { return false; }
};

/***************************************************************************/

} // ns detail

namespace tmpl {

template<typename Map>
struct calls {
	static const std::size_t value = boost::fusion::result_of::size<Map>::value;
};

template<typename Map, typename Sig>
struct exists: std::integral_constant<
	bool
	,boost::fusion::result_of::has_key<
		Map
	  ,typename detail::callable_parameters<Sig>::type
	>::value
>::type
{};

template<typename Map, typename Sig>
struct erase {
	using type = typename boost::fusion::result_of::as_map<
		typename boost::fusion::result_of::erase_key<
			 Map
			,typename detail::callable_parameters<
				typename std::remove_pointer<Sig>::type
			>::type
		>::type
	>::type;
};

template<typename Map, typename Sig>
struct insert {
	using type = typename boost::fusion::result_of::as_map<
		typename boost::fusion::result_of::push_back<
			 Map
			,boost::fusion::pair<
				 typename detail::callable_parameters<Sig>::type
				,typename std::add_pointer<Sig>::type
			>
		>::type
	>::type;
};

} // ns tmpl
} // ns boost

/***************************************************************************/

namespace boost {
namespace overloaded_function {

/***************************************************************************/

template<
	 typename... Args
	,typename Map = boost::fusion::map<
		typename boost::fusion::pair<
			typename detail::callable_parameters<
				typename std::remove_pointer<Args>::type
			>::type
			,Args
		>...
	>
>
Map create(Args... args) {
	return Map(
		boost::fusion::make_pair<
			typename detail::callable_parameters<
				typename std::remove_pointer<Args>::type
			>::type
		>(args)...
	);
}

/***************************************************************************/

template<typename Map>
constexpr std::size_t calls(Map) {
	return tmpl::calls<Map>::value;
}

/***************************************************************************/

template<
	 typename Map
	,typename... Args
	,typename Ret = typename detail::callable_parameters<
		typename std::remove_pointer<
			typename std::remove_reference<
				typename boost::fusion::result_of::at_key<
					 Map
					,typename detail::transform_parameters<Args...>::type
				>::type
			>::type
		>::type
	>::result_type
>
Ret invoke(const Map &map, const Args&... args) {
	using types = typename detail::transform_parameters<Args...>::type;
	static_assert(
		 boost::fusion::result_of::has_key<Map, types>::value
		,"calls-map doesn't contains callable-elem with specified parameters"
	);

	return boost::fusion::at_key<types>(map)(args...);
}

/***************************************************************************/

template<
	 typename Sig
	,typename Map
>
constexpr bool exists(Map) {
	return tmpl::exists<Map, Sig>::value;
}

template<
	 typename Map
	,typename F
>
bool exists(const Map &map, F f) {
	enum { value = tmpl::exists<Map, typename std::remove_pointer<F>::type>::value };
	return value && detail::at_key_check_helper<value>::apply(map, f);
}

/***************************************************************************/

template<
	 typename Map
	,typename F
	,typename Ret = typename tmpl::erase<Map, F>::type
>
Ret erase(Map &map, F) {
	return boost::fusion::as_map(boost::fusion::erase_key<
			typename detail::callable_parameters<
				typename std::remove_pointer<F>::type
			>::type
	>(map));
}

/***************************************************************************/

template<
	 typename Map
	,typename F
	,typename Ret = typename tmpl::insert<
		 Map
		,typename std::remove_pointer<F>::type
	>::type
>
Ret insert(Map &map, F f) {
	return boost::fusion::as_map(
		boost::fusion::push_back(map, boost::fusion::make_pair<
			typename detail::callable_parameters<
			 typename std::remove_pointer<F>::type
			>::type
		>(f))
	);
}

/***************************************************************************/

template<typename Map>
struct overloaded_function {
	overloaded_function(Map &&map)
		:map(map)
	{}

	template<
		 typename... Args
		,typename Ret = typename detail::callable_parameters<
			typename std::remove_pointer<
				typename std::remove_reference<
					typename boost::fusion::result_of::at_key<
						 Map
						,typename detail::transform_parameters<Args...>::type
					>::type
				>::type
			>::type
		>::result_type
	>
	Ret operator()(const Args&... args) const {
		return invoke(map, args...);
	}

private:
	Map map;
};

/***************************************************************************/

template<
	 typename... Args
	,typename Map = boost::fusion::map<
		typename boost::fusion::pair<
			typename detail::callable_parameters<
				typename std::remove_pointer<Args>::type
			>::type
			,Args
		>...
	>
>
overloaded_function<Map> make_overloaded_function(Args... args) {
	return overloaded_function<Map>(
		Map(
			boost::fusion::make_pair<
				typename detail::callable_parameters<
					typename std::remove_pointer<Args>::type
				>::type
			>(args)...
		)
	);
}

/***************************************************************************/

} // ns overloaded_function
} // ns boost

#endif // _overloaded_function_hpp
