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

Group * GroupMgr::GetGroup(uint32 id)
{
	if (0 == id)
	{
		return nullptr;
	}

	Group &g = m_id_2_group[id];
	return &g;
}
