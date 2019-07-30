#include "tcp_task.h"
#include "user.h"
#include "group.h"

using namespace mf;

ConnectClient::ConnectClient()
	:m_state(S_INIT)
	,m_user_id(0)
	, m_cur_msg(nullptr)
{

}

void ConnectClient::SetUserId(uint32 id)
{
	m_user_id = id;
}

void ConnectClient::OnRecv(const lc::MsgPack &msg)
{
	m_cur_msg = &msg;
	MsgData data;
	L_COND(data.Parse(msg.data, msg.len));

	if (S_INIT == m_state)
	{
		UserMgr::Obj().RegUser(*this, data);
		return;
	}

	UserMgr::Obj().DispatchMsg(*this, data);
	
}
