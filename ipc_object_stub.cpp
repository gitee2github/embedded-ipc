#include <thread>

#include "ipc_base.h"
#include "ipc_center.h"
#include "ipc_skeleton.h"
#include "ipc_stub_manager.h"
#include "ipc_object_stub.h"
#include "ipc_socket_manager.h"

namespace OHOS {

static const char *IPC_SERVER_SOCKET_ADDR = "/tmp/ipc.socket.server";
static const char *IPC_CLIENT_SOCKET_ADDR = "/tmp/ipc.socket.client";

IPCObjectStub::IPCObjectStub() : needStop_(false)
{
	handle_ = getpid();
	handle_ = (handle_ << 32) | IPCStubManager::GetHandleNum();
	IPCStubManager::InsertStub(handle_, this);
	key_t shmKey = HandleToKey(handle_);
	IpcCenter::ShmInit(shmKey);
	sendAddr_ = IPC_SERVER_SOCKET_ADDR;
	CreateThread(shmKey);
}

IPCObjectStub::~IPCObjectStub()
{
	SendDeadNotification();
}

int IPCObjectStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
	IPC_LOG("IPCObjectStub::OnRemoteRequest Called\n");
	return -1;
}

void IPCObjectStub::CreateThread(key_t shmKey)
{
	IpcCenter::threadNum_++;
	std::thread new_thread(std::bind(IpcCenter::ProcessHandle, shmKey, this));
	new_thread.detach();
	char cliend_socket_addr[MAX_SOCKET_ADDR_LEN];
	sprintf(cliend_socket_addr, "%s.%llx", IPC_CLIENT_SOCKET_ADDR, handle_);
	recvFd_ = IPCSkeleton::SocketListening(cliend_socket_addr);
	IPCSocketManager::InsertSocketFd(1, recvFd_);
	if (recvFd_ < 0) {
		IPC_LOG("Stub socket listen failed\n");
	}
}

void IPCObjectStub::SendDeadNotification()
{
	IpcShmData *shmPtr = OpenShm(g_client_server_shmKey);
	if (shmPtr == nullptr) {
		IPC_LOG("Open server shm failed\n");
		return;
	}

	// waiting previous ipc
	IPC_LOG("WAITING FOR PREVIOUS IPC\n");
	while (shmPtr->needReply);

	shmPtr->deadNotice = true;
	shmPtr->handle = handle_;

	shmPtr->needReply = true;

	shmdt((void *)shmPtr);
}

} // namespace OHOS
