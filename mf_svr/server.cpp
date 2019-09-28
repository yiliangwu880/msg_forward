#include "server.h"
#include "svr_util/include/read_cfg.h"

bool Server::Init()
{
	bool r = m_listener.Init(CfgMgr::Obj().GetSvrPort());
	L_COND_F(r);


	L_DEBUG("Server::Init ok");
	return true;
}

lc::BaseConMgr & Server::GetBaseConMgr()
{
	return m_listener.GetConnMgr();
}

CfgMgr::CfgMgr()
	:m_port(0)
	, is_daemon(false)
{

}

bool CfgMgr::Init()
{
	L_DEBUG("init cfg");
	su::Config cfg;
	L_COND_F(cfg.init("mf_svr_cfg.txt"));

	m_port = (uint16)cfg.GetInt("port");
	L_DEBUG("port=%d", m_port);
	is_daemon = (bool)cfg.GetInt("is_daemon");
	L_DEBUG("is_daemon=%d", is_daemon);

	return true;
}
