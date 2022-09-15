#ifndef _IPC_PROXY_MANAGER_H_
#define _IPC_PROXY_MANAGER_H_

#include <map>
#include <mutex>
#include "ipc_object_proxy.h"

namespace OHOS {
class IPCProxyManager {
public:
	static sptr< IPCObjectProxy > FindOrNewProxy(unsigned long long handle);
	static void CleanProxy(unsigned long long handle);
private:
	static std::map< unsigned long long, sptr< IPCObjectProxy > > proxyMap_;
	static std::mutex mutex_;
};

};

#endif // _IPC_PROXY_MANAGER_H_