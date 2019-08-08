/*
tcp��: len, tcp_pack.
tcp_pack:ctrl_len,ctrl_cmd, ctrl_pack��custom_pack.
--ctrl_len��ʾctrl_cmd,ctrl_pack�����ֽ�����
--��������Ϣ����û��custom_pack��.
--custom_pack �û��Զ�����Ϣ��������Э���ʽ�Զ��壬���������protobuf��
�ֶ������

2�� user��user�㣺custom_pack	--user ֮��ͨѶ���Զ���Э��

1�� user��mf�㣺 tcp_pack  --user mfͨѶЭ�飬
ctrl_cmd, ctrl_pack ������ mf::MsgData

�ֲ�ͼ��
	user		  mf				user
user��user��	-------------------	user��user��
user��mf��	--- user��mf��	--	user��mf��
*/

#pragma once

#include <string>
#include <string.h>

#ifndef uint32
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef long long int64;
typedef unsigned char uint8;
typedef int int32;
#endif


namespace mf {

	//����linux c++,�Ȳ������ֽڶ��뷽ʽ
	//#pragma pack(push)
	//#pragma pack(1)

	//#pragma pack(pop)

	struct MsgData
	{
		MsgData()
		{
			memset(this, 0, sizeof(MsgData));
		}
		uint16 ctrl_len; //ctrl_len��ʾctrl_cmd, ctrl_pack�����ֽ�����
		uint16 ctrl_cmd;
		uint16 ctrl_pack_len; //ctrl_pack �ֽ���
		const char *ctrl_pack; //����Ϊ�գ���ʾ����Ϣ��
		//�û��Զ�����Ϣ��
		uint16 custom_len;  //custom_pack�ֽ�����
		const char *custom_pack;//����Ϊ�գ���ʾ����Ϣ��

	public:
		//Parse tcp pack
		//@para uint16 tcp_pack_len, ��ʾtcp_pack��Ч����
		//@para MsgData &msg_data, [out]
		bool Parse(const char *tcp_pack, uint16 tcp_pack_len);

		//@para[in] const MsgData &msg_data, 
		//@para[out] std::string &tcp_pack
		bool Serialize(std::string &tcp_pack) const;
		//@para[out] uint16 tcp_pack_len, ��ʾ tcp_pack��Ч�ֽ�����
		//@para[out] char *tcp_pack
		//ע�⣺��Ч������Խ��
		bool Serialize(char *tcp_pack, uint16 tcp_pack_len) const;

		//һ�������tcp_pack
		template<class CtrlMsg>
		static bool Serialize(uint16 ctrl_cmd, const CtrlMsg &ctrl_msg, const char *custom_pack, uint16 custom_len, std::string &tcp_pack)
		{
			MsgData msg_data;
			std::string ctrl_pack;
			Serialize(ctrl_msg, ctrl_pack);

			msg_data.ctrl_len = sizeof(ctrl_cmd) + ctrl_pack.length();
			msg_data.ctrl_cmd = ctrl_cmd;
			msg_data.ctrl_pack = ctrl_pack.c_str();
			msg_data.ctrl_pack_len = ctrl_pack.length();

			msg_data.custom_len = custom_len;
			msg_data.custom_pack = custom_pack;

			return msg_data.Serialize(tcp_pack);
		}
	};


	//������Ϣ����
	enum Cmd : uint16
	{
		CMD_NONE = 0,
		CMD_NTF_COM,               //ͨ����Ӧ��Ϣ�� MsgNtfCom

		CMD_REQ_REG,		       //����ע�� 	MsgReqReg, mfע��ʧ�ܣ���Ͽ��ͻ�������
		CMD_RSP_REG,				//MsgNouse

		CMD_REQ_CON,			   //��������User MsgReqCon,
		CMD_RSP_CON,			   //MsgRspCon

		CMD_REQ_FORWARD,           //����ת����User	MsgReqForward

		CMD_NTF_DISCON,			   //֪ͨUser����ʧ�� MsgNtfDiscon. CMD_REQ_FORWARD����ʧ��Ҳ�ᵼ�������Ӧ��

		CMD_REQ_BROADCAST,         //����㲥ָ���� MsgReqBroadcast, mfԭ�����͵�����user
	};

	struct MsgNtfCom
	{
		MsgNtfCom()
			:req_cmd(CMD_NONE)
			,is_success(false)
			,tips_len(0)
		{}
		Cmd req_cmd; //��������������Ϣ��
		bool is_success;//false ��ʾ����ʧ��

		const char *Tips() const { return tips; }
		bool Parse(const void* data, uint16 len);
		void Serialize(std::string &out) const;
	private:
		uint16 tips_len;
		char tips[200]; //��ʾ
	};
	struct MsgReqReg
	{
		uint32 svr_id;
		uint32 group_id;
	};

	struct MsgNouse
	{
		char n;
	};

	struct MsgReqCon
	{
		uint32 dst_id; //Ŀ�������id
	};
	struct MsgRspCon
	{
		bool is_ok;
		uint32 dst_id; //Ŀ�������id
	};

	struct MsgReqBroadcast
	{
		uint32 src_id;
		uint32 group_id; //0��ʾ�㲥ȫ��
	};

	struct MsgReqForward
	{
		uint32 src_id;
		uint32 dst_id;
	};
	struct MsgNtfDiscon
	{
		uint32 svr_id;
	};


	//user��mf��. ctrl_pack�ı�����롣 ����MsgReqReg MsgReqForward ��
	struct CtrlMsgProto
	{

		//ctrl msg packͨ�ý���
		template<class CtrlMsg>
		static bool Parse(const MsgData &msg_data, CtrlMsg &msg_ctrl)
		{
			return Parse(msg_data.ctrl_pack, msg_data.ctrl_pack_len, msg_ctrl);
		}
		template<class CtrlMsg>
		static bool Parse(const void* data, uint16 len, CtrlMsg &msg)
		{
			if (sizeof(msg) != len)
			{
				return false;
			}
			msg = *((const CtrlMsg *)data);
			return true;
		}

		template<class CtrlMsg>
		static void Serialize(const CtrlMsg &msg, std::string &out)
		{
			out.clear();
			out.append((char *)&msg, sizeof(msg));
		}

		//MsgNtfCom ����
		template<>
		static bool Parse(const void* data, uint16 len, MsgNtfCom &msg)
		{
			return msg.Parse(data, len);
		}
		template<>
		static void Serialize(const MsgNtfCom &msg, std::string &out)
		{
			msg.Serialize(out);
		}

	};

}