#pragma once
#include <string>
#include "libevent_cpp/include/include_all.h"
#include "svr_util/include/su_mgr.h"
#include "svr_util/include/single_progress.h"
#include "svr_util/include/read_cfg.h"
#include "../mf_driver/include/mf_driver.h"

using namespace su;
using namespace mf;
using namespace lc;

namespace
{
	static uint32 ECHO_SVR_ID = 55;
}

//����cd
class EchoClient : public MfClientMgr, public Singleton<EchoClient>
{
public:

public:

	bool Send(uint32 dst_id, const std::string &s)
	{
		return MfClientMgr::Send(dst_id, s.c_str(), s.length());
	}

private:
	//��������mf svr list ���������������һ̨����ɹ���
	virtual void OnCon()
	{
	}

	//ȫ�����Ӷ�ʧ�ܾͷ�����
	virtual void OnDiscon()
	{
	}

	virtual void OnUserCon(uint32 dst_id)
	{
	}

	//���ӶԷ�ʧ�ܣ����߶Է��������ߣ�������á�
	virtual void OnUserDiscon(uint32 dst_id)
	{
	}

	//@para src_id ���ͷ�������id
	virtual void OnRecv(uint32 src_id, const char *custom_pack, uint16 custom_pack_len)
	{
		std::string s(custom_pack, custom_pack_len);
		Send(src_id, s);
		LB_DEBUG("echo msg %s", s.c_str());
	}

};