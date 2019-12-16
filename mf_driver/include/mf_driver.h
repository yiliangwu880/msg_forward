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
#include "../../mf_proto/include/proto.h"

namespace mf {
	class MfClientMgr;
	struct MfAddr
	{
		std::string ip;
		uint16 port;
	};

	class UserClient : public lc::ClientCon
	{
	public:
		UserClient(MfClientMgr &mgr)
			:m_mgr(mgr)
			, m_is_reg(false)
		{
		}
		virtual void OnRecv(const lc::MsgPack &msg) override;
		virtual void OnConnected() override;
		virtual void OnDisconnected() override;
	public:
		template<class CtrlMsg>
		bool SendCtrlMsg(Cmd cmd, const CtrlMsg &msg)
		{
			std::string tcp_pack;
			if (!MsgData::Serialize(cmd, msg, nullptr, 0, tcp_pack))
			{
				return false;
			}
			return SendPack(tcp_pack);
		}

		bool SendPack(const std::string &tcp_pack);
		bool IsReg() { return m_is_reg; };
	private:
		void ParseNtf(const MsgNtfCom &ntf);

	private:
		MfClientMgr &m_mgr;
		bool m_is_reg; //true表示注册成功。
	};

	class MfClientMgr
	{
		friend class UserClient;
	public:
		MfClientMgr()
			:m_lb_idx(0)
			, m_svr_id(0)
			, m_grp_id(0)
			, m_is_coning(false)
		{
		}
		~MfClientMgr();

		//connect mf svr list. connect
		//@para vec_svr_addr, 服务器地址列表
		//@para svr_id 我的服务器id
		//@para vec_groupId 我的的组id列表
		bool Init(const std::vector<MfAddr> &vec_mf_addr, uint32 svr_id, uint32 group_id = 0);

		//@para const char *custom_pack, 为user和user层之间通讯的自定义协议
		bool Send(uint32 dst_id, const char *custom_pack, uint16 custom_pack_len);

		//广播给指定组. 当group_id==0时，表示广播全部user。
		bool SendGroup(uint32 group_id, const char *custom_pack, uint16 custom_pack_len);

		//连接目标user. 通过 MfClientMgr::OnUserCon 和 MfClientMgr::OnUserDiscon 反馈连接情况
		void ConUser(uint32 dst_id);

		//一般定时调用，尝试连接断开的mf svr.
		//建议5秒以上调用一次。
		void TryReconMf();

		uint32 GetSvrId() const { return m_svr_id; }
		uint32 GetGrpId() const { return m_grp_id; }
		//断开连接
		void DisConnect();
		//true表示已连接任意一台mf svr
		bool IsCon() const { return m_is_coning; }
	private:
		void OnOneMfDiscon(); //其中一个mf链接失败
		UserClient* BlSelectSvr(); //负载均衡一台mf svr

	private:
		//注册失败回调。通常是没有mf_svr,或者svr_id已经被注册
		virtual void OnRegFail();
		//反馈连接mf svr list 情况。连接任意第一台都算成功。
		//连接已成功的情况，再连接第N台，不会回调。
		virtual void OnCon() = 0;
		//全部连接都失败就反馈。
		virtual void OnDiscon() = 0;
		//请求 ConUser 目标后，成功回调
		virtual void OnUserCon(uint32 dst_id) = 0;
		//以下情况导致失败都会回调
		//链接对方，对方主动断线， 发送消息给对方,
		virtual void OnUserDiscon(uint32 dst_id) = 0;
		//@para src_id 发送方服务器id
		virtual void OnRecv(uint32 src_id, const char *custom_pack, uint16 custom_pack_len) = 0;

	private:
		//ClientCon list
		std::vector<UserClient*> m_vec_con; //多个链接mf的客户端
		uint32 m_lb_idx; //load blance轮询数
		uint32 m_svr_id; //我的服务器id
		uint32 m_grp_id;
		bool m_is_coning; //true表示已连接任意一台mf svr
		lc::Timer m_reg_tm; //检查注册过期。
	};

}