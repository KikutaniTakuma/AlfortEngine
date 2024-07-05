#include "Thread.h"
#include "Engine/Engine.h"
#include "Utils/ExecutionLog.h"
#include "Error/Error.h"


uint32_t Lamb::Thread::currentThreadNum_ = 0;
uint32_t Lamb::Thread::kHardwareThread_  = std::thread::hardware_concurrency();

Lamb::Thread::~Thread() {
	exit_ = true;
	condition_->notify_all();
	if (thraed_->joinable()) {
		thraed_->join();
	}
}

void Lamb::Thread::Notify() {
	condition_->notify_all();
}

void Lamb::Thread::Create(
	const std::function<void(void)>& userProcess, 
	const std::function<bool(void)>& waitProcess, 
	const std::function<bool(void)>& restartProcess
) {
	if (kHardwareThread_ <= currentThreadNum_) {
		throw Lamb::Error::Code<Lamb::Thread>("over hardware thread number", ErrorPlace);
	}
	if (thraed_ and thraed_->joinable()) {
		throw Lamb::Error::Code<Lamb::Thread>("Already created", ErrorPlace);
	}

	mtx_ = std::make_unique<std::mutex>();
	condition_ = std::make_unique<std::condition_variable>();

	userProcess_ = std::make_unique<std::function<void(void)>>(userProcess);
	waitProcess_ = std::make_unique<std::function<bool(void)>>(waitProcess);
	restartProcess_ = std::make_unique<std::function<bool(void)>>(restartProcess);

	threadProcess_ = std::make_unique<std::function<void(void)>>(
		[this]() {
		HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
		if (SUCCEEDED(hr)) {
			Lamb::AddLog("CoInitializeEx succeeded");
		}
		else {
			throw Lamb::Error::Code<Lamb::Thread>("CoInitializeEx failed", ErrorPlace);
		}

		std::unique_lock<std::mutex> uniqueLock(*mtx_);

		auto& waitProcess = *waitProcess_;
		auto& restartProcess = *restartProcess_;
		auto& userProcess = *userProcess_;
		auto& condition = *condition_;

		while (!exit_) {
			if (waitProcess()) {
				isWait_ = true;
				condition.wait(uniqueLock, [this, &restartProcess]() { return restartProcess() or exit_; });
			}
			if (exit_) {
				break;
			}
			isWait_ = false;

			userProcess();
		}

		// COM 終了
		CoUninitialize();
		Lamb::AddLog("CoUninitialize succeeded");
		}
	);

	thraed_ = std::make_unique<std::thread>(*threadProcess_);
}
