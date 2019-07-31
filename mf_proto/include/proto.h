//tcp包: len, tcp_pack.
//	tcp_pack : ctrl_len, ctrl_cmd, ctrl_pack，custom_pack.
//	--ctrl_len表示ctrl_cmd, ctrl_pack的总字节数。
//	--纯控制消息，就没有custom_pack了.
//	--用户自定义消息包，具体协议格式自定义，可以protobuf或者其他的都行。
//分三层解析
//tcp pack

//mf::MsgData 

//ctrl msg pack. 就是具体的 MsgReqReg MsgReqForward 等

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

	//都用linux c++,先不限制字节对齐方式
	//#pragma pack(push)
	//#pragma pack(1)

	//#pragma pack(pop)

	struct MsgData
	{
		MsgData()
		{
			memset(this, 0, sizeof(MsgData));
		}
		uint16 ctrl_len; //ctrl_len表示ctrl_cmd, ctrl_pack的总字节数。
		uint16 ctrl_cmd;
		uint16 ctrl_pack_len; //ctrl_pack 字节数
		const char *ctrl_pack; //可以为空，表示无消息体
		//用户自定义消息包
		uint16 custom_len;  //custom_pack字节数。
		const char *custom_pack;//可以为空，表示无消息体
	};


	//控制消息定义
	enum MsgCmd : uint16
	{
		CMD_NONE = 0,
		CMD_NTF_COM,               //通用响应消息， MsgNtfCom
		CMD_REQ_REG,		       //请求注册	MsgReqReg
		CMD_REQ_CON,			   //请求连接User	MsgReqCon
		CMD_REQ_FORWARD,           //请求转发给User	MsgReqForward
		CMD_NTF_DISCON,			   //通知User连接失败 MsgNtfDiscon
		CMD_REQ_BROADCAST,         //请求广播指定组 MsgReqBroadcast
	};

	struct MsgNtfCom
	{
		MsgNtfCom()
			:req_cmd(CMD_NONE)
			,is_success(false)
			,tips_len(0)
		{}
		MsgCmd req_cmd; //被反馈的请求消息号
		bool is_success;//false 表示请求失败
		uint16 tips_len;
		char tips[100]; //提示
	};
	struct MsgReqReg
	{
		uint32 svr_id;
		uint32 group_id;
	};

	struct MsgReqCon
	{
		uint32 dst_id; //目标服务器id
	};

	struct MsgReqBroadcast
	{
		uint32 src_id;
		uint32 group_id; //0表示广播全部
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



	//mf::MsgData  层
	struct MsgDataProto
	{
		//Parse tcp pack
		//@para uint16 tcp_pack_len, 表示tcp_pack有效长度
		//@para MsgData &msg_data, [out]
		static bool Parse(const char *tcp_pack, uint16 tcp_pack_len, MsgData &msg_data);

		//@para[in] const MsgData &msg_data, 
		//@para[out] std::string &tcp_pack
		static bool Serialize(const MsgData &msg_data, std::string &tcp_pack);
		//@para[out] uint16 tcp_pack_len, 表示 tcp_pack有效字节数。
		//@para[out] char *tcp_pack
		//注意：高效，容易越界
		static bool Serialize(const MsgData &msg_data, char *tcp_pack, uint16 tcp_pack_len);

	};
	//ctrl msg pack 层. 就是具体的 MsgReqReg MsgReqForward 等
	struct CtrlMsgProto
	{

		//ctrl msg pack通用解析
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