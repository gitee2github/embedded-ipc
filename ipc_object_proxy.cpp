#include "ipc_skeleton.h"
#include "ipc_object_proxy.h"
#include "ipc_socket_manager.h"

namespace OHOS {

static const char *IPC_CLIENT_SOCKET_ADDR = "/tmp/ipc.socket.client";

IPCObjectProxy::IPCObjectProxy(unsigned long long handle) :
	recvFd_(-1), deathRecipient_(nullptr)
{
	IPC_LOG("INSERT PROXY with handle=%llx\n", handle);
	sprintf(socketAddr_, "%s.%llx", IPC_CLIENT_SOCKET_ADDR, handle);
	handle_ = handle;
	sendShmKey_ = HandleToKey(handle);
}

IPCObjectProxy::~IPCObjectProxy() {}

int IPCObjectProxy::SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
	IpcShmData *shmPtr = NULL;

	if (code == GET_SA_REQUEST_CODE || (code >= WIFI_DEVICE_ABILITY_ID && code <= WIFI_P2P_ABILITY_ID)) {
		reply.isContainHandle_ = true;
		reply.RemoteObjectHandle_ = 0;
		return 0;
	}

	shmPtr = OpenShm(sendShmKey_);
	if (shmPtr == nullptr) {
		IPC_LOG("Open Stub shm failed\n");
		return -1;
	}

	IPC_DEBUG("WAITING FOR PREVIOUS IPC\n");

	// waiting previous ipc
	while (shmPtr->needReply);

	IPC_DEBUG("SENDING REQUEST with code=%u\n", code);

	shmPtr->requestCode = code;
	shmPtr->inputSz = data.GetDataSize();
	if (shmPtr->inputSz > DATA_SIZE) {
		IPC_LOG("Sending data overflow!");
	}
	memcpy(shmPtr->inputData, (void *)data.GetData(), shmPtr->inputSz);
	if (data.ContainFileDescriptors()) {
		IPC_DEBUG("SENDING FD\n");
		shmPtr->containFd = true;
		if (!IPCSkeleton::SocketWriteFd(socketAddr_, data.ReadFileDescriptor())) {
			IPC_LOG("Send File Descriptor failed\n");
			shmdt((void*)shmPtr);
			return -1;
		}
	}
	if (data.ContainRemoteObject()) {
		shmPtr->containHandle = true;
		shmPtr->handle = data.RemoteObjectHandle_;
	}
	shmPtr->needReply = true;

	IPC_DEBUG("WAITING STUB REPLY with handle=%llx\n", handle_);

	// waiting receiver reply
	while (shmPtr->needReply);
	IPC_DEBUG("RECEIVED DATA FROM REMOTE with code=%u\n", code);

	reply.WriteUnpadBuffer(shmPtr->outputData, shmPtr->outputSz);
	if (shmPtr->containFd) {
		if (!reply.WriteFileDescriptor(IPCSkeleton::SocketReadFd(IPCSocketManager::FindSocketFd(1)))) {
			IPC_LOG("Receive reply fd failed");
			shmdt((void *)shmPtr);
			return -1;
		}
		shmPtr->containFd = false;
	}
	if (shmPtr->containHandle) {
		reply.isContainHandle_ = true;
		reply.RemoteObjectHandle_ = shmPtr->handle;
		shmPtr->containHandle = false;
	}
	shmdt((void *)shmPtr);
	return 0;
}

bool IPCObjectProxy::AddDeathRecipient(const sptr< DeathRecipient > &recipient)
{
	deathRecipient_ = recipient;
	return true;
}

void IPCObjectProxy::SendObituary()
{
	if (deathRecipient_ != nullptr) {
		deathRecipient_->OnRemoteDied(this);
		deathRecipient_ = nullptr;
	}
}

} // namespace OHOS