#ifndef _IPC_OBJECT_PROXY_H_
#define _IPC_OBJECT_PROXY_H_

#include "ipc_base.h"
#include "iremote_object.h"

namespace OHOS {

class IPCObjectProxy : public IRemoteObject {
public:
	IPCObjectProxy(unsigned long long handle);
	~IPCObjectProxy();
	int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
	bool AddDeathRecipient(const sptr< DeathRecipient > &recipient) override;
	void SendObituary();
private:
	key_t sendShmKey_;
	char socketAddr_[MAX_SOCKET_ADDR_LEN];
	int recvFd_;
	sptr< DeathRecipient > deathRecipient_;
};

}; // namespace OHOS

#endif // _IPC_OBJECT_PROXY_H_