#ifndef _IPC_BASE_H_
#define _IPC_BASE_H_

#include <cstdio>
#include <cstring>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <atomic>
#include <cerrno>

#define IPC_LOG(fmt, args...) \
	printf("[IPC LOG %s:%u]" fmt, __FILE__, __LINE__, ##args)
#define IPC_DEBUG(fmt, args...) \
//	printf("[IPC DEBUG %s:%u]" rmt, __FILE__, __LINE__, ##args)

extern key_t g_client_server_shmKey;
extern key_t g_device_auth_shmKey;

const int IPC_SHM_FLAG = IPC_CREAT | 0666;

const size_t DATA_SIZE = 0x20000;

const int32_t GET_SA_REQUEST_CODE = 2;
const int32_t WIFI_DEVICE_ABILITY_ID = 1125;
const int32_t WIFI_P2P_ABILITY_ID = 1128;

struct IpcShmData {
	size_t inputSz;
	size_t outputSz;
	char inputData[DATA_SIZE];
	char outputData[DATA_SIZE];
	volatile bool needReply;
	uint32_t requestCode;
	volatile bool containFd;
	volatile bool containHandle;
	unsigned long long handle;
	std::atomic< bool > isProcessing;
	bool deadNotice;
};

static inline IpcShmData *OpenShmCommon(key_t shmKey, int flag)
{
	int shmFd = shmget(shmKey, sizeof(IpcShmData), flag);
	if (shmFd < 0) {
		IPC_LOG("Get shm failed, errno=%d\n", errno);
		return nullptr;
	}
	void *shmPtr = shmat(shmFd, 0, 0);
	if (shmPtr == (void *)-1) {
		IPC_LOG("Map shm failed\n");
		return nullptr;
	}
	return (IpcShmData *)shmPtr;
}

static inline IpcShmData *OpenShm(key_t shmKey)
{
	return OpenShmCommon(shmKey, IPC_SHM_FLAG);
}

static inline IpcShmData *OpenShmExcl(key_t shmKey)
{
	return OpenShmCommon(shmKey, IPC_SHM_FLAG | IPC_EXCL);
}

static inline key_t HandleToKey(unsigned long long handle)
{
	key_t key = handle >> 32;
	key ^= handle & 0xFFFFFFFF;
	return key;
}

static inline void HandleToAddr(char *addr, unsigned long long handle)
{
	sprintf(addr, "/tmp/%llx.ipc.socket", handle);
}

#endif // _IPC_BASE_H_
