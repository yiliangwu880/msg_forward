#pragma once

#include <string>
#include <vector>
#include "protocol.h"
#include "log_def.h"
#include "libevent_cpp/include/include_all.h"
#include "../mf_proto/include/proto.h"

class ConnectClient : public lc::SvrCon
{
public:
	ConnectClient();
	void SetUserId(uint32 id);
	uint32 GetUserId() { return m_user_id; }
	//��ȡ��ǰ���յ�tcp��
	const lc::MsgPack *GetCurMsgPack() { return m_cur_msg; }

private:
	virtual void OnRecv(const lc::MsgPack &msg) override;
	virtual void OnConnected() override
	{
		//MsgPack msg;
		//...��ʼ��msg
		//	send_data(msg);
	}

private:
	enum State
	{
		S_INIT, //�����ӽ�����û��֤
		S_REG,//�Լ�ע��ɹ�
	}; 
	
	State m_state;
	uint32 m_user_id;//�û�id,svr id.
	const lc::MsgPack *m_cur_msg; //��ǰ���յ�tcp��
};