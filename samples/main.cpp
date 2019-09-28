
#include "../mf_driver/include/mf_driver.h"

using namespace std;
using namespace lc;
using namespace mf;


class MfClient : public MfClientMgr
{
private:
	//反馈连接mf svr list 情况。能连接任意一台都算成功。
	virtual void OnCon()
	{
		//这里链接成功，可以做收发消息了。
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

int main(int argc, char* argv[])
{
	EventMgr::Obj().Init();
	MfClient c;
	std::vector < MfAddr > vec_addr = { {"127.0.0.1", 5551} };
	c.Init(vec_addr, 1); //注册我的服务器ID 为1


	EventMgr::Obj().Dispatch();
	return 0;
}

