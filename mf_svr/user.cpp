#include "user.h"
#include "tcp_task.h"
#include <utility>
#include "group.h"
#include "server.h"
#include <functional>
#include "svr_util/include/easy_code.h"

using namespace mf;
using namespace std;
using namespace lc;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace
{
	void Parse_CMD_REQ_FORWARD(User &user, MfSvrCon &con, const mf::MsgData &msg)
	{
		MsgReqForward req;
		bool ret = CtrlMsgProto::Parse<MsgReqForward>(msg, req);
		L_COND(ret, "parse MsgReqForward fail");

		User *dst_user = UserMgr::Obj().GetUser(req.dst_id);
		if (nullptr == dst_user)
		{
			MsgNtfDiscon send;
			send.dst_id = req.dst_id;
			user.Send(CMD_NTF_DISCON, send);
			return;
		}

		const lc::MsgPack *pMsgPack = con.GetCurMsgPack();
		L_COND(pMsgPack);

		ret = dst_user->SendLcMsg(*pMsgPack); //原样转发
		if (!ret)
		{
			MsgNtfDiscon send;
			send.dst_id = req.dst_id;
			user.Send(CMD_NTF_DISCON, send);
			L_DEBUG("forward msg fail");
			return;
		}
		L_DEBUG("forward msg");
	}

	void Parse_CMD_REQ_CON(User &user, MfSvrCon &con, const mf::MsgData &msg)
	{
		MsgReqCon req;
		bool ret = CtrlMsgProto::Parse<MsgReqCon>(msg, req);
		L_COND(ret, "parse MsgReqCon fail");

		MsgNtfCom send;
		send.req_cmd = CMD_REQ_CON;
		User *dst_user = UserMgr::Obj().GetUser(req.dst_id);
		if (nullptr == dst_user)
		{
			send.is_success = false;
			user.Send(CMD_NTF_COM, send);
			return;
		}

		send.is_success = true;
		user.Send(CMD_NTF_COM, send);

	}
	void EachConSend(SvrCon &con, const lc::MsgPack *pMsgPack)
	{
		L_COND(pMsgPack);
		con.SendData(*pMsgPack);//原样转发
	}

	void BroadCastMsg(MfSvrCon &con)
	{
		L_DEBUG("broad cast msg");

		const lc::MsgPack *pMsgPack = con.GetCurMsgPack();
		L_COND(pMsgPack);
		BaseConMgr &con_mgr = Server::Obj().GetBaseConMgr();
		auto f = std::bind(EachConSend, _1, pMsgPack);
		con_mgr.Foreach(f);
	}

	void Parse_CMD_REQ_BROADCAST(User &user, MfSvrCon &con, const mf::MsgData &msg)
	{
		MsgReqBroadcast req;
		bool ret = CtrlMsgProto::Parse<MsgReqBroadcast>(msg, req);
		L_COND(ret, "parse MsgReqBroadcast fail");

		if (req.group_id == 0)//broadcast
		{
			BroadCastMsg(con);
			return;
		}

		Group *group = GroupMgr::Obj().GetGroup(req.group_id);
		L_COND(group);

		const set<uint32> &set_id = group->GetAllUser();
		FOR_IT_CONST(set_id)
		{
			User *pUser = UserMgr::Obj().GetUser(*it);
			L_COND(pUser);
			MfSvrCon *pCon = pUser->GetConnect();
			L_COND(pCon);

			const lc::MsgPack *pMsgPack = con.GetCurMsgPack();
			L_COND(pMsgPack);
			pCon->SendData(*pMsgPack);
		}
		L_DEBUG("broadcast to group %d", req.group_id);
	}
}



void UserMgr::Init()
{
	m_cmd_2_handle[CMD_REQ_FORWARD] = Parse_CMD_REQ_FORWARD;
	m_cmd_2_handle[CMD_REQ_BROADCAST] = Parse_CMD_REQ_BROADCAST;
	m_cmd_2_handle[CMD_REQ_CON] = Parse_CMD_REQ_CON;

}

bool UserMgr::RegUser(MfSvrCon &con, const mf::MsgData &msg)
{
	L_COND_F(msg.ctrl_cmd == CMD_REQ_REG);
	MsgReqReg req;
	{
		bool r = CtrlMsgProto::Parse<MsgReqReg>(msg, req);
		L_COND_F(r, "MsgRegReq parse fail");	}
	uint32 user_id = req.svr_id;
	if (0 == user_id)
	{
		L_DEBUG("empty user_id");
		return false;
	}
	{
		auto r = m_id_2_user.insert(make_pair(user_id, User(user_id, con.GetId(), req.group_id)));
		if (!r.second)
		{
			L_DEBUG("repeated reg user_id=%d", user_id);
			return false;
		}
		con.SetUserId(user_id);
	}

	if (0 != req.group_id)
	{
		Group *group = GroupMgr::Obj().GetGroup(req.group_id);
		L_COND_F(group);
		bool r = group->AddUser(user_id);
		L_COND_F(r);
	}

	L_DEBUG("reg svr id=%d", user_id);
	return true;
}

//ClientConnect 析构时调用，不要再调用ClientConnect 的函数了。
bool UserMgr::UnregUser(uint32 user_id)
{
	L_COND_F(user_id);
	bool ret = GroupMgr::Obj().UnregUser(user_id);
	if (!ret)
	{
		L_ERROR("GroupMgr::UnregUser user fail. user_id=%d ", user_id);
	}
	auto it = m_id_2_user.find(user_id);
	if (it == m_id_2_user.end())
	{
		L_ERROR("erase user fail. user_id=%d ", user_id);
		return false;
	}
	m_id_2_user.erase(it);

	//broadcast to other user
	MsgNtfDiscon send;
	send.dst_id = user_id;
	FOR_IT(m_id_2_user)
	{
		User &user = it->second;
		user.Send(CMD_NTF_DISCON, send);
	}
	return true;
}

User *UserMgr::GetUser(uint32 user_id)
{
	L_COND_R(user_id, nullptr);
	auto it = m_id_2_user.find(user_id);
	if (it == m_id_2_user.end())
	{
		return nullptr;
	}
	return &it->second;
}

void UserMgr::DispatchMsg(MfSvrCon &con, const mf::MsgData &msg)
{
	L_COND(con.GetUserId());
	auto it = m_id_2_user.find(con.GetUserId());
	if (it == m_id_2_user.end())
	{
		L_ERROR("find user fail. id=%d", con.GetUserId());
		return;
	}

	User &user = it->second;

	auto handle_it = m_cmd_2_handle.find(msg.ctrl_cmd);
	if (handle_it == m_cmd_2_handle.end())
	{
		L_ERROR("find cmd handler fail. ctrl_cmd=%d", msg.ctrl_cmd);
		return;
	}

	(*handle_it->second)(user, con, msg);
}

User::User(uint32 id, uint64 con_id, uint32 group_id)
	:m_id(id)
	,m_con_id(con_id)
	,m_group_id(group_id)
{

}

void User::DispatchMsg(MfSvrCon &con, const mf::MsgData &msg)
{

}

MfSvrCon * User::GetConnect()
{
	L_COND_R(m_con_id, nullptr);
	BaseConMgr &con_mgr = Server::Obj().GetBaseConMgr();
	SvrCon *pCon = con_mgr.FindConn(m_con_id);
	L_COND_R(pCon, nullptr);
	MfSvrCon *pClient = dynamic_cast<MfSvrCon *>(pCon);
	L_COND_R(pClient, nullptr);
	return pClient;
}

bool User::SendMfMsg(mf::MsgData &msg_data)
{
	string tcp_pack;
	MsgDataProto::Serialize(msg_data, tcp_pack);

	lc::MsgPack msg_pack;
	msg_pack.Serialize(tcp_pack);

	return SendLcMsg(msg_pack);
}

bool User::SendLcMsg(const lc::MsgPack &msg_pack)
{
	MfSvrCon *pCon = GetConnect();
	L_COND_F(pCon);
	bool ret = pCon->SendData(msg_pack);
	if (!ret)
	{
		L_WARN("send msg fail");
	}
	return ret;
}
