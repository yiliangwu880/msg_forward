
server
		根据服务id,路由消息。 容易配置，服务消费者只需要知道对方服务id,就能通讯。
		可以多个msg_forward。高可用，无状态。

client driver	
		服务器断开，定时重连。
		注册服务器id
		发送，接收
		断开连接通知。