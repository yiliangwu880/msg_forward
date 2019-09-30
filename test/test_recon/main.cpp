#include <string>
#include "libevent_cpp/include/include_all.h"
#include "svr_util/include/su_mgr.h"
#include "svr_util/include/single_progress.h"
#include "svr_util/include/log_file.h"
#include "unit_test.h"

using namespace std;
using namespace su;
using namespace lc;

void MyUnitTestPrintf(bool is_error, const char * file, int line, const char *fun, const char * pattern, va_list vp)
{
	su::LogLv lv = su::LL_INFO;
	if (is_error)
	{
		lv = su::LL_ERROR;
	}
	su::LogMgr::Obj().Printf(lv, file, line, fun, pattern, vp);

}

int main(int argc, char* argv[])
{
	//su::LogMgr::Obj().Printf(su::LL_ERROR, "", 1, "", "abc");
	UnitTestMgr::Obj().Start(MyUnitTestPrintf);
	return 0;
}

