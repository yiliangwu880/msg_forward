/*
//����libevent_cpp��
��Ҫ����д���ܹ�����

main()
{
	EventMgr::Obj().Init();

		���ñ����api


	EventMgr::Obj().Dispatch();
}

//��Ҳ���Ը���Э�飬�Լ�д�Զ���ͻ��ˣ��Ͳ�����libevent_cpp����.
*/

#pragma once

#include <string>
#include <string.h>
#include <vector>
#include "libevent_cpp/include/include_all.h"
#include "svr_util/include/singleton.h"
#include "svr_util/include/easy_code.h"
#include "svr_util/include/typedef.h"

namespace mf {
	struct MfAddr 
	{
		std::string ip;
		uint16 port;
	};
	class UserClient : public lc::ClientCon
	{
	public:
		virtual void OnRecv(const lc::MsgPack &msg) override;
		virtual void OnConnected() override;
		virtual void OnDisconnected() override;
	};

	class BaseClientMgr
	{
	public:
		BaseClientMgr() 
			:m_lb_idx(0)
			, m_user_id(0)
		{
		}
		~BaseClientMgr();
		//connect mf svr list. connect
		bool Init(const std::vector<MfAddr> &vec_svr_addr, uint32 user_id);

		bool Send(uint32 dst_id, const char *pack, uint16 pack_len);
		//�㲥��ָ����. ��group_id==0ʱ����ʾ�㲥ȫ��user��
		void SendGeroup(uint32 group_id, const char *pack, uint16 pack_len);

		void ConnectUser(uint32 dst_id); //����Ŀ��user. ͨ�� OnUserConnected �� OnUserDisconnected �����������
		
		//һ�㶨ʱ���ã��������ӶϿ���mf svr.
		void TryReconnectSvr();
	private:
		//��������mf svr list ���������������һ̨����ɹ���
		virtual void OnConnected() = 0;
		//ȫ�����Ӷ�ʧ�ܾͷ�����
		virtual void OnDisconnected() = 0;

		virtual void OnUserConnected(uint32 dst_id) = 0;
		virtual void OnUserDisconnected(uint32 dst_id) = 0;//�Է�û���ϣ����߶Է��������ߣ�������á�
		virtual void OnRecv(uint32 src_id, const char *pack, uint16 pack_len)=0;

	private:
		UserClient* BlSelectSvr(); //���ؾ���һ̨mf svr
	private:
		//ClientCon list
		std::vector<UserClient*> m_vec_con;
		uint32 m_lb_idx; //load blance��ѯ��
		uint32 m_user_id; 
	};

	class IClientParse
	{
	public:
		bool Parse(const char *tcp_pack, uint16 tcp_pack_len);
		virtual void Forward() = 0;

		void SerializeCmd();
	};
}