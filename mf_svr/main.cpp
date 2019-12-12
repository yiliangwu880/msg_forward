#include <string>
#include "libevent_cpp/include/include_all.h"
#include "log_def.h"
#include "server.h"
#include "svr_util/include/su_mgr.h"
#include "svr_util/include/single_progress.h"
#include "user.h"
#include "MsgDispatch.h"
#include <signal.h>

using namespace su;
using namespace std;


class MyLog : public lc::ILogPrinter, public Singleton<MyLog>
{
public:
	virtual void Printf(lc::LogLv lv, const char * file, int line, const char *fun, const char * pattern, va_list vp) override
	{
		//m_log.Printf((su::LogLv)lv, file, line, fun, pattern, vp);
		su::LogMgr::Obj().Printf((su::LogLv)lv, file, line, fun, pattern, vp);
	}

private:
	//su::DefaultLog m_log;
};


void OnExitProccess()
{
	if (SingleProgress::Obj().IsExit())
	{
		L_INFO("============stop proccess============");
		lc::EventMgr::Obj().StopDispatch();
	}
}

namespace {
	bool Init()
	{
		{//驱动su::timer
			static lc::Timer loop_tm;
			auto fun = std::bind(&SuMgr::OnTimer, &SuMgr::Obj());
			L_COND_F(loop_tm.StartTimer(30, fun, true));
		}
		{//驱动su::timer
			static lc::Timer loop_tm;
			auto fun = std::bind(OnExitProccess);
			L_COND_F(loop_tm.StartTimer(1000, fun, true));
		}

		MsgDispatch::Obj().Init();
		if (!Server::Obj().Init())
		{
			L_ERROR("server init fail");
			return false;
		}
		return true;
	}

	void SignalCB(int sig_type)
	{
		if (SIGUSR1 == sig_type)
		{
			L_INFO("rev SIGUSR1");
			lc::EventMgr::Obj().StopDispatch();
		}
	}
}

int main(int argc, char* argv[])
{
	L_COND_F(CfgMgr::Obj().Init());

	if (CfgMgr::Obj().IsDaemon())
	{
		//当nochdir为0时，daemon将更改进城的根目录为root(“ / ”)。
		//当noclose为0是，daemon将进城的STDIN, STDOUT, STDERR都重定向到 / dev / null。
		L_COND_F(0==daemon(1, 0));
	}

	SuMgr::Obj().Init();

	//start or stop proccess
	SingleProgress::Obj().Check(argc, argv, "mf_svr");

	lc::EventMgr::Obj().Init(&MyLog::Obj());
	lc::EventMgr::Obj().RegSignal(SIGUSR1, SignalCB);

	if (!Init())
	{
		return 0;
	}

	lc::EventMgr::Obj().Dispatch();

	return 0;
}

