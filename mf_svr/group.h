
#include <string>
#include <vector>
#include <map>
#include "svr_util/include/su_mgr.h"
#include "../mf_proto/include/proto.h"

class Group
{
public:
	Group();
	bool AddUser(uint32 id);
	bool DelUser(uint32 id);

private:
	uint32 m_id;
	std::set<uint32> m_user_s;
};

class GroupMgr : public Singleton<GroupMgr>
{
public:
	Group *GetGroup(uint32 id);

private:
	std::map<uint32, Group> m_id_2_group;
};