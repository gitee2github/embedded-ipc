#ifndef _IPC_REMOTE_OBJECT_H_
#define _IPC_REMOTE_OBJECT_H_

#include "message_parcel.h"
#include "message_option.h"
#include "iremote_broker.h"

#define ERR_NONE 0
#define NO_ERROR 0

namespace OHOS {

class IRemoteBroker;

class IRemoteObject : public virtual Parcelable {
public:
	IRemoteObject();
	~IRemoteObject();
	class DeathRecipient : public RefBase {
	public:
		virtual void OnRemoteDied(const wptr< IRemoteObject > &object) {}
	};

	virtual int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
	virtual bool AddDeathRecipient(const sptr< DeathRecipient > &recipient);
	virtual bool RemoveDeathRecipient(const sptr< DeathRecipient > &recipient);
	virtual bool Marshalling(Parcel &parcel) const override;
	virtual bool IsProxyObject() const;
	virtual sptr< IRemoteBroker > AsInterface();
	unsigned long long GetHandle();
	unsigned long long handle_;
	bool isDSoftBusObj;
};

} // namespace OHOS

#endif // _IPC_REMOTE_OBJECT_H_
