
#include <string>
#include <vector>
#include <map>
#include "svr_util/include/su_mgr.h"
#include "../mf_proto/include/proto.h"

class ConnectClient;
class User
{
public:
	User(uint32 id, uint64 con_id);

	void DispatchMsg(ConnectClient &con, const mf::MsgData &msg);
	ConnectClient *GetConnect();
	//bool Send();
private:
	uint32 m_id;
	uint64 m_con_id; //ConnectClient id
};


typedef void (*HandleMsg)(User &user, ConnectClient &con, const mf::MsgData &msg);
class UserMgr : public Singleton<UserMgr>
{
public:
	void Init();

	bool RegUser(ConnectClient &con, const mf::MsgData &msg);
	bool UnregUser(uint32 id);
	User *GetUser(uint32 id);

	void DispatchMsg(ConnectClient &con, const mf::MsgData &msg);

private:
	std::map<uint32, User> m_id_2_user;
	std::map<uint16, HandleMsg> m_cmd_2_handle;
};