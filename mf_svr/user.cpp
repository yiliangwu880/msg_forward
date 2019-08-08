#include "user.h"
#include "tcp_task.h"
#include <utility>
#include "group.h"
#include "server.h"
#include <functional>
#include "svr_util/include/easy_code.h"
#include "svr_util/include/su_include.h"

using namespace mf;
using namespace std;
using namespace lc;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;


bool UserMgr::RegUser(MfSvrCon &con, const mf::MsgData &msg)
{
	su::ScopeGuard sg([&] { con.DisConnect(); }); //失败必须断开
	
	L_COND_F(msg.ctrl_cmd == CMD_REQ_REG);
	MsgReqReg req;
	{
		bool r = CtrlMsgProto::Parse(msg, req);
		L_COND_F(r, "MsgRegReq parse fail");	}
	uint32 user_id = req.svr_id;
	if (0 == user_id)
	{
		L_DEBUG("empty user_id");
		
		return false;
	}
	//add user and group 
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
	MsgNouse send; 
	L_COND_F(Send(CMD_RSP_REG, send));

	L_DEBUG("reg svr id=%d, group_id=%d", user_id, req.group_id);
	sg.Dismiss();
	return true;
}

//ClientConnect 析构时调用，不要再调用MfSvrCon 的函数了。
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
	send.svr_id = user_id;
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
	msg_data.Serialize(tcp_pack);

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


