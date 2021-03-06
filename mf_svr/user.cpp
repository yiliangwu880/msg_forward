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
	if (msg.ctrl_cmd != CMD_REQ_REG)
	{
		L_DEBUG("RegUser cmd must be CMD_REQ_REG. cur cmd=%d", msg.ctrl_cmd);
		return false;
	}
	MsgReqReg req;
	{
		bool r = CtrlMsgProto::Parse(msg, req);
		L_COND_F(r, "MsgRegReq parse fail");	}
	uint32 user_id = req.svr_id;
	L_COND_F(0 != user_id, "empty user_id");

	//add user and group 
	User *user=nullptr;
	{
		auto r = m_id_2_user.insert(make_pair(user_id, User(user_id, con.GetId(), req.group_id)));
		if (!r.second)
		{
			L_DEBUG("repeated reg user_id=%d", user_id);//不用反馈给客户端，避免复杂度。 客户端自己定时检查注册失败
			return false;
		}
		con.SetUserId(user_id);
		user = &(r.first->second);
		L_COND_F(user);
	}

	if (0 != req.group_id)
	{
		Group *group = GroupMgr::Obj().GetGroup(req.group_id);
		L_COND_F(group);
		bool r = group->AddUser(user_id);
		L_COND_F(r);
	}
	MsgNone send;
	L_COND_F(user->Send(CMD_RSP_REG, send));

	L_DEBUG("reg svr id=%d, group_id=%d", user_id, req.group_id);
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
	for( auto &v : m_id_2_user)
	{
		User &user = v.second;
		if (!user.IsConnect())
		{//有些user已经断开连接，还没处理删除操作。
			continue;
		}
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

MfSvrCon * User::GetConnect()
{
	L_COND_R(m_con_id, nullptr);
	BaseConMgr &con_mgr = Server::Obj().GetBaseConMgr();
	SvrCon *pCon = con_mgr.FindConn(m_con_id);
	if (pCon == nullptr)
	{
		L_ERROR("FindConn fail. m_con_id=%lld", m_con_id);
		return nullptr;
	}
	MfSvrCon *pClient = dynamic_cast<MfSvrCon *>(pCon);
	L_COND_R(pClient, nullptr);
	return pClient;
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

bool User::IsConnect()
{
	SvrCon *pCon = Server::Obj().GetBaseConMgr().FindConn(m_con_id);
	return pCon != nullptr;
}


