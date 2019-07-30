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


//tcp��: len, tcp_pack.
//	tcp_pack : ctrl_len, ctrl_cmd, ctrl_pack��custom_cmd, custom_pack.
//	--ctrl_len��ʾctrl_cmd, ctrl_pack�����ֽ�����
//	--��������Ϣ����û��custom_cmd, custom_pack��.
//	--�û��Զ�����Ϣ��������Э���ʽ�Զ��壬����protobuf���������Ķ��С�


namespace mf {

	//����linux,�Ȳ������ֽڶ��뷽ʽ
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
		uint16 custom_len;  //custom_cmd, custom_pack���ֽ�����
		uint32 custom_cmd;
		uint16 custom_pack_len; //custom_pack �ֽ���
		const char *custom_pack;//����Ϊ�գ���ʾ����Ϣ��

		//@para uint16 tcp_pack_len, ��ʾtcp_pack��Ч����
		bool Parse(const char *tcp_pack, uint16 tcp_pack_len);
		bool Serialize(std::string &out);
	};


	//������Ϣ����
	enum MsgCmd
	{
		MsgNoneCmd = 0,
		CMD_REQ_REG,		//����ע��	MsgReqReg
		CMD_REQ_FORWARD,    //����ת��	MsgReqForward
		CMD_NTF_FORWARD_FAIL,    //֪ͨת��ʧ�� MsgNtfForwardFail

	};


	struct MsgReqReg
	{
		bool Parse(const void* data, uint16 len);
		void Serialize(std::string &out);

		uint32 svr_id;
		uint32 group_id;
	};

	//MsgForwardCmd
	struct MsgReqForward
	{
		uint32 src_id;
		uint32 dst_id;
	};

	struct MsgNtfForwardFail
	{
		uint32 dst_id;
	};

	//ͨ�ý��������л�
	template<class CtrlMsg>
	bool Parse(const MsgData &msg_data, CtrlMsg &msg_ctrl)
	{
		return Parse<CtrlMsg>(msg_data.ctrl_pack, msg_data.ctrl_pack_len, msg_ctrl);
	}

	template<class CtrlMsg>
	bool Parse(const void* data, uint16 len, CtrlMsg &msg)
	{
		if (sizeof(msg) != len)
		{
			return false;
		}
		msg = *((const CtrlMsg *)data);
		return true;
	}

	template<class CtrlMsg>
	void Serialize(const CtrlMsg &msg, std::string &out)
	{
		out.clear();
		out.append((char *)msg, sizeof(msg));
	}
}