
#include "mf_driver.h"
#include "../mf_proto/include/proto.h"
#include "log_def.h"

using namespace std;
using namespace lc;
using namespace su;
using namespace mf;


 
void mf::UserClient::OnRecv(const lc::MsgPack &msg)
{
	mf::MsgData msg_data;
	L_COND(msg_data.Parse(msg.data, msg.len));

	switch (msg_data.ctrl_cmd)
	{
	case CMD_NTF_COM:
	{
		MsgNtfCom ntf;
		L_COND(CtrlMsgProto::Parse(msg_data, ntf));
		ParseNtf(ntf);
	}
	break;
	case CMD_RSP_REG:
	{
		m_is_reg = true;
		//接收到就是成功
		if (!m_mgr.m_is_coning)
		{
			m_mgr.OnCon();
			m_mgr.m_is_coning = true;
			m_mgr.m_reg_tm.StopTimer();
		}
		//L_TRACE("rev CMD_RSP_REG");
	}
	break;
	case CMD_RSP_CON:
	{
		MsgRspCon rsp;
		L_COND(CtrlMsgProto::Parse(msg_data, rsp));
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
		L_COND(CtrlMsgProto::Parse(msg_data, rsp));
		L_COND(rsp.dst_id == m_mgr.GetSvrId());
		m_mgr.OnRecv(rsp.src_id, msg_data.custom_pack, msg_data.custom_len);
	}
	break;
	case CMD_NTF_DISCON:
	{
		MsgNtfDiscon ntf;
		L_COND(CtrlMsgProto::Parse(msg_data, ntf));
		m_mgr.OnUserDiscon(ntf.svr_id);
	}
	break;
	case CMD_REQ_BROADCAST:
	{
		MsgReqBroadcast rsp;
		L_COND(CtrlMsgProto::Parse(msg_data, rsp));
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

	return lc::ClientCon::SendPack(tcp_pack);
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
	send.group_id = m_mgr.GetGrpId();
	L_TRACE("OnConnected, req reg");
	L_COND(SendCtrlMsg(CMD_REQ_REG, send));
}

void mf::UserClient::OnDisconnected()
{
	m_is_reg = false;
	m_mgr.OnOneMfDiscon();
}


mf::MfClientMgr::~MfClientMgr()
{
	DisConnect();
}

void mf::MfClientMgr::DisConnect()
{
	m_lb_idx = 0;
	m_svr_id = 0;
	m_grp_id = 0;
	m_is_coning = false;
	for (UserClient * p : m_vec_con)
	{
		delete p;
	}
	m_vec_con.clear();
}

bool mf::MfClientMgr::Init(const vector<MfAddr> &vec_mf_addr, uint32 svr_id, uint32 group_id)
{
	L_COND_F(svr_id);
	L_COND_F(!vec_mf_addr.empty());
	event_base *eb = EventMgr::Obj().GetEventBase();
	if (nullptr == eb)
	{
		L_ERROR("you should call EventMgr::Obj().Init() before call BaseClientMgr::Init");
		return false;
	}
	if (0 != m_svr_id)
	{
		L_ERROR("repeated init");
		return false;
	}

	m_svr_id = svr_id;
	m_grp_id = group_id;
	bool ret = false;
	for(const MfAddr &addr : vec_mf_addr)
	{
		UserClient *p = new UserClient(*this);
		L_COND_F(p);
		L_DEBUG("connect to %s %d", addr.ip.c_str(), addr.port);
		ret = p->ConnectInit(addr.ip.c_str(), addr.port);
		if (!ret)
		{
			L_ERROR("UserClient ConnectInit fail.");
			delete p;
			return false;
		}
		m_vec_con.push_back(p);
	}
	auto reg_time_out = [&]()//this析构，m_reg_tm也会析构，保证StopTime,不调用野指针
	{
		this->OnRegFail();
	};
	m_reg_tm.StartTimer(2000, reg_time_out);
	return true;
}

bool mf::MfClientMgr::Send(uint32 dst_id, const char *pack, uint16 pack_len)
{
	L_COND_F(dst_id);
	L_COND_F(pack);
	L_COND_F(pack_len);
	UserClient* client = BlSelectSvr();
	L_COND_F(client, "BlSelectSvr fail");

	MsgReqForward send;
	send.src_id = m_svr_id;
	send.dst_id = dst_id;

	string tcp_pack;
	MsgData::Serialize(CMD_REQ_FORWARD, send, pack, pack_len, tcp_pack);

	return client->SendPack(tcp_pack);
}


bool mf::MfClientMgr::SendGroup(uint32 group_id, const char *pack, uint16 pack_len)
{
	L_COND_F(pack);
	L_COND_F(pack_len);
	UserClient* client = BlSelectSvr();
	L_COND_F(client);

	MsgReqBroadcast send;
	send.src_id = m_svr_id;
	send.group_id = group_id;

	string tcp_pack;
	MsgData::Serialize(CMD_REQ_BROADCAST, send, pack, pack_len, tcp_pack);
	return client->SendPack(tcp_pack);
}


void mf::MfClientMgr::ConUser(uint32 dst_id)
{
	UserClient* client = BlSelectSvr();
	L_COND(client);

	MsgReqCon send;
	send.dst_id = dst_id;

	client->SendCtrlMsg(CMD_REQ_CON, send);
}

void mf::MfClientMgr::TryReconMf()
{
	for(UserClient * p : m_vec_con)
	{
		L_COND(p);
		if (!p->IsWaitConnectReq())
		{
			continue;
		}
		L_TRACE("TryReconnect");
		p->TryReconnect();
	}
}


void mf::MfClientMgr::OnOneMfDiscon()
{

	for (UserClient * p : m_vec_con)
	{
		if (p->IsConnect())
		{
			return;
		}
	}
	//all mf disconnect
	OnDiscon();
	m_is_coning = false;
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
		if (client->IsReg())
		{
			return client;
		}
	}
	L_ERROR("all mf client disconnect");
	return nullptr;
}

void mf::MfClientMgr::OnRegFail()
{
	L_FATAL("reg mf fail, svr_id=%x", m_svr_id);
}