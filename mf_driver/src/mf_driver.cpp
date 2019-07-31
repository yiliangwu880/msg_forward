
#include "mf_driver.h"
#include "../mf_proto/include/proto.h"
#include "log_def.h"

using namespace std;
using namespace lc;
using namespace su;

void mf::UserClient::OnRecv(const lc::MsgPack &msg)
{

}

void mf::UserClient::OnConnected()
{

}

void mf::UserClient::OnDisconnected()
{

}


mf::BaseClientMgr::~BaseClientMgr()
{
	FOR_IT(m_vec_con)
	{
		delete *it;
	}
}

bool mf::BaseClientMgr::Init(const vector<MfAddr> &vec_svr_addr, uint32 user_id)
{
	L_COND_F(user_id);
	L_COND_F(!vec_svr_addr.empty());
	bool ret = false;
	event_base *eb = EventMgr::Obj().GetEventBase();
	if (nullptr == eb)
	{
		L_ERROR("you should call EventMgr::Obj().Init() before call BaseClientMgr::Init");
		return false;
	}
	m_user_id = user_id;
	FOR_IT_CONST(vec_svr_addr)
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


bool mf::BaseClientMgr::Send(uint32 dst_id, const char *pack, uint16 pack_len)
{
	L_COND_F(dst_id);
	L_COND_F(pack);
	L_COND_F(pack_len);
	UserClient* client = BlSelectSvr();
	L_COND_F(client);

	MsgReqForward send;
	send.src_id = m_user_id;
	send.dst_id = dst_id;

	string tcp_pack;
	CtrlMsgProto::Serialize(CMD_REQ_FORWARD, send, pack, pack_len, tcp_pack);
	MsgPack msg_pack;
	msg_pack.Serialize(tcp_pack);

	client->SendData(msg_pack);
	return true;
}


void mf::BaseClientMgr::SendGeroup(uint32 group_id, const char *pack, uint16 pack_len)
{
	L_COND(pack);
	L_COND(pack_len);
	UserClient* client = BlSelectSvr();
	L_COND(client);

	MsgReqBroadcast send;
	send.src_id = m_user_id;
	send.group_id = group_id;

	string tcp_pack;
	CtrlMsgProto::Serialize(CMD_REQ_BROADCAST, send, pack, pack_len, tcp_pack);
	MsgPack msg_pack;
	msg_pack.Serialize(tcp_pack);

	client->SendData(msg_pack);
}

void mf::BaseClientMgr::TryReconnectSvr()
{
	FOR_IT(m_vec_con)
	{
		UserClient *p = *it;
		p->TryReconnect();
	}
}

mf::UserClient* mf::BaseClientMgr::BlSelectSvr()
{
	L_COND_R(!m_vec_con.empty(), nullptr);

	m_lb_idx++;
	if (m_lb_idx >= m_vec_con.size())
	{
		m_lb_idx = 0;
	}
	return m_vec_con[m_lb_idx];
}
