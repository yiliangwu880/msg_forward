术语:
	msg_foward : 消息转发服务器。
	user       : msg_foward的客户端，被服务器对象。客户端进程。
	
设计目的：
问题：一套服务器群，互相之间通讯，直接用地址互相连接，配置繁琐。
解决方法：
	进程都有自己的id, 都注册到msg_forward, 就可以根据对方id互相发消息。
	集中管理，进程间不直连，统一到msg_forward转发。避免信息分散不对等的情况。 信息分散不对等的情况例子：进程1，重启新的进程1. ID一样。其他进程不知道，继续和旧进程1通讯。

不用第三方的消息队列服务的原因：
	第三方的消息队列服务有activemq、rabbitmq、kafka 等。
	不合适直接用的原因：
	msg_forward核心功能就是消息可靠转发、而不是消息队列。相对第三方来说，它功能小、学习成本底。 
	msg_forward部署可以做得很简单。
	转发消息用，不需要存库，msg_forward可以更快。

	
msg_foward user之间通讯的消息结构：
tcp包: len, tcp_pack.
tcp_pack:ctrl_len,ctrl_cmd,ctrl_pack，custom_cmd, custom_pack.	
--ctrl_len表示ctrl_cmd,ctrl_pack的总字节数。
--纯控制消息，就没有custom_cmd, custom_pack了.	
--用户自定义消息包，具体协议格式自定义，可以protobuf或者其他的都行。
													
		
server 功能：
	可以多个msg_forward。高可用，无状态。状态改变都是来源客户端user.
	注册服务器id
	路由消息。
	{
		1对1，
		1对组
	}
	可靠转发，连接失败才需要反馈给客户端。 (类似tcp连接那样理解)
	广播新注册 user到all user
	广播断开user到all user
	管理user信息： 组，id
	
client driver 功能：
	连接服务器列表
	负载均衡策略：发送轮询一个服务器使用
	断线重连定时器，循环定时越来越慢。  svr_utili写可复用timer适配器。 适配底层，用户接口。
	注册id
	发送，接收
	更新all client 状态
	