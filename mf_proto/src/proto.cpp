#include "proto.h"


bool mf::MsgData::Parse(const char *tcp_pack, uint16 tcp_pack_len)
{
	if (0 == tcp_pack_len || nullptr == tcp_pack)
	{
		return false;
	}
	const char *cur = tcp_pack; //读取指针

	ctrl_len = (decltype(ctrl_len))(*cur);
	cur = cur + sizeof(ctrl_len);

	if (tcp_pack_len < ctrl_len)
	{
		return false;
	}

	if (ctrl_len < sizeof(ctrl_cmd))
	{
		return false;
	}

	ctrl_cmd = (decltype(ctrl_cmd))(*cur);
	cur = cur + sizeof(ctrl_cmd);

	ctrl_pack_len = ctrl_len - sizeof(ctrl_cmd);
	ctrl_pack = cur;

	//point to custom
	custom_len = tcp_pack_len - ctrl_len - sizeof(ctrl_len);
	if (0 == custom_len)
	{
		return true;
	}
	cur = tcp_pack + sizeof(ctrl_len) + ctrl_len;
	custom_pack = cur;
	return true;
}

bool mf::MsgData::Serialize(std::string &tcp_pack) const
{
	if (0 == ctrl_len)
	{
		return false;
	}
	tcp_pack.clear();
	tcp_pack.append((const char *)&ctrl_len, sizeof(ctrl_len));
	tcp_pack.append((const char *)&ctrl_cmd, sizeof(ctrl_cmd));
	if (nullptr != ctrl_pack)
	{
		tcp_pack.append(ctrl_pack, ctrl_pack_len);
	}
	if (nullptr != custom_pack)
	{
		tcp_pack.append(custom_pack, custom_len);
	}
	return true;
}

bool mf::MsgData::Serialize(char *tcp_pack, uint16 tcp_pack_len) const
{
	{//check illegal
		if (0 == ctrl_len || nullptr == tcp_pack || 0 == tcp_pack_len)
		{
			return false;
		}
		uint16 total_size = sizeof(ctrl_len) + ctrl_len + custom_len;
		if (tcp_pack_len < total_size)
		{
			return false;
		}
	}

	char *cur = tcp_pack;
	memcpy(cur, (const char *)&ctrl_len, sizeof(ctrl_len));
	cur += sizeof(ctrl_len);
	memcpy(cur, (const char *)&ctrl_cmd, sizeof(ctrl_cmd));
	cur += sizeof(ctrl_cmd);
	if (nullptr != ctrl_pack)
	{
		memcpy(cur, ctrl_pack, ctrl_pack_len);
		cur += ctrl_pack_len;
	}
	if (nullptr != custom_pack)
	{
		memcpy(cur, custom_pack, custom_len);
		cur += custom_len;
	}
	return true;
}

Cmd req_cmd; //被反馈的请求消息号
bool is_success;//false 表示请求失败

const char *Tips() const { return tips; }
bool Parse(const void* data, uint16 len);
void Serialize(std::string &out);
	private:
		uint16 tips_len;
		char tips[200]; //提示

bool mf::MsgNtfCom::Parse(const void* data, uint16 len)
{
	if (len < sizeof(req_cmd) + sizeof(is_success) + sizeof(tips_len))
	{
		return false;
	}

	const char *cur = data;

	req_cmd = (decltype(req_cmd))(*cur);
	cur = cur + sizeof(req_cmd);
	is_success = (decltype(is_success))(*cur);
	cur = cur + sizeof(is_success);
	tips_len = (decltype(tips_len))(*cur);
	cur = cur + sizeof(tips_len);

	if (len < sizeof(req_cmd) + sizeof(is_success) + sizeof(tips_len) + tips_len)
	{
		return false;
	}
	if (tips_len>sizeof(tips))
	{
		tips_len = 0;
		return false;
	}
	memcpy(tips, cur, tips_len);
	return true;
}

void mf::MsgNtfCom::Serialize(std::string &out) const
{
	out.clear();
	out.append((const char *)&req_cmd, sizeof(req_cmd));
	out.append((const char *)&is_success, sizeof(is_success));
	out.append((const char *)&tips_len, sizeof(tips_len));
	if (tips_len > sizeof(tips))
	{
		return;
	}
	out.append((const char *)tips, tips_len);
}
