#include "iremote_broker.h"
#include <utility>

namespace OHOS {
BrokerRegistration &BrokerRegistration::Get()
{
	static BrokerRegistration instance;
	return instance;
}

BrokerRegistration::~BrokerRegistration()
{
	std::lock_guard<std::mutex> lockGuard(creatorMutex_);
	for (auto it = creators_.begin(); it != creators_.end();) {
		it = creators_.erase(it);
	}
}

bool BrokerRegistration::Register(const std::u16string &descriptor, const Constructor &creator)
{
	if (descriptor.empty()) {
		return false;
	}

	std::lock_guard<std::mutex> lockGuard(creatorMutex_);
	auto it = creators_.find(descriptor);
	if (it == creators_.end()) {
		return creators_.insert({ descriptor, creator }).second;
	}
	return false;
}

void BrokerRegistration::Unregister(const std::u16string &descriptor)
{
	std::lock_guard<std::mutex> lockGuard(creatorMutex_);
	if (!descriptor.empty()) {
		auto it = creators_.find(descriptor);
		if (it != creators_.end()) {
			creators_.erase(it);
		}
	}
}

sptr<IRemoteBroker> BrokerRegistration::NewInstance(const std::u16string &descriptor, const sptr<IRemoteObject> &object)
{
	std::lock_guard<std::mutex> lockGuard(creatorMutex_);

	sptr<IRemoteBroker> broker;
	if (object != nullptr) {
		if (object->IsProxyObject()) {
			auto it = creators_.find(descriptor);
			if (it != creators_.end()) {
				broker = it->second(object);
			}
		} else {
			broker = object->AsInterface().GetRefPtr();
		}
	}
	return broker;
}
} // namespace OHOS
