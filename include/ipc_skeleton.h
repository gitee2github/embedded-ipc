#ifndef _IPC_SKELETON_H_
#define _IPC_SKELETON_H_

#include <sys/types.h>
#include <unistd.h>

#include "parcel.h"
#include "refbase.h"

namespace OHOS {

class IRemoteObject;

class IPCSkeleton {
public:
	IPCSkeleton() = default;
	~IPCSkeleton() = default;
	static pid_t GetCallingPid();
	static uid_t GetCallingUid();
	static sptr< IRemoteObject > GetContextObject();
	static bool SetContextObject(sptr< IRemoteObject > &object);
	static int SocketListening(const char *addr);
	static int SocketReadFd(int socketFd);
	static bool SocketWriteFd(const char *addr, int fd);

private:
	static sptr< IRemoteObject > obj_;
};

} // namespace OHOS

#endif // _IPC_SKELETON_H_
