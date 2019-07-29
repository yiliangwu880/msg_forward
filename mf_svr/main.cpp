#include <string>
#include "libevent_cpp/include/include_all.h"
#include "log_def.h"
#include "svr_util/include/singleton.h"
#include "server.h"
#include "svr_util/include/single_progress.h"


using namespace su;

class MyLog : public lc::ILogPrinter, public Singleton<MyLog>
{
public:
	virtual void Printf(lc::LogLv lv, const char * file, int line, const char *fun, const char * pattern, va_list vp) override
	{
		m_log.Printf((su::LogLv)lv, file, line, fun, pattern, vp);
	}
	
private:
	su::DefaultLog m_log;

};


void OnExitProccess()
{
	L_DEBUG("OnExitProccess");

}

void OnLoopTimer()
{
	//L_DEBUG("on");
	su::TimeDriver::Obj().CheckTimeOut();
}

int main(int argc, char* argv[]) 
{
	SPCheckArg::Obj().CheckMainArg("mf_svr", argc, argv, OnExitProccess);

	lc::LogMgr::Obj().SetLogPrinter(MyLog::Obj());
	lc::EventMgr::Obj().Init();

	lc::Timer loop_tm;
	loop_tm.StartTimer(1000, OnLoopTimer, true);
	if (!Server::Obj().Init())
	{
		L_ERROR("server init fail");
		return 0;
	}

	lc::EventMgr::Obj().Dispatch();

	return 0;
}

