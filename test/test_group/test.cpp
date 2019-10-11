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
	class GroupBase : public MfClientMgr
	{
	public:
	public:
		GroupBase()
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

	class Group1U1 : public GroupBase, public Singleton<Group1U1>
	{
	public:
		bool m_rev_4;
		Group1U1()
			:m_rev_4(false)
		{
		}
	private:
		virtual void OnCon()
		{

		}
		virtual void OnRecv(uint32 src_id, const char *custom_pack, uint16 custom_pack_len)
		{
			string s(custom_pack, custom_pack_len);
			UNIT_ASSERT(s == "group");
			UNIT_ASSERT(src_id == 4);
			m_rev_4 = true;
		}
	};

	class Group1U2 : public GroupBase, public Singleton<Group1U2>
	{
	public:
		bool m_rev_4;
		Group1U2()
			:m_rev_4(false)
		{
		}
	private:
		virtual void OnCon()
		{

		}
		virtual void OnRecv(uint32 src_id, const char *custom_pack, uint16 custom_pack_len)
		{
			UNIT_ASSERT(src_id == 4);
			m_rev_4 = true;
		}
	};
	class Group1U3 : public GroupBase, public Singleton<Group1U3>
	{
	public:
		bool m_rev_4;
		Group1U3()
			:m_rev_4(false)
		{
		}
	private:
		virtual void OnCon()
		{

		}
		virtual void OnRecv(uint32 src_id, const char *custom_pack, uint16 custom_pack_len)
		{
			UNIT_ASSERT(src_id == 4);
			m_rev_4 = true;
		}
	};

	class Group0U4 : public GroupBase, public Singleton<Group0U4>
	{
	public:
		bool m_is_done;
		uint32_t m_con_num;
		Group0U4()
		{
			m_is_done = false;
			m_con_num = 0;
		}
	private:
		virtual void OnCon()
		{
			ConUser(1);
			ConUser(2);
			ConUser(3);
		}
		virtual void OnUserCon(uint32 dst_id)
		{
			if (1 == dst_id)
			{
				UNIT_INFO("OnUserCon 1");
			}
			else if (2 == dst_id)
			{
				UNIT_INFO("OnUserCon 2");
			}
			else if (3 == dst_id)
			{
				UNIT_INFO("OnUserCon 3");
			}
			else
			{
				UNIT_ASSERT(false);
			}
			m_con_num++;

			if (m_con_num==3)
			{
				UNIT_INFO("send msg to group 1");
				SendGroup(1, "group");
				m_is_done = true;
			}
		}

	};

	class Group0U5 : public GroupBase, public Singleton<Group0U5>
	{
	public:
		bool m_is_done;
		Group0U5()
		{
			m_is_done = false;
		}
	private:
		virtual void OnCon()
		{
			ConUser(1);
			ConUser(2);
			ConUser(3);
		}
		virtual void OnUserCon(uint32 dst_id)
		{
			if (1 == dst_id)
			{
				UNIT_INFO("OnUserCon 1");
				UNIT_INFO("send msg to group 2");
				SendGroup(2, "none");
				m_is_done = true;
			}
		}

	};

}

UNITTEST(cd)
{
	UNIT_ASSERT(CfgMgr::Obj().Init());
	EventMgr::Obj().Init();

	Group1U1::Obj().Init(CfgMgr::Obj().GetVecAddr(), 1, 1);
	Group1U2::Obj().Init(CfgMgr::Obj().GetVecAddr(), 2, 1);
	Group1U3::Obj().Init(CfgMgr::Obj().GetVecAddr(), 3, 1);
	Group0U4::Obj().Init(CfgMgr::Obj().GetVecAddr(), 4);
	Group0U5::Obj().Init(CfgMgr::Obj().GetVecAddr(), 5);

	lc::Timer tt;
	auto f = [] {
		EventMgr::Obj().StopDispatch();
	};
	tt.StartTimer(2000, f);
	EventMgr::Obj().Dispatch();

	UNIT_ASSERT(Group1U1::Obj().m_rev_4 == true);
	UNIT_ASSERT(Group1U2::Obj().m_rev_4 == true);
	UNIT_ASSERT(Group1U3::Obj().m_rev_4 == true);
	UNIT_ASSERT(Group0U4::Obj().m_is_done == true);
	UNIT_ASSERT(Group0U5::Obj().m_is_done == true);
	UNIT_INFO("-------end---------");
}