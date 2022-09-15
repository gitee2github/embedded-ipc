#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "ipc_base.h"
#include "message_parcel.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"
#include "ipc_proxy_manager.h"
#include "ipc_stub_manager.h"

namespace OHOS {

MessageParcel::MessageParcel() : Parcel(), isContainHandle_(false), fd_(-1) {}

MessageParcel::~MessageParcel() {}

bool MessageParcel::WriteFileDescriptor(int fd)
{
	if (fd < 0) {
		return false;
	}

	int dupFd = dup(fd);
	if (dupFd < 0) {
		return false;
	}

	fd_ = dupFd;

	return true;
}

int MessageParcel::ReadFileDescriptor()
{
	return fd_;
}

bool MessageParcel::ContainFileDescriptors() const
{
	return fd_ >= 0;
}

bool MessageParcel::WriteRawData(const void *data, size_t size)
{
	if (data == nullptr || size > MAX_RAWDATA_SIZE) {
		return false;
	}
	if (!WriteInt32(size)) {
		return false;
	}

	return WriteUnpadBuffer(data, size);
}

const void *MessageParcel::ReadRawData(size_t size)
{
	int32_t bufferSize = ReadInt32();
	if (static_cast< unsigned int >(bufferSize) != size) {
		return nullptr;
	}
	return ReadUnpadBuffer(size);
}

bool MessageParcel::ContainRemoteObject()
{
	return isContainHandle_;
}

sptr< IRemoteObject > MessageParcel::ReadRemoteObject()
{
	if (!isContainHandle_) {
		IPC_LOG("Read an invalid remote object!\n");
		return nullptr;
	}
	if (!RemoteObjectHandle_) {
		return IPCSkeleton::GetContextObject();
	}
	return IPCProxyManager::FindOrNewProxy(RemoteObjectHandle_);
}

bool MessageParcel::WriteRemoteObject(const sptr< IRemoteObject > &object)
{
	isContainHandle_ = true;
	RemoteObjectHandle_ = object->GetHandle();
	return true;
}

} //namespace OHOS
