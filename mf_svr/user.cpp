#include "user.h"
#include "tcp_task.h"
#include <utility>
#include "group.h"
#include "server.h"

using namespace mf;
using namespace std;
using namespace lc;


namespace
{
	void Parse_CMD_REQ_FORWARD(User &user, ConnectClient &con, const mf::MsgData &msg)
	{
		MsgReqForward req;
		bool ret = Parse<MsgReqForward>(msg, req);
		L_COND(ret, "parse MsgReqForward fail");

		User *pUser = UserMgr::Obj().GetUser(req.dst_id);
		if (nullptr == pUser)
		{
			//MsgNtfForwardFail send;
			//send.dst_id = req.dst_id;
		//	pUser->Send(send);
			return;
		}

	}

}








void UserMgr::Init()
{
	m_cmd_2_handle[CMD_REQ_FORWARD] = Parse_CMD_REQ_FORWARD;


}

bool UserMgr::RegUser(ConnectClient &con, const mf::MsgData &msg)
{
	L_COND_F(msg.ctrl_cmd == CMD_REQ_REG);
	MsgReqReg req;
	{
		bool r = req.Parse(msg.ctrl_pack, msg.ctrl_pack_len);
		L_COND_F(r, "MsgRegReq parse fail");	}
	uint32 user_id = req.svr_id;
	L_COND_F(user_id);
	{
		auto r = m_id_2_user.insert(make_pair(user_id, User(user_id, con.GetId())));
		L_COND_F(r.second);
		con.SetUserId(user_id);
	}
	L_DEBUG("reg svr id=%d", user_id);

	if (0 != req.group_id)
	{
		Group *group = GroupMgr::Obj().GetGroup(req.group_id);
		L_COND_F(group);
		bool r = group->AddUser(user_id);
		L_COND_F(r);
	}

	return true;
}

bool UserMgr::UnregUser(uint32 id)
{

	return true;
}

User *UserMgr::GetUser(uint32 id)
{
	L_COND_R(id, nullptr);
	auto it = m_id_2_user.find(id);
	if (it == m_id_2_user.end())
	{
		return nullptr;
	}
	return &it->second;
}

void UserMgr::DispatchMsg(ConnectClient &con, const mf::MsgData &msg)
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

User::User(uint32 id, uint64 con_id)
	:m_id(id)
	,m_con_id(con_id)
{

}

void User::DispatchMsg(ConnectClient &con, const mf::MsgData &msg)
{

}

ConnectClient * User::GetConnect()
{
	L_COND_R(m_con_id, nullptr);
	BaseConMgr &con_mgr = Server::Obj().GetBaseConMgr();
	SvrCon *pCon = con_mgr.FindConn(m_con_id);
	L_COND_R(pCon, nullptr);
	ConnectClient *pClient = dynamic_cast<ConnectClient *>(pCon);
	L_COND_R(pClient, nullptr);
	return pClient;
}
