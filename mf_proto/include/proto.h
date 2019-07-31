//tcp��: len, tcp_pack.
//	tcp_pack : ctrl_len, ctrl_cmd, ctrl_pack��custom_pack.
//	--ctrl_len��ʾctrl_cmd, ctrl_pack�����ֽ�����
//	--��������Ϣ����û��custom_pack��.
//	--�û��Զ�����Ϣ��������Э���ʽ�Զ��壬����protobuf���������Ķ��С�
//���������
//tcp pack

//mf::MsgData 

//ctrl msg pack. ���Ǿ���� MsgReqReg MsgReqForward ��

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
	};


	//������Ϣ����
	enum MsgCmd : uint16
	{
		CMD_NONE = 0,
		CMD_NTF_COM,               //ͨ����Ӧ��Ϣ�� MsgNtfCom
		CMD_REQ_REG,		       //����ע��	MsgReqReg
		CMD_REQ_CON,			   //��������User	MsgReqCon
		CMD_REQ_FORWARD,           //����ת����User	MsgReqForward
		CMD_NTF_DISCON,			   //֪ͨUser����ʧ�� MsgNtfDiscon
		CMD_REQ_BROADCAST,         //����㲥ָ���� MsgReqBroadcast
	};

	struct MsgNtfCom
	{
		MsgNtfCom()
			:req_cmd(CMD_NONE)
			,is_success(false)
			,tips_len(0)
		{}
		MsgCmd req_cmd; //��������������Ϣ��
		bool is_success;//false ��ʾ����ʧ��
		uint16 tips_len;
		char tips[100]; //��ʾ
	};
	struct MsgReqReg
	{
		uint32 svr_id;
		uint32 group_id;
	};

	struct MsgReqCon
	{
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
		uint32 dst_id;
	};



	//mf::MsgData  ��
	struct MsgDataProto
	{
		//Parse tcp pack
		//@para uint16 tcp_pack_len, ��ʾtcp_pack��Ч����
		//@para MsgData &msg_data, [out]
		static bool Parse(const char *tcp_pack, uint16 tcp_pack_len, MsgData &msg_data);

		//@para[in] const MsgData &msg_data, 
		//@para[out] std::string &tcp_pack
		static bool Serialize(const MsgData &msg_data, std::string &tcp_pack);
		//@para[out] uint16 tcp_pack_len, ��ʾ tcp_pack��Ч�ֽ�����
		//@para[out] char *tcp_pack
		//ע�⣺��Ч������Խ��
		static bool Serialize(const MsgData &msg_data, char *tcp_pack, uint16 tcp_pack_len);

	};
	//ctrl msg pack ��. ���Ǿ���� MsgReqReg MsgReqForward ��
	struct CtrlMsgProto
	{

		//ctrl msg packͨ�ý���
		template<class CtrlMsg>
		static bool Parse(const MsgData &msg_data, CtrlMsg &msg_ctrl)
		{
			return Parse<CtrlMsg>(msg_data.ctrl_pack, msg_data.ctrl_pack_len, msg_ctrl);
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

		template<class CtrlMsg>
		static bool Serialize(uint16 ctrl_cmd, const CtrlMsg &ctrl_msg, const char *custom_pack, uint16 custom_len, std::string &tcp_pack)
		{
			MsgData msg_data;
			std::string ctrl_pack;
			Serialize<CtrlMsg>(ctrl_msg, ctrl_pack);

			msg_data.ctrl_len = sizeof(ctrl_cmd) + ctrl_pack.length();
			msg_data.ctrl_cmd = ctrl_cmd;
			msg_data.ctrl_pack = ctrl_pack.c_str();
			msg_data.ctrl_pack_len = ctrl_pack.length();

			msg_data.custom_len = custom_len;
			msg_data.custom_pack = custom_pack;

			return MsgDataProto::Serialize(msg_data, tcp_pack);
		}
	};

}