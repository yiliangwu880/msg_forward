#pragma once
#include <string>
#include <vector>
#include <map>
#include "svr_util/include/su_mgr.h"
#include "../mf_proto/include/proto.h"
#include "libevent_cpp/include/connector.h"
#include "log_def.h"

class MfSvrCon;
//尽量不要构造，析构函数写逻辑。因为User放MAP里面的，会经常多次构造，析构。
class User
{
public:
	User(uint32 id, uint64 con_id, uint32 group_id);

	MfSvrCon *GetConnect();

	template<class CtrlMsg>
	bool Send(mf::Cmd cmd, const CtrlMsg &send);

	bool SendLcMsg(const lc::MsgPack &msg_pack);

	uint32 GetGroupId() const { return m_group_id; }
	uint32 GetId() const { return m_id; }
	//return false 表示连接已经断开，等删除对象。
	bool IsConnect(); 
private:
	uint32 m_id;
	uint64 m_con_id; //ConnectClient id
	uint32 m_group_id;
};

class UserMgr : public Singleton<UserMgr>
{
public:
	bool RegUser(MfSvrCon &con, const mf::MsgData &msg);
	bool UnregUser(uint32 user_id);
	User *GetUser(uint32 user_id);

private:
	std::map<uint32, User> m_id_2_user;
};



////////////////////define////////////////////
template<class CtrlMsg>
bool User::Send(mf::Cmd cmd, const CtrlMsg &send)
{
	std::string tcp_pack;
	mf::MsgData::Serialize(cmd, send, nullptr, 0, tcp_pack);
	lc::MsgPack msg_pack;
	COND_F(msg_pack.Serialize(tcp_pack));
	return SendLcMsg(msg_pack);
}