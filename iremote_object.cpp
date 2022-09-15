#include "ipc_base.h"
#include "iremote_object.h"
#include "ipc_skeleton.h"
#include "ipc_socket_manager.h"

key_t g_client_server_shmKey = 0x544F53;
key_t g_device_auth_shmKey = 0x52544F;

namespace OHOS {

static const char *IPC_SERVER_SOCKET_ADDR = "/tmp/ipc.socket.server";

IRemoteObject::IRemoteObject() : handle_(0), isDSoftBusObj(true) {}

IRemoteObject::~IRemoteObject() {}

bool IRemoteObject::AddDeathRecipient(const sptr< DeathRecipient > &recipient)
{
	return true;
}

bool IRemoteObject::RemoveDeathRecipient(const sptr< DeathRecipient > &recipient)
{
	return true;
}

int IRemoteObject::SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
	IpcShmData *shmPtr = NULL;

	if (code == GET_SA_REQUEST_CODE || (code >= WIFI_DEVICE_ABILITY_ID && code <= WIFI_P2P_ABILITY_ID)) {
		reply.isContainHandle_ = true;
		reply.RemoteObjectHandle_ = 0;
		return 0;
	}

	shmPtr = OpenShm(isDSoftBusObj ? g_client_server_shmKey : g_device_auth_shmKey);
	if (shmPtr == nullptr) {
		IPC_LOG("Open server shm failed\n");
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
		if (!IPCSkeleton::SocketWriteFd(IPC_SERVER_SOCKET_ADDR, data.ReadFileDescriptor())) {
			IPC_LOG("Send File Descriptor failed\n");
			shmdt((void*)shmPtr);
			return -1;
		}
	}
	if (data.isContainHandle_) {
		shmPtr->containHandle = true;
		shmPtr->handle = data.RemoteObjectHandle_;
	}
	shmPtr->needReply = true;

	// waiting receiver reply
	while (shmPtr->needReply);
	IPC_DEBUG("RECEIVED DATA FROM REMOTE with code=%u\n", code);

	reply.WriteUnpadBuffer(shmPtr->outputData, shmPtr->outputSz);
	if (shmPtr->containFd) {
		if (!reply.WriteFileDescriptor(IPCSkeleton::SocketReadFd(IPCSocketManager::FindSocketFd(0)))) {
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

bool IRemoteObject::Marshalling(Parcel &parcel) const
{
	return true;
}

unsigned long long IRemoteObject::GetHandle()
{
	return handle_;
}

sptr< IRemoteBroker > IRemoteObject::Asnterface()
{
	return nullptr;
}

bool IRemoteObject::IsProxyObject() const
{
	return true;
}

} // namespace OHOS
