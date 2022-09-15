#ifndef _IPC_SOCKET_MANAGER_H_
#define _IPC_SOCKET_MANAGER_H_

#include <map>
#include <mutex>

namespace OHOS {

class IPCSocketManager {
public:
	static void InsertSocketFd(unsigned long long handle, int socketFd);
	static int FindSocketFd(unsigned long long handle);
private:
	static std::map< unsigned long long, int > socketFdMap_;
	static std::mutex mutex_;
};

} // namespace OHOS

#endif // _IPC_SOCKET_MANAGER_H_