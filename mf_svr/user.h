#pragma once
#include <string>
#include <vector>
#include <map>
#include "svr_util/include/su_mgr.h"
#include "../mf_proto/include/proto.h"
#include "libevent_cpp/include/connector.h"

class MfSvrCon;
//尽量不要构造，析构函数写逻辑。因为放MAP里面的，会经常多次构造，析构。
class User
{
public:
	
	User(uint32 id, uint64 con_id, uint32 group_id);

	void DispatchMsg(MfSvrCon &con, const mf::MsgData &msg);
	MfSvrCon *GetConnect();

	bool SendMfMsg(mf::MsgData &msg_data);

	template<class CtrlMsg>
	bool Send(mf::MsgCmd cmd, const CtrlMsg &send);

	bool SendLcMsg(const lc::MsgPack &msg_pack);

	uint32 GetGroupId() const { return m_group_id; }
	uint32 GetId() const { return m_id; }
private:
	uint32 m_id;
	uint64 m_con_id; //ConnectClient id
	uint32 m_group_id;
};



typedef void (*HandleMsg)(User &user, MfSvrCon &con, const mf::MsgData &msg);
class UserMgr : public Singleton<UserMgr>
{
public:
	void Init();

	bool RegUser(MfSvrCon &con, const mf::MsgData &msg);
	bool UnregUser(uint32 user_id);
	User *GetUser(uint32 user_id);

	void DispatchMsg(MfSvrCon &con, const mf::MsgData &msg);

private:
	std::map<uint32, User> m_id_2_user;
	std::map<uint16, HandleMsg> m_cmd_2_handle;
};


////////////////////define////////////////////


template<class CtrlMsg>
bool User::Send(mf::MsgCmd cmd, const CtrlMsg &send)
{

	std::string tcp_pack;
	mf::CtrlMsgProto::Serialize(cmd, send, nullptr, 0, tcp_pack);
	lc::MsgPack msg_pack;
	msg_pack.len = tcp_pack.length();
	if (msg_pack.len < lc::MAX_MSG_DATA_LEN)
	{
		return false;
	}
	memcpy(msg_pack.data, tcp_pack.c_str(), tcp_pack.length());

	return SendLcMsg(msg_pack);
}