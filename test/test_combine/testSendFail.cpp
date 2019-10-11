//测试 广播断开user到all user
//可靠转发，连接失败才需要反馈给user。 (类似tcp连接那样理解)
//重复mf_svr 地址，无效mf_svr地址
//MfClientMgr::DisConnect
//MfClientMgr::OnRegFail

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
	void DelU2();
	void DelU3();
	class UserBase : public MfClientMgr
	{
	public:
	public:
		UserBase()
		{}

		bool Send(uint32 dst_id, const string &s)
		{
			return MfClientMgr::Send(dst_id, s.c_str(), s.length());
		}
		bool SendGroup(uint32 grp_id, const string &s)
		{
			return MfClientMgr::SendGroup(grp_id, s.c_str(), s.length());
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

		}

	};

	class U4 : public UserBase, public Singleton<U4>
	{
	public:
		bool m_is_discon;
		uint32 rev_num;
		U4()
			:m_is_discon(false)
			, rev_num(0)
		{
		}
		virtual ~U4() {}
	private:
		virtual void OnCon()
		{
			UNIT_INFO("U4 OnCon");
		}
		virtual void OnRecv(uint32 src_id, const char *custom_pack, uint16 custom_pack_len)
		{
		}
		virtual void OnDiscon()
		{
			UNIT_INFO("U4 discon");
			m_is_discon = true;
		}
	};

	class U5 : public UserBase, public Singleton<U5>
	{
	public:
		bool m_is_reg_fail;
		uint32 rev_num;
		U5()
			:m_is_reg_fail(false)
			, rev_num(0)
		{
		}
		virtual ~U5() {}
	private:
		virtual void OnCon()
		{
			UNIT_INFO("U5 OnCon");
		}
		virtual void OnRecv(uint32 src_id, const char *custom_pack, uint16 custom_pack_len)
		{
		}
		virtual void OnDiscon()
		{
			UNIT_INFO("U5 discon");
		}
		virtual void OnRegFail()
		{
			UNIT_INFO("U5 OnRegFail");
			m_is_reg_fail = true;
			EventMgr::Obj().StopDispatch();
		}
	};

	class U1 : public UserBase, public Singleton<U1>
	{
	public:
		bool m_is_ok;
		uint32 rev_num;
		bool m_is_wait_fail;
		U1()
			:m_is_ok(false)
			, rev_num(0)
			, m_is_wait_fail(false)
		{
		}
	private:
		virtual void OnCon()
		{
			UNIT_INFO("U1 OnCon");
			Send(2, "1");
		}
		virtual void OnRecv(uint32 src_id, const char *custom_pack, uint16 custom_pack_len)
		{
			string s(custom_pack, custom_pack_len);
			UNIT_ASSERT(s == "2");
			UNIT_ASSERT(src_id == 2);
			DelU2();
			Send(2, "3");
			m_is_wait_fail = true;
			UNIT_INFO("del u2, wait fail");
		}

		virtual void OnUserDiscon(uint32 dst_id)
		{
			if (dst_id == 2)
			{
				UNIT_ASSERT(m_is_wait_fail);
				UNIT_INFO("del u3");
				DelU3();
			}
			else if (dst_id == 3)
			{
				m_is_ok = true;
				UNIT_INFO("u3 disconnect, StopDispatch");
			}


		}
	};

	class U2 : public UserBase
	{
	public:
		bool m_is_ok;
		uint32 rev_num;
		U2()
			:m_is_ok(false)
			, rev_num(0)
		{
		}
		virtual ~U2(){}
	private:
		virtual void OnCon()
		{
			UNIT_INFO("U2 OnCon");
		}
		virtual void OnRecv(uint32 src_id, const char *custom_pack, uint16 custom_pack_len)
		{
			string s(custom_pack, custom_pack_len);
			UNIT_ASSERT(s == "1");
			UNIT_ASSERT(src_id == 1);
			Send(1, "2");
			UNIT_INFO("u2 rev u1 msg.");
			U4::Obj().DisConnect();
		}
	};

	class U3 : public UserBase
	{
	public:
		bool m_is_ok;
		uint32 rev_num;
		U3()
			:m_is_ok(false)
			, rev_num(0)
		{
		}
		virtual ~U3() {}
	private:
		virtual void OnCon()
		{
			UNIT_INFO("U3 OnCon");
		}
		virtual void OnRecv(uint32 src_id, const char *custom_pack, uint16 custom_pack_len)
		{
		}		
		virtual void OnDiscon()
		{
			UNIT_INFO("U3 discon");
		}
		virtual void OnRegFail()
		{
			UNIT_INFO("U3 OnRegFail");
		}
	};

	U2 *g_pU2 = nullptr;
	U3 *g_pU3 = nullptr;
	void DelU2()
	{
		delete g_pU2;
		g_pU2 = nullptr;
	}
	void DelU3()
	{
		delete g_pU3;
		g_pU3 = nullptr;
	}
}

UNITTEST(cd_testSendFail)
{
	lc::Timer tm;
	auto f = []()
	{
		UNIT_INFO("start connect");
		std::vector < MfAddr > vec_addr = CfgMgr::Obj().GetVecAddr();
		for (auto &v : CfgMgr::Obj().GetVecAddr())
		{
			vec_addr.push_back(v);
		}
		vec_addr.push_back({ "127.0.0.1", 33454 }); //加个无效的

		g_pU2 = new U2;
		g_pU3 = new U3;
		g_pU3->Init(vec_addr, 3);
		g_pU2->Init(vec_addr, 2);
		U1::Obj().Init(vec_addr, 1);
		U4::Obj().Init(vec_addr, 4);
		vec_addr.clear(); 
		vec_addr.push_back({ "127.0.0.1", 33454 }); //加个无效的
		U5::Obj().Init(vec_addr, 5);
		
		UNIT_INFO("end connect");
	};
	tm.StartTimer(2000, f); //延时运行，等上次测试状态结束

	EventMgr::Obj().Dispatch();

	UNIT_ASSERT(U1::Obj().m_is_ok);
	UNIT_ASSERT(U4::Obj().m_is_discon);
	UNIT_ASSERT(U5::Obj().m_is_reg_fail);
	UNIT_INFO("cd_testSendFail end");
}