#include <tuple>
#include <string>
#include <functional>
#include <type_traits>
#include <boost/mpl/assert.hpp>

//Test Case
namespace rpc
{
	struct Response
	{
		int r;
		std::string s;
	};

	struct Request
	{
		double f;
		bool b;
	};

	//测试RPC方法
	Response GetResponseTest1(void);
	Response GetResponseTest2(int x, int y);
	Response GetResponseTest3(std::string s);
	Response GetResponseTest4(std::string &s, int x);
	Response GetResponseTest5(const Request res, int x);
}

//Specialize template declare (catch all case)
template<typename T>
struct function_traits;

template<typename R, typename ...Args>
struct function_traits< R(Args...) > {
	static constexpr std::size_t nargs = sizeof...(Args);
	using return_type = R;
	template <std::size_t index>
	struct argument
	{
		static_assert(index < nargs, "RPC_CALL_ERROR: invalid parameter index.");
		using type = typename std::tuple_element<index, std::tuple<Args...>>::type;
	};
};

template<typename R, typename ...Args>
struct function_traits< R(*)(Args...) > : public function_traits<R(Args...)> {
};

template<typename R, typename base, typename ...Args>
struct function_traits< R(base::*)(Args...) > : public function_traits<R(Args...)> {
	using base_type = base;
};

template<typename R>
struct function_traits< R(void) > {
	static constexpr std::size_t nargs = 0;
	using return_type = R;
};

template<typename R>
struct function_traits< R(*)(void) > : public function_traits<R(void)> {
};

template<typename R, typename base >
struct function_traits< R(base::*)(void) > : public function_traits<R(void)> {
};

//general function
template<typename R, typename ...Args>
struct function_traits< std::function<R(Args...)> > {
	static const size_t nargs = sizeof...(Args);
	using return_type = R;
	template <size_t index>
	struct argument {
		static_assert(index < nargs, "RPC_CALL_ERROR: invalid parameter index.");
		using type = typename std::tuple_element<index, std::tuple<Args...>>::type;
	};
};

template< std::size_t I, typename Func>
struct TupleUnpacker {
	template <typename... Args>
	static void unpackCheckHelper(Func func, Args ...args) {
		auto myTuple = std::make_tuple(args...);
		using traits = function_traits<decltype(func)>;
        using func_traits_type = typename traits::template argument<I>::type;
        using arg_traits_type = typename std::tuple_element<I, decltype(myTuple)>::type;
		static_assert(std::is_same<func_traits_type, arg_traits_type>::value,
			"RPC_CALL_ERROR: some invalid argument type");
		TupleUnpacker<I - 1, decltype(func)>::unpackCheckHelper(func, args...);
	}
};

template<typename Func>
struct TupleUnpacker <0, Func> {
	template <typename... Args>
	static void unpackCheckHelper(Func func, Args ...args) {
		auto myTuple = std::make_tuple(args...);
		using traits = function_traits<decltype(func)>;
        using func_traits_type = typename traits::template argument<0>::type;
        using arg_traits_type = typename std::tuple_element<0, decltype(myTuple)>::type;
		static_assert(std::is_same<func_traits_type, arg_traits_type>::value,
			"RPC_CALL_ERROR: some invalid argument type");
	}
};

template<typename Func, typename... Args>
void RpcFunctionTraitsCheck(Func func, Args ...args) {
	using traits = function_traits<decltype(func)>;
	static_assert(sizeof...(args) == traits::nargs, "RPC_CALL_ERROR: the number of argument is not equal.");
	TupleUnpacker<sizeof...(args)-1, decltype(func)>::unpackCheckHelper(func, args...);
}

template<typename Func>
void RpcFunctionTraitsCheck(Func func) {
	using traits = function_traits<decltype(func)>;
	static_assert(traits::nargs == 0, "RPC_CALL_ERROR: the number of argument is not equal.");
}

template<typename Func, typename... Args>
void SyncCall(const char* method_name, Func func, Args ...args) {
	RpcFunctionTraitsCheck(func, args...);
    return;
}

template<typename Func>
void SyncCall(const char* method_name, Func func) {
	RpcFunctionTraitsCheck(func);
    return;
}

#define SYNC_CALL(methed_name, ...) \
  SyncCall(#methed_name, methed_name, ##__VA_ARGS__)

int main() {
	double x, y;
	std::string str, *pstr;
	rpc::Request req, *preq;
	//const rpc::Request creq;
	/*
		Response GetResponseTest1(void);
		Response GetResponseTest2(int x, int y);
		Response GetResponseTest3(std::string s);
		Response GetResponseTest4(std::string &s, int x);
		Response GetResponseTest5(const Request res, int x);
		Response GetResponseTest5(const Request res, int x, int y, int z, int m, int n);
	*/

	//SYNC_CALL(rpc::GetResponseTest1);				//accept
	//SYNC_CALL(rpc::GetResponseTest1, x);			//compiler error:向一个空签名的方法传值

	//SYNC_CALL(rpc::GetResponseTest2, x, y);			//accepet
	//SYNC_CALL(rpc::GetResponseTest2);				//compiler error:参数个数不匹配
	//SYNC_CALL(rpc::GetResponseTest2, x);			//compiler error:参数个数不匹配
	SYNC_CALL(rpc::GetResponseTest2, str, y);		//compiler error:参数类型不匹配

	//SYNC_CALL(rpc::GetResponseTest3, str);			//accept
	//SYNC_CALL(rpc::GetResponseTest3, pstr);			//compiler error:参数类型不匹配

	//SYNC_CALL(rpc::GetResponseTest4, str, x);		//accept

	//SYNC_CALL(rpc::GetResponseTest5, req, x);		//accept
	//SYNC_CALL(rpc::GetResponseTest5, creq, x);		//accept
	//SYNC_CALL(rpc::GetResponseTest5, preq, x);		//compiler error:参数类型不匹配
}
