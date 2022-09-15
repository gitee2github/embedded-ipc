#include <thread>
#include <cstring>

#include "ipc_base.h"
#include "ipc_center.h"
#include "ipc_skeleton.h"
#include "ipc_socket_manager.h"
#include "ipc_proxy_manager.h"

namespace OHOS {

static const char *IPC_SERVER_SOCKET_ADDR = "/tmp/ipc.socket.server";
static const char *IPC_CLIENT_SOCKET_ADDR = "/tmp/ipc.socket.client";

size_t IpcCenter::threadNum_ = 0;

IpcCenter::IpcCenter() {}

IpcCenter::~IpcCenter() {}

bool IpcCenter::ShmInit(key_t shmKey)
{
	IpcShmData *shmPtr = OpenShm(shmKey);
	if (shmPtr == nullptr) {
		IPC_LOG("Create shm with key=0x%x\n", shmKey);
		return false;
	}
	shmPtr->needReply = false;
	shmPtr->containFd = false;
	shmPtr->isProcessing = false;
	shmPtr->deadNotice = false;
	shmdt((void *)shmPtr);
	return true;
}

bool IpcCenter::Init(bool isServer, IPCObjectStub *stub)
{
	if (!isServer) {
		IPC_LOG("Only the server can call this interface\n");
		return false;
	}

	if (stub == nullptr) {
		IPC_LOG("Invalid stub\n");
		return false;
	}

	if (isServer && !ShmInit(g_client_server_shmKey)) {
		IPC_LOG("Shm init failed\n");
		return false;
	}

	if (stub->recvFd_ >= 0) {
		close(stub->recvFd_);
	}

	stub->recvFd_ = IPCSkeleton::SocketListening(IPC_SERVER_SOCKET_ADDR);
	if (stub->recvFd_ < 0) {
		IPC_LOG("Starting socket listen failed\n");
		return false;
	}

	IPCSocketManager::InsertSocketFd(0, stub->recvFd_);
	stub->sendAddr_ = IPC_CLIENT_SOCKET_ADDR;
	stub->handle_ = 0;

	return ThreadCreate(stub);
}

void IpcCenter::ProcessHandle(key_t shmKey, IPCObjectStub *ipcStub)
{
	IpcShmData *shmPtr = OpenShm(shmKey);
	if (shmPtr == nullptr) {
		IPC_LOG("Open shm failed");
		return;
	}
	IPC_LOG("STUB LISTENING with handle=%llx\n", ipcStub->handle_);
	do {
		while (!shmPtr->needReply) {
			usleep(10);
			if (ipcStub->needStop_) {
				IPC_LOG("STUB LISTENING END with handle=%llx\n", ipcStub->handle_);
				shmdt((void*)shmPtr);
				return;
			}
		}

		if (shmPtr->deadNotice) {
			if (ipcStub->handle_) {
				IPC_LOG("Client received a wrong notice\n");
				shmdt((void*)shmPtr);
				return;
			}
			IPCProxyManager::CleanProxy(shmPtr->handle);
			shmPtr->deadNotice = false;
			shmPtr->needReply = false;
			continue;
		}

		IPC_LOG("PROCESSING REMOTE REQUEST with handle=%llx\n", ipcStub->handle_);
		MessageParcel data, reply;
		MessageOption option;
		data.WriteUnpadBuffer(shmPtr->inputData, shmPtr->inputSz);
		if (shmPtr->containFd) {
			shmPtr->containFd = false;
			if (!data.WriteFileDescriptor(IPCSkeleton::SocketReadFd(ipcStub->recvFd_))) {
				IPC_LOG("Process file descriptor failed\n");
				shmdt((void *)shmPtr);
				return;
			}
		}
		if (shmPtr->containHandle) {
			data.RemoteObjectHandle_ = shmPtr->handle;
			data.isContainHandle_ = true;
			shmPtr->containHandle = false;
		}
		ipcStub->OnRemoteRequest(shmPtr->requestCode, data, reply, option);
		shmPtr->outputSz = reply.GetDataSize();
		if (shmPtr->outputSz > DATA_SIZE) {
			IPC_LOG("Callback data overflow!\n");
		}
		memcpy(shmPtr->outputData, (void*)reply.GetData(), shmPtr->outputSz);
		if (reply.ContainFileDescriptors()) {
			if (!IPCSkeleton::SocketWriteFd(ipcStub->sendAddr_, reply.ReadFileDescriptor())) {
				IPC_LOG("Send file descriptor in reply failed\n");
				shmdt((void *)shmPtr);
				return;
			}
			shmPtr->containFd = true;
		}
		if (reply.ContainRemoteObject()) {
			shmPtr->handle = reply.RemoteObjectHandle_;
			shmPtr->containHandle = true;
		}
		shmPtr->needReply = false;
		IPC_LOG("IPC STUB PROCESS END with handle=%llx\n", ipcStub->handle_);
	} while (!ipcStub->needStop_);
	IPC_LOG("STUB LISTENING END with handle=%llx\n", ipcStub->handle_);
	shmdt((void*)shmPtr);
}

bool IpcCenter::ThreadCreate(IPCObjectStub *stub)
{
	++threadNum_;
	std::thread new_thread(std::bind(&IpcCenter::ProcessHandle, g_client_server_shmKey, stub));
	new_thread.detach();
	return true;
}

} // namespace OHOS
