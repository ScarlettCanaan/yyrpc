
# YYRPC
***YYRPC***  是基于libuv异步网络库和msgpack序列化框架的轻量级rpc服务

# 总览
- 直接使用CPP的`*.h`头文件作为IDL用于client和server之间描述RPC的接口和通信数据结构
- 支持std库的标准容器型数据结构`container`，支持自定义结构，使用 `YYRPC_PROPERTY(...)` 注册自定义结构的各个参数
- 网络接入层使用半同步半异步模式，采用独立IO线程，同步Accept，异步处理请求，工作线程采用独立线程池
- 提供完善的类型检测机制，所有类型检测异常、序列化异常均在**Compile Time**完成

    ***NOTE:***各种类型检测异常、序列化异常均会在编译时报错，请留意错误信息，如：`RPC_CALL_ERROR:argument type mismatch`
- 提供简易的接口注册方式，注册方法接受任意能被转换为`std::function`的类型实现RPC方法

#性能

> 待测试...

# 使用范例
> 待完善，yyrpc wiki施工中

#编译方法

- 编译环境准备：

	- Linux
	- GCC-5.4及以上版本(笔者是5.4)
	- cmake

- 编译安装方法：

	进入yyrpc根目录
```
cmake {dir}
make
make install
```
#使用方法
1. 编写IDL文件(client和server端均包含的`*.h`文件)
	- 使用macro `YYRPC_METHOD(method_name, ...)`注册远程过程调用方法
	- 使用macro `YYRPC_PROPERTY(...)`注册自定义结构（YYRPC原生支持标准容器型数据结构`container`，无需注册） 
2. server端
	- 在使用`YYRPC_METHOD(method_name, ...)`注册之后，会以***method_name***为类名生成一个class，可用
	  `method_name.bind(std::function)`来实现方法
	- `method_name.bind(std::function)`的参数可接受任何可被转换成`std::function`的对象，如函数指针、lambda表达式等。
	- 使用`InitRPC(const char* cfgFile)`加载RPC配置文件
	- 使用`RunRpc()`启动RPC服务
3. client端
	- 使用`InitRPC(const char* cfgFile)`加载RPC配置文件
	- 使用`RunRpc()`启动RPC服务
	- 直接调用IDL文件所注册的方法，向server端发起远程过程调用
	- 直接调用方法会返回一个`AsyncResult`，用户可在需要的时刻调用`wait()`等待RPC返回结果，或者调用`then<>(std::function callback)`注册匿名方法，在RPC返回结果时回调(其实只是同步调用，因为现在还不能支持多线程)


