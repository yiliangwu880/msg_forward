#include <string>
#include "libevent_cpp/include/include_all.h"
#include "log_def.h"
#include "server.h"
#include "svr_util/include/su_mgr.h"
#include "svr_util/include/single_progress.h"



using namespace su;
using namespace std;

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


int main(int argc, char* argv[]) 
{
	SuMgr::Obj().Init();

	{//start or stop proccess
		if (argc == 2 && string("stop") == argv[1])
		{
			SPCheckArg::Obj().Stop("mf_svr");
			return 0;
		}
		SPCheckArg::Obj().Start("mf_svr", OnExitProccess);
	}
	lc::EventMgr::Obj().Init(&MyLog::Obj());

	{//Çý¶¯su::timer
		static lc::Timer loop_tm;
		auto fun = std::bind(&SuMgr::OnTimer, &SuMgr::Obj());
		loop_tm.StartTimer(30, fun, true);
	}

	if (!Server::Obj().Init())
	{
		L_ERROR("server init fail");
		return 0;
	}

	lc::EventMgr::Obj().Dispatch();

	return 0;
}

