//测试多个mf_svr的可用性

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
	static bool g_is_done = false;
	void  MainMgr_ConUser();

	static lc::Timer s_tm;

class MfClient : public MfClientMgr, public Singleton<MfClient>
{
public:
	enum Status
	{
		WAIT_CON,
		WAIT_ECHO_SVR_CON,
		RUN_ECHO, //不停发收
	};
	Status m_status; 
	lc::Timer m_tm;
	lc::Timer m_send_tm;
	uint32 m_repeated_user_con_cnt;
public:
	MfClient()
		:m_status(WAIT_CON)
		, m_repeated_user_con_cnt(0)
	{}

	bool Send(uint32 dst_id, const string &s)
	{
		return MfClientMgr::Send(dst_id, s.c_str(), s.length());
	}

private:
	//反馈连接mf svr list 情况。能连接任意一台都算成功。
	virtual void OnCon()
	{
		UNIT_ASSERT(m_status == WAIT_CON);
		m_status = WAIT_ECHO_SVR_CON;

		auto f = []()
		{
			MfClient::Obj().ConUser(ECHO_SVR_ID);
		};
		m_tm.StartTimer(1000, f, true);
	}

	//全部连接都失败就反馈。
	virtual void OnDiscon()
	{
		UNIT_INFO("---------OnDiscon-----------");
		UNIT_ASSERT(m_status == WAIT_CON);
	}

	virtual void OnUserCon(uint32 dst_id)
	{
		UNIT_INFO("OnUserCon %d", dst_id);
		m_repeated_user_con_cnt++;
		UNIT_ASSERT(dst_id == ECHO_SVR_ID);
		if (WAIT_ECHO_SVR_CON == m_status)
		{
			m_status = RUN_ECHO;
			Send(ECHO_SVR_ID, "echo");
			UNIT_INFO("start send echo");
		}
		else
		{//重复连接
			m_tm.StopTimer();
			UNIT_ASSERT(m_repeated_user_con_cnt < 3); //重复连接次数太多，情况不明。
		}
	}

	//链接对方失败，或者对方主动断线，都会调用。
	virtual void OnUserDiscon(uint32 dst_id)
	{
		UNIT_INFO("OnUserDiscon %d", dst_id);
		UNIT_ASSERT(false);
	}

	//@para src_id 发送方服务器id
	virtual void OnRecv(uint32 src_id, const char *custom_pack, uint16 custom_pack_len)
	{
		UNIT_ASSERT(RUN_ECHO == m_status);
		UNIT_INFO("rev msg from echo");
		UNIT_ASSERT(ECHO_SVR_ID == src_id);
		string s(custom_pack, custom_pack_len);
		UNIT_ASSERT(s == "echo");
		g_is_done = true;

		auto f = []()
		{
			MfClient::Obj().Send(ECHO_SVR_ID, "echo");
		};
		m_send_tm.StartTimer(1000, f);
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
	if (!g_is_done)
	{
		UNIT_ERROR("connect mf_svr fail, mabe your haven't start  mf_svr");
	}
	UNIT_ASSERT(g_is_done);
	UNIT_INFO("-------end----------")
}