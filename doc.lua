
server 功能：
	可以多个msg_forward。高可用，无状态。
	注册服务器id
	路由消息。
	{
		1对1，
		1对组
	}
	路由失败反馈给客户端
	广播新注册client到all client
	广播断开client到all client
	
	
client driver 功能：
	连接服务器列表
	负载均衡策略：发送轮询一个服务器使用
	断线重连定时器，循环定时越来越慢。  svr_utili写可复用timer适配器。 适配底层，用户接口。
	注册id
	发送，接收
	更新all client 状态
	