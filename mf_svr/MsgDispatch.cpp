#include "user.h"
#include "tcp_task.h"
#include <utility>
#include "group.h"
#include "server.h"
#include <functional>
#include "svr_util/include/easy_code.h"
#include "MsgDispatch.h"

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
		bool ret = CtrlMsgProto::Parse(msg, req);
		L_COND(ret, "parse MsgReqForward fail");
		if (req.src_id != user.GetId())
		{
			MsgNtfCom send;
			send.Init(CMD_REQ_FORWARD, "error src id");
			user.Send(CMD_REQ_FORWARD, send);
			return;
		}
		if (req.dst_id == 0)
		{
			MsgNtfCom send;
			send.Init(CMD_REQ_FORWARD, "dst_id == 0");
			user.Send(CMD_REQ_FORWARD, send);
			return;
		}
		User *dst_user = UserMgr::Obj().GetUser(req.dst_id);
		if (nullptr == dst_user || !dst_user->IsConnect())
		{
			MsgNtfDiscon send;
			send.svr_id = req.dst_id;
			user.Send(CMD_NTF_DISCON, send);
			return;
		}

		const lc::MsgPack *pMsgPack = con.GetCurMsgPack();
		L_COND(pMsgPack);

		ret = dst_user->SendLcMsg(*pMsgPack); //原样转发
		if (!ret)
		{
			MsgNtfDiscon send;
			send.svr_id = req.dst_id;
			user.Send(CMD_NTF_DISCON, send);
			L_DEBUG("forward msg fail");
			return;
		}
		L_DEBUG("forward msg。 src_id, dst_id=%d %d", req.src_id, req.dst_id);
	}

	void Parse_CMD_REQ_CON(User &user, MfSvrCon &con, const mf::MsgData &msg)
	{
		MsgReqCon req;
		bool ret = CtrlMsgProto::Parse(msg, req);
		L_COND(ret, "parse MsgReqCon fail");
		if (0 == req.dst_id)
		{
			L_DEBUG("Parse_CMD_REQ_CON fail,  illegal para. dis_id==0");
			MsgNtfCom send;
			send.Init(CMD_REQ_CON, "dst_id == 0");
			user.Send(CMD_RSP_CON, send);
			return;
		}

		MsgRspCon send;
		send.dst_id = req.dst_id;
		User *dst_user = UserMgr::Obj().GetUser(req.dst_id);
		if (nullptr == dst_user)
		{
			send.is_ok = false;
			user.Send(CMD_RSP_CON, send);
			return;
		}

		send.is_ok = true;
		user.Send(CMD_RSP_CON, send);
	}

	void EachConSend(SvrCon &con, const lc::MsgPack *pMsgPack, uint64 src_id)
	{
		if (con.GetId() == src_id)
		{
			return;
		}
		L_COND(pMsgPack);
		con.SendData(*pMsgPack);//原样转发
	}

	void BroadCastMsg(MfSvrCon &con)
	{
		L_DEBUG("broad cast msg");

		const lc::MsgPack *pMsgPack = con.GetCurMsgPack();
		L_COND(pMsgPack);
		BaseConMgr &con_mgr = Server::Obj().GetBaseConMgr();
		auto f = std::bind(EachConSend, _1, pMsgPack, con.GetId());
		con_mgr.Foreach(f);
	}

	void Parse_CMD_REQ_BROADCAST(User &user, MfSvrCon &con, const mf::MsgData &msg)
	{
		MsgReqBroadcast req;
		bool ret = CtrlMsgProto::Parse(msg, req);
		L_COND(ret, "parse MsgReqBroadcast fail");

		if (0 == req.group_id)//broadcast
		{
			BroadCastMsg(con);
			return;
		}

		Group *group = GroupMgr::Obj().GetGroup(req.group_id);
		L_COND(group);


		const lc::MsgPack *pMsgPack = con.GetCurMsgPack();
		L_COND(pMsgPack);
		const set<uint32> &set_id = group->GetAllUser();
		for(const auto v : set_id)
		{
			if (user.GetId() == v)
			{
				continue;
			}
			User *pUser = UserMgr::Obj().GetUser(v);
			L_COND(pUser);
			if (!pUser->IsConnect())//连接已经断开，还没删除对象
			{
				continue;
			}
			MfSvrCon *pCon = pUser->GetConnect();
			L_COND(pCon);
			pCon->SendData(*pMsgPack);
		}
		L_DEBUG("broadcast to group %d", req.group_id);
	}
}


void MsgDispatch::Init()
{
	m_cmd_2_handle[CMD_REQ_FORWARD] = Parse_CMD_REQ_FORWARD;
	m_cmd_2_handle[CMD_REQ_BROADCAST] = Parse_CMD_REQ_BROADCAST;
	m_cmd_2_handle[CMD_REQ_CON] = Parse_CMD_REQ_CON;
}

void MsgDispatch::DispatchMsg(MfSvrCon &con, const mf::MsgData &msg)
{
	L_COND(con.GetUserId());
	User *user = UserMgr::Obj().GetUser(con.GetUserId());
	L_COND(user);

	auto handle_it = m_cmd_2_handle.find((mf::Cmd)msg.ctrl_cmd);
	if (handle_it == m_cmd_2_handle.end())
	{
		L_ERROR("find cmd handler fail. ctrl_cmd=%d", msg.ctrl_cmd);
		return;
	}

	(*handle_it->second)(*user, con, msg);
}
