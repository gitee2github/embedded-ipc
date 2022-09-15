#ifndef _IREMOTE_BROKER_H
#define _IREMOTE_BROKER_H

#ifdef __cplusplus
#include <unordered_map>
#include <functional>
#include <mutex>
#include "iremote_object.h"
#include "refbase.h"

namespace OHOS {

class IRemoteBroker : public virtual RefBase {
public:
	IRemoteBroker() = default;
	virtual ~IRemoteBroker() override = default;
	virtual sptr< IRemoteObject > AsObject() = 0;
	static inline sptr< IRemoteBroker > AsImplement(const sptr< IRemoteObject > &object)
	{
		return nullptr;
	}
};

#define DECLARE_INTERFACE_DESCRIPTOR(DESCRIPTOR)				\
	static inline const std::u16string metaDescriptor_ = {DESCRIPTOR} ;	\
	static inline const std::u16string &GetDescriptor()			\
	{									\
		return metaDescriptor_;						\
	}

template <typename T> class BrokerCreator {
public:
	BrokerCreator() = default;
	~BrokerCreator() = default;
	sptr<IRemoteBroker> operator () (const sptr<IRemoteObject> &object)
	{
		T *proxy = new (std::nothrow) T(object);
		if (proxy != nullptr) {
			return static_cast<IRemoteBroker *>(proxy);
		}
		return nullptr;
	};
};

class BrokerRegistration {
	using Constructor = std::function<sptr<IRemoteBroker>(const sptr<IRemoteObject> &object)>;

public:
	static BrokerRegistration &Get();
	bool Register(const std::u16string &descriptor, const Constructor &creator);
	void Unregister(const std::u16string &descriptor);
	sptr<IRemoteBroker> NewInstance(const std::u16string &descriptor, const sptr<IRemoteObject> &object);

protected:
	BrokerRegistration() = default;
	~BrokerRegistration();

private:
	BrokerRegistration(const BrokerRegistration &) = delete;
	BrokerRegistration(BrokerRegistration &&) = delete;
	BrokerRegistration &operator = (const BrokerRegistration &) = delete;
	BrokerRegistration &operator = (BrokerRegistration &&) = delete;
	std::mutex creatorMutex_;
	std::unordered_map<std::u16string, Constructor> creators_;
};

template <typename T> class BrokerDelegator {
public:
	BrokerDelegator();
	~BrokerDelegator();

private:
	BrokerDelegator(const BrokerDelegator &) = delete;
	BrokerDelegator(BrokerDelegator &&) = delete;
	BrokerDelegator &operator = (const BrokerDelegator &) = delete;
	BrokerDelegator &operator = (BrokerDelegator &&) = delete;
};

template <typename T> BrokerDelegator<T>::BrokerDelegator()
{
	const std::u16string descriptor = T::GetDescriptor();
	BrokerRegistration &registration = BrokerRegistration::Get();
	registration.Register(descriptor, BrokerCreator<T>());
}

template <typename T> BrokerDelegator<T>::~BrokerDelegator()
{
	const std::u16string descriptor = T::GetDescriptor();
	BrokerRegistration &registration = BrokerRegistration::Get();
	registration.Unregister(descriptor);
}

template <typename INTERFACE> inline sptr<INTERFACE> iface_cast(const sptr<IRemoteObject> &object)
{
	const std::u16string descriptor = INTERFACE::GetDescriptor();
	BrokerRegistration &registration = BrokerRegistration::Get();
	sptr<IRemoteBroker> broker = registration.NewInstance(descriptor, object);
	return static_cast<INTERFACE *>(broker.GetRefPtr());
}

};

#endif

#endif
