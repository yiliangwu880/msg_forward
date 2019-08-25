#pragma once
#include <string>
#include <vector>
#include <map>
#include "svr_util/include/su_mgr.h"
#include "../mf_proto/include/proto.h"
#include "libevent_cpp/include/connector.h"

class MfSvrCon;

typedef void(*HandleMsg)(User &user, MfSvrCon &con, const mf::MsgData &msg);
class MsgDispatch : public Singleton<MsgDispatch>
{
public:
	void Init();

	void DispatchMsg(MfSvrCon &con, const mf::MsgData &msg);

private:
	std::map<mf::Cmd, HandleMsg> m_cmd_2_handle;
};
