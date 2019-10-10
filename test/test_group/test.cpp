//综合测试

#include <string>
#include "libevent_cpp/include/include_all.h"
#include "svr_util/include/su_mgr.h"
#include "svr_util/include/single_progress.h"
#include "svr_util/include/read_cfg.h"
#include "unit_test.h"
#include "../mf_driver/include/mf_driver.h"
#include "cfg.h"

using namespace std;
using namespace su;
using namespace mf;
using namespace lc;


namespace
{
	static bool g_is_done = false;
	void  MainMgr_ConUser();


class Group1U1 : public MfClientMgr, public Singleton<Group1U1>
{
public:
public:
	Group1U1()
	{}

	bool Send(uint32 dst_id, const string &s)
	{
		return MfClientMgr::Send(dst_id, s.c_str(), s.length());
	}

private:
	//反馈连接mf svr list 情况。能连接任意一台都算成功。
	virtual void OnCon()
	{
		UNIT_INFO("CloseMgr OnCon, MainMgr::ConUser(2)");
		UNIT_ASSERT(WAIT_CON == m_status);
		MainMgr_ConUser();
		UNIT_ASSERT(GetSvrId() == 2);
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
		string s(custom_pack, custom_pack_len);
		UNIT_INFO("CloseMgr OnRecv [%s]", s.c_str());
		UNIT_ASSERT(1 == src_id);
		Send(src_id, s);
		SendGroup(0, "bbb", 3);
	}

};




}

UNITTEST(cd)
{
	UNIT_ASSERT(CfgMgr::Obj().Init());
	EventMgr::Obj().Init();

	MainMgr::Obj().Init(CfgMgr::Obj().GetVecAddr(), 1);


	EventMgr::Obj().Dispatch();

}