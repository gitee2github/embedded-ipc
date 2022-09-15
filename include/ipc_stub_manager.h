#ifndef _IPC_STUB_MANAGER_H_
#define _IPC_STUB_MANAGER_H_

#include <map>
#include <mutex>
#include "ipc_object_stub.h"

namespace OHOS {
class IPCStubManager {
public:
	static unsigned long long GetHandleNum();
	static void InsertStub(unsigned long long handle, sptr< IPCObjectStub > stub);
private:
	static std::map< unsigned long long, sptr< IPCObjectStub > > stubMap_;
	static std::mutex mutex_;
};

};

#endif // _IPC_STUB_MANAGER_H_