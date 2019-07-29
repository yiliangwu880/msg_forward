
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


//tcp��: len, tcp_pack.
//	--ctrl_len��ʾctrl_cmd, ctrl_pack�����ֽ�����
//	--��������Ϣ����û��custom_cmd, custom_pack��.
//	tcp_pack : ctrl_len, ctrl_cmd, ctrl_pack��custom_cmd, custom_pack.
//	--�û��Զ�����Ϣ��������Э���ʽ�Զ��壬����protobuf���������Ķ��С�
//	custom_pack :


#pragma pack(push)
#pragma pack(1)
struct MfPack
{
	const static uint16 MAX_MF_DATA_LEN = 1024 * 4; //4k
	MfPack() :len(0)
	{}
	uint16 len; //tcp_pack��Ч�ֽ���
	char tcp_pack[MAX_MF_DATA_LEN];
};
#pragma pack(pop)