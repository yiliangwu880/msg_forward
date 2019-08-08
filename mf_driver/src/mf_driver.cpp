
#include "mf_driver.h"
#include "../mf_proto/include/proto.h"
#include "log_def.h"

using namespace std;
using namespace lc;
using namespace su;
using namespace mf;


mf::UserClient(mf::MfClientMgr &mgr)
	:m_mgr(mgr)
{

}

void mf::UserClient::OnRecv(const lc::MsgPack &msg)
{
	mf::MsgData msg_data;
	if (!msg_data.Parse(msg.data, msg.len))
	{
		L_ERROR("parse mf::MsgData fail");
		return;
	}
	switch (msg_data.ctrl_cmd)
	{
	case CMD_NTF_COM:
	{
		MsgNtfCom ntf;
		L_COND(CtrlMsgProto::Parse(msg_data, ntf), "Parse CtrlMsgProto fail");
		ParseNtf(ntf);
	}
	break;
	case CMD_RSP_REG:
	{
		//接收到就是成功
		m_mgr.OnCon();
	}
	break;
	case CMD_RSP_CON:
	{
		MsgRspCon rsp;
		L_COND(CtrlMsgProto::Parse(msg_data, rsp), "Parse CtrlMsgProto fail");
		L_COND(rsp.dst_id, "Parse CtrlMsgProto fail, dst_id == 0");
		if (rsp.is_ok)
		{
			m_mgr.OnUserCon(rsp.dst_id);
		}
		else
		{
			m_mgr.OnUserDiscon(rsp.dst_id);
		}
	}
	break;
	case CMD_REQ_FORWARD:
	{
		MsgReqForward rsp;
		L_COND(CtrlMsgProto::Parse(msg_data, rsp), "Parse CtrlMsgProto fail");
		L_COND(rsp.dst_id == m_mgr.GetSvrId());
		m_mgr.OnRecv(rsp.src_id, msg_data.custom_pack, msg_data.custom_len);
	}
		break;
	case CMD_NTF_DISCON:
	{
		MsgNtfDiscon ntf;
		L_COND(CtrlMsgProto::Parse(msg_data, ntf), "Parse CtrlMsgProto fail");
		m_mgr.OnUserDiscon(ntf.svr_id);
	}
		break;
	case CMD_REQ_BROADCAST:
	{
		MsgReqBroadcast rsp;
		L_COND(CtrlMsgProto::Parse(msg_data, rsp), "Parse CtrlMsgProto fail");
		m_mgr.OnRecv(rsp.src_id, msg_data.custom_pack, msg_data.custom_len);
	}
		break;
	default:
		L_ERROR("unknow mf Cmd =%d", msg_data.ctrl_cmd);
		break;
	}
}


bool mf::UserClient::SendPack(const string &tcp_pack)
{
	L_COND_F(!tcp_pack.empty());
	MsgPack msg_pack;
	msg_pack.Serialize(tcp_pack);
	return client->SendData(msg_pack);
}


void mf::UserClient::ParseNtf(const MsgNtfCom &ntf)
{
	switch (ntf.req_cmd)
	{
	default:
		L_ERROR("unknow ntf.req_cmd cmd =%d", ntf.req_cmd);
		break;
	}
}

void mf::UserClient::OnConnected()
{
	MsgReqReg send;
	send.svr_id = m_mgr.GetSvrId();
	send.group_id = m_mgr.GetGroupId();

	L_COND(SendCtrlMsg(CMD_REQ_CON, send));
}

void mf::UserClient::OnDisconnected()
{
	m_mgr.OnOneMfDisconnect();
}


mf::MfClientMgr::~MfClientMgr()
{
	FOR_IT(m_vec_con)
	{
		delete *it;
	}
}

bool mf::MfClientMgr::Init(const vector<MfAddr> &vec_mf_addr, uint32 svr_id, uint32 group_id)
{
	L_COND_F(svr_id);
	L_COND_F(!vec_mf_addr.empty());
	bool ret = false;
	event_base *eb = EventMgr::Obj().GetEventBase();
	if (nullptr == eb)
	{
		L_ERROR("you should call EventMgr::Obj().Init() before call BaseClientMgr::Init");
		return false;
	}
	m_svr_id = svr_id;
	m_grp_id = group_id;
	FOR_IT_CONST(vec_mf_addr)
	{
		const MfAddr &addr = *it;
		UserClient *p = new UserClient();
		L_COND_F(p);
		ret = p->ConnectInit(addr.ip.c_str(), addr.port);
		L_COND_F(ret);
		m_vec_con.push_back(p);
	}
	return true;
}


bool mf::MfClientMgr::Send(uint32 dst_id, const char *pack, uint16 pack_len)
{
	L_COND_F(dst_id);
	L_COND_F(pack);
	L_COND_F(pack_len);
	UserClient* client = BlSelectSvr();
	L_COND_F(client);

	MsgReqForward send;
	send.src_id = m_svr_id;
	send.dst_id = dst_id;

	string tcp_pack;
	CtrlMsgProto::Serialize(CMD_REQ_FORWARD, send, pack, pack_len, tcp_pack);
	MsgPack msg_pack;
	msg_pack.Serialize(tcp_pack);

	client->SendData(msg_pack);
	return true;
}


void mf::MfClientMgr::SendGroup(uint32 group_id, const char *pack, uint16 pack_len)
{
	L_COND(pack);
	L_COND(pack_len);
	UserClient* client = BlSelectSvr();
	L_COND(client);

	MsgReqBroadcast send;
	send.src_id = m_svr_id;
	send.group_id = group_id;

	string tcp_pack;
	CtrlMsgProto::Serialize(CMD_REQ_BROADCAST, send, pack, pack_len, tcp_pack);
	MsgPack msg_pack;
	msg_pack.Serialize(tcp_pack);

	client->SendData(msg_pack);
}

void mf::MfClientMgr::TryReconMf()
{
	FOR_IT(m_vec_con)
	{
		UserClient *p = *it;
		if (p->IsConnect())
		{
			continue;
		}
		p->TryReconnect();
	}
}


void mf::MfClientMgr::OnOneMfDisconnect()
{
	bool is_connect = false;
	FOR_IT(m_vec_con)
	{
		UserClient *p = *it;
		if (p->IsConnect())
		{
			is_connect = true;
			return;
		}
	}
	//all mf disconnect
	OnDiscon();
}

mf::UserClient* mf::MfClientMgr::BlSelectSvr()
{
	L_COND_R(!m_vec_con.empty(), nullptr);

	for (uint32 i = 0; i < m_vec_con.size(); ++i)
	{
		m_lb_idx++;
		if (m_lb_idx >= m_vec_con.size())
		{
			m_lb_idx = 0;
		}
		UserClient *client = m_vec_con[m_lb_idx];
		if (client->IsConnect())
		{
			return client;
		}
	}
	return nullptr;
}
