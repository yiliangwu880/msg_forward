
#include "../mf_driver/include/mf_driver.h"

using namespace std;
using namespace lc;
using namespace mf;


class MfClient : public MfClientMgr
{
private:
	//��������mf svr list ���������������һ̨����ɹ���
	virtual void OnCon()
	{
		//�������ӳɹ����������շ���Ϣ�ˡ�
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
	}

};

int main(int argc, char* argv[])
{
	EventMgr::Obj().Init();
	MfClient c;
	std::vector < MfAddr > vec_addr = { {"127.0.0.1", 5551} };
	c.Init(vec_addr, 1); //ע���ҵķ�����ID Ϊ1


	EventMgr::Obj().Dispatch();
	return 0;
}

