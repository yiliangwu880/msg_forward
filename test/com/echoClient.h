#pragma once
#include <string>
#include "libevent_cpp/include/include_all.h"
#include "svr_util/include/su_mgr.h"
#include "svr_util/include/single_progress.h"
#include "svr_util/include/read_cfg.h"
#include "../mf_driver/include/mf_driver.h"

using namespace su;
using namespace mf;
using namespace lc;

namespace
{
	static uint32 ECHO_SVR_ID = 55;
}

//回显cd
class EchoClient : public MfClientMgr, public Singleton<EchoClient>
{
public:

public:

	bool Send(uint32 dst_id, const std::string &s)
	{
		return MfClientMgr::Send(dst_id, s.c_str(), s.length());
	}

private:
	//反馈连接mf svr list 情况。能连接任意一台都算成功。
	virtual void OnCon()
	{
	}

	//全部连接都失败就反馈。
	virtual void OnDiscon()
	{
	}

	virtual void OnUserCon(uint32 dst_id)
	{
	}

	//链接对方失败，或者对方主动断线，都会调用。
	virtual void OnUserDiscon(uint32 dst_id)
	{
	}

	//@para src_id 发送方服务器id
	virtual void OnRecv(uint32 src_id, const char *custom_pack, uint16 custom_pack_len)
	{
		std::string s(custom_pack, custom_pack_len);
		Send(src_id, s);
		LB_DEBUG("echo msg %s", s.c_str());
	}

};