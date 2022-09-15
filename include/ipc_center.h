#ifndef _IPC_CENTER_H_
#define _IPC_CENTER_H_

#include "iremote_object.h"
#include "ipc_object_stub.h"

namespace OHOS {

class IpcCenter {
public:
	IpcCenter();
	~IpcCenter();
	static bool ShmInit(key_t ShmKey);
	bool Init(bool isServer, IPCObjectStub *stub);
	bool ThreadCreate(IPCObjectStub *stub);
	static void ProcessHandle(key_t ShmKey, IPCObjectStub *ipcStub);
	static size_t threadNum_;
private:
	key_t receive_shm_key_;
};

} // namespace OHOS

#endif //_IPC_CENTER_H_
