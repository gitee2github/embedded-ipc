#include "ipc_socket_manager.h"

namespace OHOS {

std::map< unsigned long long, int > IPCSocketManager::socketFdMap_;
std::mutex IPCSocketManager::mutex_;

void IPCSocketManager::InsertSocketFd(unsigned long long handle, int socketFd)
{
	std::lock_guard< std::mutex > lock(mutex_);
	socketFdMap_.emplace(handle, socketFd);
}

int IPCSocketManager::FindSocketFd(unsigned long long handle)
{
	std::lock_guard< std::mutex > lock(mutex_);
	auto it = socketFdMap_.find(handle);
	if (it == socketFdMap_.end()) {
		return -1;
	}
	return it->second;
}

} // namespace OHOS