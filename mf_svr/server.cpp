#include "server.h"
#include "svr_util/include/read_cfg.h"

bool Server::Init()
{
	L_COND_F(CfgMgr::Obj().Init());
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
{

}

bool CfgMgr::Init()
{
	L_DEBUG("init cfg");
	su::Config cfg;
	L_COND_F(cfg.init("cfg/cfg.txt"));

	m_port = (uint16)cfg.GetInt("port");
	L_DEBUG("port=%d", m_port);
	return true;
}
