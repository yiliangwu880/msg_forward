
#include <string>
#include <vector>

#ifndef uint32
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef long long int64;
typedef unsigned char uint8;
typedef int int32;
#endif


//tcp包: len, tcp_pack.
//	--ctrl_len表示ctrl_cmd, ctrl_pack的总字节数。
//	--纯控制消息，就没有custom_cmd, custom_pack了.
//	tcp_pack : ctrl_len, ctrl_cmd, ctrl_pack，custom_cmd, custom_pack.
//	--用户自定义消息包，具体协议格式自定义，可以protobuf或者其他的都行。
//	custom_pack :


#pragma pack(push)
#pragma pack(1)
struct MfPack
{
	const static uint16 MAX_MF_DATA_LEN = 1024 * 4; //4k
	MfPack() :len(0)
	{}
	uint16 len; //tcp_pack有效字节数
	char tcp_pack[MAX_MF_DATA_LEN];
};
#pragma pack(pop)