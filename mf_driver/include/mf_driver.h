/*
//依赖libevent_cpp库
需要下面写才能工作：

main()
{
	EventMgr::Obj().Init();

		调用本库的api


	EventMgr::Obj().Dispatch();
}

//你也可以根据协议，自己写自定义客户端，就不依赖libevent_cpp库了.
*/

#pragma once

#include <string>
#include <string.h>
#include <vector>
#include "libevent_cpp/include/include_all.h"
#include "svr_util/include/singleton.h"
#include "svr_util/include/easy_code.h"
#include "svr_util/include/typedef.h"

namespace mf {
	struct MfAddr 
	{
		std::string ip;
		uint16 port;
	};
	class UserClient : public lc::ClientCon
	{
	public:
		virtual void OnRecv(const lc::MsgPack &msg) override;
		virtual void OnConnected() override;
		virtual void OnDisconnected() override;
	};

	class BaseClientMgr
	{
	public:
		BaseClientMgr() 
			:m_lb_idx(0)
			, m_user_id(0)
		{
		}
		~BaseClientMgr();
		//connect mf svr list. connect
		bool Init(const std::vector<MfAddr> &vec_svr_addr, uint32 user_id);

		bool Send(uint32 dst_id, const char *pack, uint16 pack_len);
		//广播给指定组. 当group_id==0时，表示广播全部user。
		void SendGeroup(uint32 group_id, const char *pack, uint16 pack_len);

		void ConnectUser(uint32 dst_id); //连接目标user. 通过 OnUserConnected 和 OnUserDisconnected 反馈连接情况
		
		//一般定时调用，尝试连接断开的mf svr.
		void TryReconnectSvr();
	private:
		//反馈连接mf svr list 情况。能连接任意一台都算成功。
		virtual void OnConnected() = 0;
		//全部连接都失败就反馈。
		virtual void OnDisconnected() = 0;

		virtual void OnUserConnected(uint32 dst_id) = 0;
		virtual void OnUserDisconnected(uint32 dst_id) = 0;//对方没连上，或者对方主动断线，都会调用。
		virtual void OnRecv(uint32 src_id, const char *pack, uint16 pack_len)=0;

	private:
		UserClient* BlSelectSvr(); //负载均衡一台mf svr
	private:
		//ClientCon list
		std::vector<UserClient*> m_vec_con;
		uint32 m_lb_idx; //load blance轮询数
		uint32 m_user_id; 
	};

	class IClientParse
	{
	public:
		bool Parse(const char *tcp_pack, uint16 tcp_pack_len);
		virtual void Forward() = 0;

		void SerializeCmd();
	};
}