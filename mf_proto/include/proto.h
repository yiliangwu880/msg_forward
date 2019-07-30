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


//tcp包: len, tcp_pack.
//	tcp_pack : ctrl_len, ctrl_cmd, ctrl_pack，custom_cmd, custom_pack.
//	--ctrl_len表示ctrl_cmd, ctrl_pack的总字节数。
//	--纯控制消息，就没有custom_cmd, custom_pack了.
//	--用户自定义消息包，具体协议格式自定义，可以protobuf或者其他的都行。


namespace mf {

	//都用linux,先不限制字节对齐方式
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
		uint16 custom_len;  //custom_cmd, custom_pack总字节数。
		uint32 custom_cmd;
		uint16 custom_pack_len; //custom_pack 字节数
		const char *custom_pack;//可以为空，表示无消息体

		//@para uint16 tcp_pack_len, 表示tcp_pack有效长度
		bool Parse(const char *tcp_pack, uint16 tcp_pack_len);
		bool Serialize(std::string &out);
	};


	//控制消息定义
	enum MsgCmd
	{
		MsgNoneCmd = 0,
		CMD_REQ_REG,		//请求注册	MsgReqReg
		CMD_REQ_FORWARD,    //请求转发	MsgReqForward
		CMD_NTF_FORWARD_FAIL,    //通知转发失败 MsgNtfForwardFail

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

	//通用解析，序列化
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