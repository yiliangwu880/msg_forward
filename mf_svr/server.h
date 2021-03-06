#pragma once
#include <string>
#include <vector>
#include "libevent_cpp/include/include_all.h"
#include "log_def.h"
#include "svr_util/include/singleton.h"
#include "svr_util/include/easy_code.h"
#include "tcp_task.h"

class Server: public Singleton<Server>
{
public:
	bool Init();
	lc::BaseConMgr &GetBaseConMgr();

private:
	lc::Listener<MfSvrCon> m_listener;
};

class CfgMgr : public Singleton<CfgMgr>
{
public:
	CfgMgr();
	bool Init();

	unsigned short GetSvrPort() const { return m_port; }
	const char *GetSvrIp() const { return m_ip.c_str(); }
	bool IsDaemon()const { return is_daemon; }
private:
	unsigned short m_port;
	std::string m_ip;
	bool is_daemon;
};