
#include "proto.h"




bool mf::MsgDataProto::Parse(const char *tcp_pack, uint16 tcp_pack_len, MsgData &msg_data)
{
	if (0 == tcp_pack_len || nullptr == tcp_pack)
	{
		return false;
	}
	const char *cur = tcp_pack; //∂¡»°÷∏’Î

	msg_data.ctrl_len = (decltype(msg_data.ctrl_len))(*cur);
	cur = cur + sizeof(msg_data.ctrl_len);

	if (tcp_pack_len < msg_data.ctrl_len)
	{
		return false;
	}

	if (msg_data.ctrl_len < sizeof(msg_data.ctrl_cmd))
	{
		return false;
	}

	msg_data.ctrl_cmd = (decltype(msg_data.ctrl_cmd))(*cur);
	cur = cur + sizeof(msg_data.ctrl_cmd);

	msg_data.ctrl_pack_len = msg_data.ctrl_len - sizeof(msg_data.ctrl_cmd);
	msg_data.ctrl_pack = cur;

	//point to custom
	msg_data.custom_len = tcp_pack_len - msg_data.ctrl_len - sizeof(msg_data.ctrl_len);
	if (0 == msg_data.custom_len)
	{
		return true;
	}
	cur = tcp_pack + sizeof(msg_data.ctrl_len) + msg_data.ctrl_len;
	msg_data.custom_pack = cur;
	return true;
}

bool mf::MsgDataProto::Serialize(const MsgData &msg_data, std::string &tcp_pack)
{
	if (0 == msg_data.ctrl_len)
	{
		return false;
	}
	tcp_pack.clear();
	tcp_pack.append((char *)&msg_data.ctrl_len, sizeof(msg_data.ctrl_len));
	tcp_pack.append((char *)&msg_data.ctrl_cmd, sizeof(msg_data.ctrl_cmd));
	if (nullptr != msg_data.ctrl_pack)
	{
		tcp_pack.append(msg_data.ctrl_pack, msg_data.ctrl_pack_len);
	}
	if (nullptr != msg_data.custom_pack)
	{
		tcp_pack.append(msg_data.custom_pack, msg_data.custom_len);
	}
	return true;
}

bool mf::MsgDataProto::Serialize(const MsgData &msg_data, char *tcp_pack, uint16 tcp_pack_len)
{
	{//check illegal
		if (0 == msg_data.ctrl_len || nullptr == tcp_pack || 0 == tcp_pack_len)
		{
			return false;
		}
		uint16 total_size = sizeof(msg_data.ctrl_len) + msg_data.ctrl_len + msg_data.custom_len;
		if (tcp_pack_len < total_size)
		{
			return false;
		}
	}

	char *cur = tcp_pack;
	memcpy(cur, (char *)&msg_data.ctrl_len, sizeof(msg_data.ctrl_len));
	cur += sizeof(msg_data.ctrl_len);
	memcpy(cur, (char *)&msg_data.ctrl_cmd, sizeof(msg_data.ctrl_cmd));
	cur += sizeof(msg_data.ctrl_cmd);
	if (nullptr != msg_data.ctrl_pack)
	{
		memcpy(cur, msg_data.ctrl_pack, msg_data.ctrl_pack_len);
		cur += msg_data.ctrl_pack_len;
	}
	if (nullptr != msg_data.custom_pack)
	{
		memcpy(cur, msg_data.custom_pack, msg_data.custom_len);
		cur += msg_data.custom_len;
	}
	return true;
}
