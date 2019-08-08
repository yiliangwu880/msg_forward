#include "tcp_task.h"
#include "user.h"
#include "group.h"
#include "MsgDispatch.h"

using namespace mf;

MfSvrCon::MfSvrCon()
	:m_state(S_INIT)
	,m_user_id(0)
	, m_cur_msg(nullptr)
{

}

MfSvrCon::~MfSvrCon()
{
	if (0 != m_user_id)
	{
		UserMgr::Obj().UnregUser(m_user_id);
	}
}

void MfSvrCon::SetUserId(uint32 id)
{
	m_user_id = id;
}

void MfSvrCon::OnRecv(const lc::MsgPack &msg)
{
	m_cur_msg = &msg;
	MsgData data;
	L_COND(data.Parse(msg.data, msg.len));

	if (S_INIT == m_state)
	{
		bool r = UserMgr::Obj().RegUser(*this, data);
		if (r)
		{
			m_state = S_REG;
		}
		return;
	}
	//验证，注册通过

	MsgDispatch::Obj().DispatchMsg(*this, data);
	
}
