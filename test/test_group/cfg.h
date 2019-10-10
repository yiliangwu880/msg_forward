#pragma once
#include <string>
#include "libevent_cpp/include/include_all.h"
#include "svr_util/include/su_mgr.h"
#include "svr_util/include/single_progress.h"
#include "svr_util/include/read_cfg.h"
#include "unit_test.h"
#include "../mf_driver/include/mf_driver.h"

using namespace su;
using namespace mf;
using namespace lc;



class CfgMgr : public Singleton<CfgMgr>
{
public:
	bool Init()
	{
		UNIT_INFO("init cfg");
		su::Config cfg;
		UNIT_ASSERT(cfg.init("../combine_svr/mf_svr_cfg.txt"));

		uint16 port = (uint16)cfg.GetInt("port");
		UNIT_INFO("port=%d", port);
		m_vec_addr.push_back({ "127.0.0.1", port });
		return true;
	}
	const std::vector < MfAddr > &GetVecAddr() const { return m_vec_addr; };
private:
	std::vector < MfAddr > m_vec_addr;
};
namespace mm
{
	struct MfAddr
	{
		std::string ip;
	};
}