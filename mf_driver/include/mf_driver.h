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
	class MfClientMgr;
	struct MfAddr 
	{
		std::string ip;
		uint16 port;
	};
	class UserClient : public lc::ClientCon
	{
		
	public:
		UserClient(MfClientMgr &mgr);
		virtual void OnRecv(const lc::MsgPack &msg) override;
		virtual void OnConnected() override;
		virtual void OnDisconnected() override;
	public:
		template<class CtrlMsg>
		bool SendCtrlMsg(Cmd cmd, const CtrlMsg &msg)
		{
			string tcp_pack;
			if (!MsgData::Serialize(cmd, msg, nullptr, 0, tcp_pack))
			{
				return false;
			}
			return SendPack(tcp_pack);
		}

		bool SendPack(const string &tcp_pack);

	private:
		void ParseNtf(const MsgNtfCom &ntf);

	private:
		MfClientMgr &m_mgr;
	};

	class MfClientMgr
	{
		friend class UserClient
	public:
		MfClientMgr() 
			:m_lb_idx(0)
			, m_svr_id(0)
			, m_grp_id(0)
		{
		}
		~MfClientMgr();

		//connect mf svr list. connect
		//@para vec_svr_addr, ��������ַ�б�
		//@para svr_id �ҵķ�����id
		//@para vec_groupId �ҵĵ���id�б�
		bool Init(const std::vector<MfAddr> &vec_mf_addr, uint32 svr_id, uint32 group_id = 0);

		//@para const char *custom_pack, Ϊuser��user��֮��ͨѶ���Զ���Э��
		bool Send(uint32 dst_id, const char *custom_pack, uint16 custom_pack_len);

		//�㲥��ָ����. ��group_id==0ʱ����ʾ�㲥ȫ��user��
		bool SendGroup(uint32 group_id, const char *custom_pack, uint16 custom_pack_len);

		//����Ŀ��user. ͨ�� MfClientMgr::OnUserCon �� MfClientMgr::OnUserDiscon �����������
		void ConUser(uint32 dst_id); 

		//һ�㶨ʱ���ã��������ӶϿ���mf svr.
		//����5�����ϵ���һ�Ρ�
		void TryReconMf();

		uint32 GetSvrId() const { return m_svr_id; }
		uint32 GetGrpId() const { return m_grp_id; }

	private:
		void OnOneMfDiscon(); //����һ��mf����ʧ��
		UserClient* BlSelectSvr(); //���ؾ���һ̨mf svr

	private:
		//��������mf svr list ���������������һ̨����ɹ���
		virtual void OnCon() = 0;
		//ȫ�����Ӷ�ʧ�ܾͷ�����
		virtual void OnDiscon() = 0;

		virtual void OnUserCon(uint32 dst_id) = 0;
		virtual void OnUserDiscon(uint32 dst_id) = 0;//���ӶԷ�ʧ�ܣ����߶Է��������ߣ�������á�
		//@para src_id ���ͷ�������id
		virtual void OnRecv(uint32 src_id, const char *custom_pack, uint16 custom_pack_len)=0;

	private:
		//ClientCon list
		std::vector<UserClient*> m_vec_con; //�������mf�Ŀͻ���
		uint32 m_lb_idx; //load blance��ѯ��
		uint32 m_svr_id; //�ҵķ�����id
		uint32 m_grp_id;
	};

}