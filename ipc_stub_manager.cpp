#include "ipc_stub_manager.h"

namespace OHOS {

std::map< unsigned long long, sptr< IPCObjectStub > > IPCStubManager::stubMap_;
std::mutex IPCStubManager::mutex_;

unsigned long long IPCStubManager::GetHandleNum()
{
	std::lock_guard< std::mutex > lock(mutex_);
	return stubMap_.size();
}

void IPCStubManager::InsertStub(unsigned long long handle, sptr< IPCObjectStub > stub)
{
	std::lock_guard< std::mutex > lock(mutex_);
	stubMap_.emplace(handle, stub);
}

} // namespace OHOS