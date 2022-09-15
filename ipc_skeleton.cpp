#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#include "ipc_base.h"
#include "iremote_object.h"
#include "ipc_skeleton.h"

namespace OHOS {

sptr< IRemoteObject > IPCSkeleton::obj_ = nullptr;

pid_t IPCSkeleton::GetCallingPid()
{
	return getpid();
}

uid_t IPCSkeleton::GetCallingUid()
{
	return getuid();
}

bool IPCSkeleton::SetContextObject(sptr< IRemoteObject > &object)
{
	obj_ = object;
	return true;
}

sptr< IRemoteObject > IPCSkeleton::GetContextObject()
{
	if (obj_ == nullptr) {
		obj_ = new IRemoteObject();
	}
	return obj_;
}

int IPCSkeleton::SocketListening(const char *addr)
{
	unlink(addr);

	int socketFd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (socketFd < 0) {
		IPC_LOG("Socket failed errno=%d\n", errno);
		return false;
	}

	struct sockaddr_un socketAddr;
	memset(&socketAddr, 0, sizeof(socketAddr));
	socketAddr.sun_family = AF_UNIX;
	strcpy(socketAddr.sun_path, addr);
	int ret = bind(socketFd, (struct sockaddr *)&socketAddr, sizeof(socketAddr));
	if (ret < 0) {
		IPC_LOG("Bind socket failed errno=%d\n", errno);
		close(socketFd);
		socketFd = -1;
		return false;
	}

	ret = listen(socketFd, 3);
	if (ret < 0) {
		IPC_LOG("listen socket failed errno=%d\n", errno);
		close(socketFd);
		socketFd = -1;
		return false;
	}
	return socketFd;
}

int IPCSkeleton::SocketReadFd(int socketFd)
{
	if (socketFd < 0) {
		IPC_LOG("Read fd from an uninitialized socket\n");
		return -1;
	}

	struct sockaddr_un acceptAddr;
	socklen_t sockLen = sizeof(acceptAddr);
	int recvFd = accept(socketFd, (struct sockaddr *)&acceptAddr, &sockLen);
	if (recvFd < 0) {
		IPC_LOG("Accept failed errno=%d\n", errno);
		return -1;
	}

	struct msghdr msg;
	struct iovec iov[1];
	char buf[100] = "";
	msg.msg_name = nullptr;
	msg.msg_namelen = 0;
	iov[0].iov_base = buf;
	iov[0].iov_len = 100;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	msg.msg_flags = 0;
	char cm[CMSG_LEN(sizeof(int))];
	msg.msg_control = cm;
	msg.msg_controllen = CMSG_LEN(sizeof(int));
	int ret = recvmsg(recvFd, &msg, 0);
	if (ret < 0) {
		IPC_LOG("Receive error, errno=%d\n", errno);
		close(recvFd);
		return -1;
	}

	struct cmsghdr *cmsgPtr = CMSG_FIRSTHDR(&msg);
	if (cmsgPtr == nullptr || cmsgPtr->cmsg_len != CMSG_LEN(sizeof(int)) ||
		cmsgPtr->cmsg_level != SOL_SOCKET ||
		cmsgPtr->cmsg_type != SCM_RIGHTS) {
		IPC_LOG("Received wrong data\n");
		close(recvFd);
		return -1;
	}
	close(recvFd);
	return *((int *)CMSG_DATA(cmsgPtr));
}

bool IPCSkeleton::SocketWriteFd(const char *addr, int fd)
{
	int socketFd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (socketFd < 0) {
		IPC_LOG("Socket failed errno=%d\n", errno);
		return false;
	}

	struct sockaddr_un socketAddr;
	memset(&socketAddr, 0, sizeof(socketAddr));
	socketAddr.sun_family = AF_UNIX;
	strcpy(socketAddr.sun_path, addr);
	int ret = connect(socketFd, (struct sockaddr *)&socketAddr, sizeof(socketAddr));
	if (ret < 0) {
		IPC_LOG("Connect failed errno=%d\n", errno);
		close(socketFd);
		return false;
	}

	struct msghdr msg;
	struct iovec iov[1];
	char buf[100] = "IPC Socket Data with File Descriptor";
	msg.msg_name = nullptr;
	msg.msg_namelen = 0;
	iov[0].iov_base = buf;
	iov[0].iov_len = 100;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	msg.msg_flags = 0;
	char cm[CMSG_LEN(sizeof(int))];
	msg.msg_control = cm;
	msg.msg_controllen = CMSG_LEN(sizeof(int));
	struct cmsghdr *cmsgPtr = CMSG_FIRSTHDR(&msg);
	cmsgPtr->cmsg_len = CMSG_LEN(sizeof(int));
	cmsgPtr->cmsg_level = SOL_SOCKET;
	cmsgPtr->cmsg_type = SCM_RIGHTS;
	*((int *)CMSG_DATA(cmsgPtr)) = fd;
	ret = sendmsg(socketFd, &msg, 0);
	if (ret < 0) {
		IPC_LOG("Send failed errno=%d\n", errno);
	}
	close(socketFd);
	return ret >= 0;
}

} //namespace OHOS
