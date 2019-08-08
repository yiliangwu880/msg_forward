#pragma once

#include <string>
#include <vector>
#include "protocol.h"
#include "log_def.h"
#include "libevent_cpp/include/include_all.h"
#include "../mf_proto/include/proto.h"

class MfSvrCon : public lc::SvrCon
{
public:
	MfSvrCon();
	~MfSvrCon();
	void SetUserId(uint32 id);
	uint32 GetUserId() { return m_user_id; }
	//获取当前接收的tcp包
	const lc::MsgPack *GetCurMsgPack() { return m_cur_msg; }

private:
	virtual void OnRecv(const lc::MsgPack &msg) override;
	virtual void OnConnected() override
	{
	}

private:
	enum State
	{
		S_INIT, //刚连接进来，没验证
		S_REG,//已经注册成功
	}; 
	
	State m_state;
	uint32 m_user_id;//用户id,svr id.
	const lc::MsgPack *m_cur_msg; //当前接收的tcp包
};