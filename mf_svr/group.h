#pragma once
#include <string>
#include <vector>
#include <map>
#include "svr_util/include/su_mgr.h"
#include "../mf_proto/include/proto.h"
#include "server.h"
#include "user.h"

class Group
{
public:
	Group();
	bool AddUser(uint32 id);
	bool DelUser(uint32 id);
	const std::set<uint32> &GetAllUser() { return m_user_s; };

private:
	uint32 m_id;
	std::set<uint32> m_user_s;
};

class GroupMgr : public Singleton<GroupMgr>
{
public:
	Group *GetGroup(uint32 id);
	bool UnregUser(uint32 user_id);
private:
	std::map<uint32, Group> m_id_2_group;
};