
#include <string>
#include <vector>
#include "protocol.h"
#include "log_def.h"
#include "libevent_cpp/include/include_all.h"

class ConnectClient : public lc::SvrCon
{
private:
	virtual void OnRecv(const lc::MsgPack &msg) override
	{

	}
	virtual void OnConnected() override
	{
		//MsgPack msg;
		//...≥ı ºªØmsg
		//	send_data(msg);
	}

};