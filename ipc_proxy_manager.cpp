#include "ipc_proxy_manager.h"

namespace OHOS {

std::map< unsigned long long, sptr< IPCObjectProxy > > IPCProxyManager::proxyMap_;
std::mutex IPCProxyManager::mutex_;

sptr< IPCObjectProxy > IPCProxyManager::FindOrNewProxy(unsigned long long handle)
{
	std::lock_guard< std::mutex > lock(mutex_);
	auto it = proxyMap_.find(handle);
	if (it == proxyMap_.end()) {
		sptr< IPCObjectProxy > proxy(new IPCObjectProxy(handle));
		proxyMap_.emplace(handle, proxy);
		IPC_LOG("INSERT PROXY with handle=%llx\n", handle);
		return proxy;
	}
	return it->second;
}

void IPCProxyManager::CleanProxy(unsigned long long handle)
{
	std::lock_guard< std::mutex > lock(mutex_);
	auto it = proxyMap_.find(handle);
	if (it != proxyMap_.end()) {
		it->second->SendObituary();
		proxyMap_.erase(it);
	}
}

}; // namespace OHOS