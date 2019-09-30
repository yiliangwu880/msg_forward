//测试重链

#include <string>
#include "libevent_cpp/include/include_all.h"
#include "svr_util/include/su_mgr.h"
#include "svr_util/include/single_progress.h"
#include "svr_util/include/read_cfg.h"
#include "unit_test.h"
#include "../mf_driver/include/mf_driver.h"
#include "cfg.h"
#include "svr_util/include/log_file.h"
#include "../com/echoClient.h"

using namespace std;
using namespace su;
using namespace mf;
using namespace lc;


namespace
{
	static bool g_is_end = false;
	void  MainMgr_ConUser();

	static lc::Timer s_tm;

class MfClient : public MfClientMgr, public Singleton<MfClient>
{
public:
	enum Status
	{
		WAIT_CON,
		WAIT_DISCON,

	};
	Status m_status;
	uint32 m_con_cnt;
public:
	MfClient()
		:m_status(WAIT_CON)
		, m_con_cnt(0)
	{}

	bool Send(uint32 dst_id, const string &s)
	{
		return MfClientMgr::Send(dst_id, s.c_str(), s.length());
	}

private:
	//反馈连接mf svr list 情况。能连接任意一台都算成功。
	virtual void OnCon()
	{
		m_con_cnt++;
		UNIT_INFO("OnCon %d", m_con_cnt);
		if (3==m_con_cnt)//1次初始化，2次重连
		{
			UNIT_INFO("send a1");
			Send(ECHO_SVR_ID, "a1");
		}
	}

	//全部连接都失败就反馈。
	virtual void OnDiscon()
	{
		UNIT_INFO("---------OnDiscon-----------");
	}

	virtual void OnUserCon(uint32 dst_id)
	{
		UNIT_INFO("OnUserCon %d", dst_id);
	}

	//链接对方失败，或者对方主动断线，都会调用。
	virtual void OnUserDiscon(uint32 dst_id)
	{
		UNIT_INFO("OnUserDiscon %d", dst_id);
	}

	//@para src_id 发送方服务器id
	virtual void OnRecv(uint32 src_id, const char *custom_pack, uint16 custom_pack_len)
	{
		UNIT_INFO("rev msg from echo");
		UNIT_ASSERT(ECHO_SVR_ID == src_id);
		string s(custom_pack, custom_pack_len);
		UNIT_ASSERT(s == "a1");

		g_is_end = true;

		EventMgr::Obj().StopDispatch();
	}

};


void SignalCB(int sig_type)
{
	if (SIGUSR1 == sig_type)
	{
		UNIT_INFO("rev SignalCB SIGUSR1");
		EventMgr::Obj().StopDispatch();
	}
}
}



UNITTEST(recon)
{
	UNIT_ASSERT(CfgMgr::Obj().Init());
	EventMgr::Obj().Init();
	EventMgr::Obj().RegSignal(SIGUSR1, SignalCB);
	MfClient::Obj().Init(CfgMgr::Obj().GetVecAddr(), 1);
	EchoClient::Obj().Init(CfgMgr::Obj().GetVecAddr(), ECHO_SVR_ID);

	auto f = []()
	{
		MfClient::Obj().TryReconMf();
		EchoClient::Obj().TryReconMf();
	};
	s_tm.StartTimer(1000, f, true);
	
	EventMgr::Obj().Dispatch();
	if (!g_is_end)
	{
		UNIT_ERROR("connect mf_svr fail, mabe your haven't start  mf_svr");
	}
	UNIT_ASSERT(g_is_end);
	UNIT_INFO("-------end----------")
}