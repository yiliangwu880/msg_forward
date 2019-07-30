
#include "proto.h"

bool mf::MsgReqReg::Parse(const void* data, uint16 len)
{
	if (sizeof(this)!=len)
	{
		return false;
	}
	*this = *((MsgReqReg *)data);
	return true;
}

void mf::MsgReqReg::Serialize(std::string &out)
{
	out.clear();
	out.append((char *)this, sizeof(this));
}

//	tcp_pack : ctrl_len, ctrl_cmd, ctrl_pack£¬custom_cmd, custom_pack.
bool mf::MsgData::Parse(const char *tcp_pack, uint16 tcp_pack_len)
{
	if (0 == tcp_pack_len || nullptr == tcp_pack)
	{
		return false;
	}
	const char *cur = tcp_pack; //¶ÁÈ¡Ö¸Õë

	ctrl_len = (decltype(ctrl_len))(*cur);
	cur = cur + sizeof(ctrl_len);
	if (0 == ctrl_len)
	{
		return false;
	}

	ctrl_cmd = (decltype(ctrl_cmd))(*cur);
	cur = cur + sizeof(ctrl_cmd);

	if (ctrl_len > sizeof(ctrl_cmd))
	{
		ctrl_pack_len = ctrl_len - sizeof(ctrl_cmd);
		ctrl_pack = cur;
	}

	//point to custom
	custom_len = tcp_pack_len - ctrl_len;
	if (0 == custom_len)
	{
		return true;
	}
	cur = tcp_pack + sizeof(ctrl_len) + ctrl_len;
	if (tcp_pack_len < ctrl_len)
	{
		return false;
	}

	custom_cmd = (decltype(custom_cmd))(*cur);
	cur = cur + sizeof(custom_cmd);

	if (custom_len > sizeof(custom_cmd))
	{
		custom_pack_len = custom_len - sizeof(custom_cmd);
		custom_pack = cur;
	}
	return true;
}


//	tcp_pack : ctrl_len, ctrl_cmd, ctrl_pack£¬custom_cmd, custom_pack.
bool mf::MsgData::Serialize(std::string &out)
{
	if (0 == ctrl_len )
	{
		return false;
	}
	out.clear();
	out.append((char *)&ctrl_len, sizeof(ctrl_len));
	out.append((char *)&ctrl_cmd, sizeof(ctrl_cmd));
	if (nullptr != ctrl_pack)
	{
		out.append(ctrl_pack, ctrl_pack_len);
	}
	out.append((char *)&custom_cmd, sizeof(custom_cmd));
	if (nullptr != custom_pack)
	{
		out.append(custom_pack, custom_pack_len);
	}
	return true;
}
