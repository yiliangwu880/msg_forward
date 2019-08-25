后期放到 game builder 项目主目录

目标：
{
	合适创业公司的服务器。
	使用学习成本低，快速开展业务开发。
	性能符合需求就行，不要求做到极致。
	尽量单进程点线程模式，降低开发复杂度。
	每个库独立性好，可以直接应用于别得项目。不需要增加各种依赖关系。
}

细节实现方法：
{
	每个库独立性好. 所以不同库出现相同的功能文件，比如log_file.cpp
	库日志输入方式，可扩展给引用
	遵守用简单容易维护的原则，只要不涉及到性能瓶颈的方面，都使用简单方法实现。：
	{
		服务器ID由类型，+ 编号组成。 用十进制划分。比如 1003,1004，简单异读，不考虑小性能。
		cs通信，不用传统的req_cmdid, rsp_cmdid. 改为复用 req_rsp_cmdid. 好处：请求响应不冲突，代码还可以动态化，自动化，不需要手动写响应消息号。
	}
	研究使用protobuf最新版本
	C++11
	配置读取使用protobuf 文本格式。 项目代码不用加入多种解析库（json,ini,xml）了，容易学习。 
	使用测试库，比如boost.test util_test等
	
}

game builder项目：
{

	1 网络收发库： libevent2,C++封装，快速写网络通讯， 服务器，客户端链接
	{
		所有服务器，需要用进程间通讯的，都要引用这个库。 默认省略写 include 1
	}
	有独立测试项目，后面所有功能，都做一个独立测试项目，写测试用例。
	
	11 线程库
	{
	}
	21 svr_util 库：
	{
		服务器常用的功能：日志，唯一进程，参数读取，信号，崩溃日志，线程间通讯库
	}
	
	2  msg_forward 消息转发服务：服务器通讯用，做其他服务器的转发服务。
	{
		根据服务id,路由消息。 容易配置，服务消费者只需要知道对方服务id,就能通讯。
		可以多个msg_forward。高可用，无状态。
	}
	22 mfd  msg_forward客户端驱动
	{
		服务器断开，定时重连。
		注册服务器id
		发送，接收
		断开连接通知。
	}
	
	3 网关：客户端 入口，验证，转发消息
	{
	}
	
	4 基础登陆验证流程库，可根据不同游戏扩展，
	{
		无状态
		可扩展内容：
		{
			支持本服db验证，
			第三方服务器验证
			验证结果通知接口
		}
	}
	 
	5 接入服务器，access
	{
		
		可扩展内容：
		{
			验证接口：接登陆验证用
			转发机制： 默认转发服务器，制定转发
		}
	}
	
	6 DbProxy:
	{
		和客户端：用protobuf 描述对象以及行为 
		转化客户端消息为db操作
		可创建表，或者集合
		可扩展：
		{
			默认mysql
			db类型： mysql,mongodb等。
			取模分库
			缓存redis
		}
	}
	
	-----------------------后面后期看需要开发--------------
	7 lua接入库
	{
		多参考别的项目，采取简单好用方法
	}
	
	50 服务器中心驱动：服务发现功能
	{	
		etcd,
		开发：
		c++ etcd客户端：
		先不做了，超大型服务器才用得上。
	}
	
}

待定项:
{
	进程间通讯：
	{
		方法1：多台转发服务器做服务，其他服务器作为客户端
		方法2：开源库？
	}
}


