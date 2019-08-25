#include "group.h"

Group::Group()
	:m_id(0)
{

}

bool Group::AddUser(uint32 id)
{
	bool r = m_user_s.insert(id).second;
	return r;
}

bool Group::DelUser(uint32 id)
{
	int cnt = m_user_s.erase(id);
	return cnt == 1;
}

Group * GroupMgr::GetGroup(uint32 grp_id)
{
	if (0 == grp_id)
	{
		return nullptr;
	}

	Group &g = m_id_2_group[grp_id];//没有直接创建新的
	return &g;
}

bool GroupMgr::UnregUser(uint32 user_id)
{
	L_COND_F(user_id);
	uint32 group_id = 0;
	if (User *user = UserMgr::Obj().GetUser(user_id))
	{
		group_id = user->GetGroupId();
	}
	else
	{
		L_ERROR("find user fail. user_id=%d", user_id);
	}
	if (0 == group_id)
	{
		return true;
	}

	Group *group = GetGroup(group_id);
	L_COND_F(group);
	bool ret = group->DelUser(user_id);
	if (!ret)
	{
		L_ERROR("group del user fail. user_id, group_id=%d %d", user_id, group_id);
	}
	return true;

}
