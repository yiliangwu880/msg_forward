术语:
	mf	  == msg foward, 消息转发服务器。
	user  == msg foward的客户端，被服务器对象。客户端进程。服务器群的其中一个服务器进程。
	cd	  == client driver,驱动，给user作为库使用，可以再这个基础快速写出客户端链接应用。

设计目的：
问题：一套服务器群，数量超过几台以上，互相之间直接socket通讯会带来以下问题：
		配置繁琐，每条连接都要一个服务端的ip地址。
		连接网状拓扑，分析问题难度大。
		
解决方法：
	服务器群都链接mf,mf作为消息转发中心出。 
	User的角度去看，User之间的链接概念类似tcp/ip链接，可靠传送。
	User进程都有自己的id, 都注册到mf。
	User进程可选用cd,也可以自己根据协议写一个客户端驱动。
	
缺点：通讯需要转发，不是最快的通讯方式。 
	
不用第三方的消息队列服务的原因：
	第三方的消息队列服务有activemq、rabbitmq、kafka 等。
	不合适直接用的原因：
	mf核心功能就是消息可靠转发、而不是消息队列。
	mf功能少、学习成本底。 
	mf部署可以做得很简单。
	转发消息用，不需要存库，mf可以更快。

	
mf user之间通讯的消息结构：
{
tcp包: len, tcp_pack.
tcp_pack:ctrl_len,ctrl_cmd, ctrl_pack，custom_pack.	
--ctrl_len表示ctrl_cmd,ctrl_pack的总字节数。
--纯控制消息，就没有custom_pack了.	
--custom_pack 用户自定义消息包，具体协议格式自定义，比如可以用protobuf。
分二层解析

2） user和user层：custom_pack	--user 之间通讯的自定义协议

1） user和mf层： tcp_pack  --user mf通讯协议，
	tcp_pack 解析出 mf::MsgData	

分层图：
	user		  mf				user
user和user层-------------------	user和user层
user和mf层	--- user和mf层	--	user和mf层
}
		
mf 功能：
	可以多个mf。高可用，无状态。状态改变都是来源客户端user.
	注册服务器id
	路由消息。
	{
		1对1，
		1对组
	}
	可靠转发，连接失败才需要反馈给user。 (类似tcp连接那样理解)
	广播断开user到all user
	管理user信息： 组，id
	
client driver 功能：
	连接服务器列表
	负载均衡策略：发送轮询一个服务器使用
	断线重连定时器
	注册id
	发送，接收
	更新all user 状态
	
	

	