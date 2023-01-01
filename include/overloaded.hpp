
// The MIT License (MIT)
//
// Copyright (c) 2013-2023 niXman (github dot nixman at pm.me)
//
// This file is the part of the project 'Overloaded':
//       github.com/nixman/overloaded
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef __OVERLOADED_FUNCTION_HPP
#define __OVERLOADED_FUNCTION_HPP

#include <functional> // to hold lambdas

#include <boost/mpl/vector.hpp>
#include <boost/mpl/accumulate.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/set.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/transform.hpp>

#include <boost/fusion/container/map.hpp>
#include <boost/fusion/include/map.hpp>
#include <boost/fusion/container/map/map_fwd.hpp>
#include <boost/fusion/include/map_fwd.hpp>
#include <boost/fusion/container/map/convert.hpp>
#include <boost/fusion/include/as_map.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/fusion/sequence/intrinsic/has_key.hpp>
#include <boost/fusion/include/has_key.hpp>
#include <boost/fusion/sequence/intrinsic/at_key.hpp>
#include <boost/fusion/include/at_key.hpp>

/*************************************************************************************************/
// I hope the fix will be accepted in 1.82

#include <boost/version.hpp>
#if BOOST_VERSION < 108200
namespace boost {
namespace fusion {

template <typename First, typename Second>
BOOST_CONSTEXPR BOOST_FUSION_GPU_ENABLED
inline typename result_of::make_pair<First,Second>::type
make_pair(Second&& val)
{
    return pair<First, typename detail::as_fusion_element<Second>::type>(
        BOOST_FUSION_FWD_ELEM(Second, val));
}

} // ns fusion
} // ns boost
#endif // BOOST_VERSION < 108200

namespace overloaded {
namespace details {

/*************************************************************************************************/

template<typename T, typename RR = typename std::remove_reference<T>::type>
struct callable_signature
    :callable_signature<
        decltype(&RR::operator())
    >
{};

// func pointer
template<typename Ret, typename... Args>
struct callable_signature<Ret(*)(Args...)> {
    using result_type = Ret;
    using args = boost::mpl::vector<
        typename std::remove_const<
            typename std::remove_reference<Args>::type
        >::type...
    >;
    using signature = Ret(Args...);
};

template<typename Ret>
struct callable_signature<Ret(*)(void)> {
    using result_type = Ret;
    using args = boost::mpl::vector<void>;
    using signature = Ret(void);
};

// class member function pointer
template<typename Obj, typename Ret, typename... Args>
struct callable_signature<Ret(Obj::*)(Args...)> {
    using result_type = Ret;
    using args = boost::mpl::vector<
        typename std::remove_const<
            typename std::remove_reference<Args>::type
        >::type...
    >;
    using signature = Ret(Args...);
};

template<typename Obj, typename Ret>
struct callable_signature<Ret(Obj::*)(void)> {
    using result_type = Ret;
    using args = boost::mpl::vector<void>;
    using signature = Ret(void);
};

template<typename Obj, typename Ret, typename... Args>
struct callable_signature<Ret(Obj::*)(Args...) const> {
    using result_type = Ret;
    using args = boost::mpl::vector<
        typename std::remove_const<
            typename std::remove_reference<Args>::type
        >::type...
    >;
    using signature = Ret(Args...);
};

template<typename Obj, typename Ret>
struct callable_signature<Ret(Obj::*)(void) const> {
    using result_type = Ret;
    using args = boost::mpl::vector<void>;
    using signature = Ret(void);
};

// ref-to-pointer
template<typename Ret, typename... Args>
struct callable_signature<Ret(*&)(Args...)> {
    using result_type = Ret;
    using args = boost::mpl::vector<
        typename std::remove_const<
            typename std::remove_reference<Args>::type
        >::type...
    >;
    using signature = Ret(Args...);
};

template<typename Ret>
struct callable_signature<Ret(*&)(void)> {
    using result_type = Ret;
    using args = boost::mpl::vector<void>;
    using signature = Ret(void);
};

// reference
template<typename Ret, typename... Args>
struct callable_signature<Ret(&)(Args...)> {
    using result_type = Ret;
    using args = boost::mpl::vector<
        typename std::remove_const<
            typename std::remove_reference<Args>::type
        >::type...
    >;
    using signature = Ret(Args...);
};

template<typename Ret>
struct callable_signature<Ret(&)(void)> {
    using result_type = Ret;
    using args = boost::mpl::vector<void>;
    using signature = Ret(void);
};

// signature
template<typename Ret, typename... Args>
struct callable_signature<Ret(Args...)> {
    using result_type = Ret;
    using args = boost::mpl::vector<
        typename std::remove_const<
            typename std::remove_reference<Args>::type
        >::type...
    >;
    using signature = Ret(Args...);
};

template<typename Ret>
struct callable_signature<Ret(void)> {
    using result_type = Ret;
    using args = boost::mpl::vector<void>;
    using signature = Ret(void);
};

/*************************************************************************************************/

template<bool OK, typename F, typename Decayed>
struct callable_holder_real;

// for func pointers/refs
template<typename F, typename Decayed>
struct callable_holder_real<false, F, Decayed> {
    using type = Decayed;
};

// for any other callable type
template<typename F, typename Decayed>
struct callable_holder_real<true, F, Decayed> {
    using signature = typename callable_signature<F>::signature;
    using std_function_type = std::function<signature>;
    using type = typename std::conditional<
         std::is_lvalue_reference<F>::value || !std::is_copy_constructible<F>::value
        ,F
        ,std_function_type
    >::type;
};

template<typename F>
struct callable_holder {
    using decayed = typename std::decay<F>::type;
    using type = typename callable_holder_real<
         std::is_class<decayed>::value
        ,F
        ,decayed
    >::type;
};

/*************************************************************************************************/

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
                ,typename std::conditional<
                     std::is_pointer<Args>::value && !std::is_const<Args>::value
                    ,typename std::add_pointer<
                        typename std::add_const<
                            typename std::remove_pointer<Args>::type
                        >::type
                    >::type
                    ,typename std::decay<Args>::type
                >::type
            >::type...
        >
        ,boost::mpl::vector<void>
    >::type;
};

/*************************************************************************************************/

template<bool>
struct at_key_check_helper;

template<>
struct at_key_check_helper<true> {
    template<typename Map, typename F>
    static bool apply(const Map &map, const F &f) {
        return f == boost::fusion::at_key<
            typename callable_signature<
                typename std::remove_pointer<F>::type
            >::args
        >(map);
    }
};

template<>
struct at_key_check_helper<false> {
    template<typename Map, typename F>
    static bool apply(const Map &, const F &) { return false; }
};

/*************************************************************************************************/

template<typename Map>
struct size_template {
    enum { value = boost::fusion::result_of::size<Map>::value };
};

template<typename Map, typename Sig>
struct exists_template: std::integral_constant<
     bool
    ,boost::fusion::result_of::has_key<
         Map
        ,typename details::callable_signature<Sig>::args
    >::value
>::type
{};

/*************************************************************************************************/

template<typename ...Types>
struct only_unique_sequence {
    using vector = boost::mpl::vector<Types...>;
    using type = typename boost::mpl::accumulate<
        vector,
        boost::mpl::pair<
             typename boost::mpl::clear<vector>::type
            ,boost::mpl::set0<>
        >,
        boost::mpl::if_<
            boost::mpl::contains<
                 boost::mpl::second<boost::mpl::_1>
                ,boost::mpl::_2
            >,
            boost::mpl::_1,
            boost::mpl::pair<
                boost::mpl::push_back<
                     boost::mpl::first<boost::mpl::_1>
                    ,boost::mpl::_2
                >,
                boost::mpl::insert<
                     boost::mpl::second<boost::mpl::_1>
                    ,boost::mpl::_2
                >
            >
        >
    >::type::first;
};

template<typename ...Funcs>
struct map_generator {
    using type = boost::fusion::map<
        typename boost::fusion::pair<
             typename details::callable_signature<Funcs>::args
            ,typename details::callable_holder<Funcs>::type
        >...
    >;

    static_assert(
        boost::fusion::result_of::size<type>::value
            == boost::mpl::size<typename only_unique_sequence<Funcs...>::type>::value
        ,"only unique signatures is allowed!"
    );
};

/*************************************************************************************************/

template<
     typename... Funcs
    ,typename FuncsMap = typename details::map_generator<Funcs...>::type
>
FuncsMap create(Funcs &&...funcs) {
    return {
        boost::fusion::make_pair<
             typename details::callable_signature<Funcs>::args
            ,typename details::callable_holder<Funcs>::type
        >(std::forward<Funcs>(funcs))...
    };
}

/*************************************************************************************************/

template<typename Sig, typename Map>
constexpr bool exists(const Map &) {
    return details::exists_template<Map, Sig>::value;
}

template<typename Map, typename F>
bool exists(const Map &map, F &&f) {
    enum { value = details::exists_template<Map, typename std::remove_pointer<F>::type>::value };
    return value && details::at_key_check_helper<value>::apply(map, std::forward<F>(f));
}

/*************************************************************************************************/

} // ns details

/*************************************************************************************************/

template<typename Map>
struct overloaded_function {
    template<typename Map2>
    overloaded_function(Map2 &&map)
        :map{std::forward<Map2>(map)}
    {}

    static constexpr std::size_t size() {
        return boost::fusion::result_of::size<Map>::value;
    }
    template<typename Signature>
    static constexpr bool exists() {
        return details::exists_template<Map, Signature>::value;
    }

    template<
         typename ...Args
        ,typename Ret = typename details::callable_signature<
            typename std::remove_pointer<
                typename std::remove_reference<
                    typename boost::fusion::result_of::at_key<
                         Map
                        ,typename details::transform_parameters<Args...>::type
                    >::type
                >::type
            >::type
        >::result_type
    >
    Ret invoke(Args &&...args) const {
        using types = typename details::transform_parameters<Args...>::type;
        static_assert(
             boost::fusion::result_of::has_key<Map, types>::value
            ,"calls-map doesn't contains callable-elem with specified parameters"
        );

        return boost::fusion::at_key<types>(map)(std::forward<Args>(args)...);
    }
    template<
         typename ...Args
        ,typename Ret = typename details::callable_signature<
            typename std::remove_pointer<
                typename std::remove_reference<
                    typename boost::fusion::result_of::at_key<
                        Map
                        ,typename details::transform_parameters<Args...>::type
                    >::type
                >::type
            >::type
        >::result_type
    >
    Ret operator()(Args &&...args) const {
        using types = typename details::transform_parameters<Args...>::type;
        static_assert(
             boost::fusion::result_of::has_key<Map, types>::value
            ,"calls-map doesn't contains callable-elem with specified parameters"
        );

        return boost::fusion::at_key<types>(map)(std::forward<Args>(args)...);
    }

private:
    Map map;
};

/*************************************************************************************************/

template<typename ...Funcs>
struct make_overloaded {
    using type = overloaded_function<typename details::map_generator<Funcs...>::type>;
};

/*************************************************************************************************/

template<
     typename... Funcs
    ,typename FuncsMap = typename details::map_generator<Funcs...>::type
>
auto make(Funcs &&...funcs) -> overloaded_function<FuncsMap> {
    return {details::create(std::forward<Funcs>(funcs)...)};
}

/*************************************************************************************************/

} // ns overloaded

#endif // __OVERLOADED_FUNCTION_HPP
