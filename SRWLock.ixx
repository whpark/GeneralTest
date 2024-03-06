module;

// from : 
// https://www.reddit.com/r/cpp/comments/1b55686/maybe_possible_bug_in_stdshared_mutex_on_windows/

#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <shared_mutex>
#include <thread>
#include <vector>

#include <catch.hpp>

export module SRWLock;

#if 0

namespace test {

	struct ThreadTestData {
		int32_t numThreads = 0;
		std::shared_mutex sharedMutex = {};
		std::atomic<int32_t> readCounter;
	};
	std::atomic<bool> bDeadlockDetected = false;

	int DoStuff(ThreadTestData* data) {
		// Acquire reader lock
		data->sharedMutex.lock_shared();

		// wait until all read threads have acquired their shared lock
		data->readCounter.fetch_add(1);
		using clock_t = std::chrono::steady_clock;
		auto t0 = clock_t::now();
		while (data->readCounter.load() != data->numThreads) {
			if (clock_t::now() - t0 > std::chrono::seconds(1)) {
				bDeadlockDetected = true;
				std::cerr << "Deadlock detected" << std::endl;
				break;
			}
			if (bDeadlockDetected.load(std::memory_order_relaxed)) {
				break;
			}
			std::this_thread::yield();
		}

		// Release reader lock
		data->sharedMutex.unlock_shared();

		return 0;
	}

	TEST_CASE("SRWLock", "[SRWLock]") {
		int count = 0;
		while (!bDeadlockDetected.load(std::memory_order_relaxed)) {
			ThreadTestData data = {};
			data.numThreads = 5;

			// Acquire write lock
			data.sharedMutex.lock();

			// Create N threads
			std::vector<std::unique_ptr<std::thread>> readerThreads;
			readerThreads.reserve(data.numThreads);
			for (int i = 0; i < data.numThreads; ++i) {
				readerThreads.emplace_back(std::make_unique<std::thread>(DoStuff, &data));
			}

			// Release write lock
			data.sharedMutex.unlock();

			// Wait for all readers to succeed
			for (auto& thread : readerThreads) {
				thread->join();
			}

			// Cleanup
			readerThreads.clear();

			// Spew so we can tell when it's deadlocked
			count += 1;
			std::cout << count << std::endl;
		}

	}

} // namespace test

#endif
