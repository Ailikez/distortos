/**
 * \file
 * \brief MutexOperationsTestCase class implementation
 *
 * \author Copyright (C) 2014 Kamil Szczygiel http://www.distortec.com http://www.freddiechopin.info
 *
 * \par License
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \date 2014-11-11
 */

#include "MutexOperationsTestCase.hpp"

#include "mutexTestTryLockWhenLocked.hpp"
#include "waitForNextTick.hpp"

#include "distortos/Mutex.hpp"
#include "distortos/StaticThread.hpp"
#include "distortos/ThisThread.hpp"

#include <cerrno>

namespace distortos
{

namespace test
{

namespace
{

/*---------------------------------------------------------------------------------------------------------------------+
| local constants
+---------------------------------------------------------------------------------------------------------------------*/

/// single duration used in tests
constexpr auto singleDuration = TickClock::duration{1};

/// size of stack for test thread, bytes
constexpr size_t testThreadStackSize {256};

/// priority of current test thread and lock-unlock test thread
constexpr uint8_t testThreadPriority {UINT8_MAX - 1};

/*---------------------------------------------------------------------------------------------------------------------+
| local functions
+---------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Simple thread that locks and unlocks the mutex, with additional correctness checks.
 *
 * First the mutex is locked and this operation is checked - it must happen immediately and it must return 0. Result of
 * that check can be read from sharedRet. In case of failure, thread returns at this point.
 *
 * Then thread waits for semaphoreMutex to be unlocked.
 *
 * Last operation is unlocking of the mutex, which is checked in the same way as locking. Result can be read from
 * sharedRet after this thread terminates.
 *
 * \param [in] mutex is a reference to mutex that will be locked and unlocked, mut be unlocked
 * \param [out] sharedRet is a reference to variable used to return result of operations
 * \param [in] semaphoreMutex is a mutex used as semaphore, must be locked
 */

void lockUnlockThread(Mutex& mutex, bool& sharedRet, Mutex& semaphoreMutex)
{
	{
		// simple lock - must succeed immediately
		const auto start = TickClock::now();
		const auto ret = mutex.lock();
		sharedRet = ret == 0 && start == TickClock::now();
	}

	if (sharedRet == false)
		return;

	semaphoreMutex.lock();
	semaphoreMutex.unlock();

	{
		// simple unlock - must succeed immediately
		const auto start = TickClock::now();
		const auto ret = mutex.unlock();
		sharedRet = ret == 0 && start == TickClock::now();
	}
}

/**
 * \brief Tests Mutex::unlock() - it must succeed immediately
 *
 * \param [in] mutex is a reference to mutex that will be unlocked
 *
 * \return true if test succeeded, false otherwise
 */

bool testUnlock(Mutex& mutex)
{
	waitForNextTick();
	const auto start = TickClock::now();
	const auto ret = mutex.unlock();
	if (ret != 0 || start != TickClock::now())
		return false;

	return true;
}

/**
 * \brief Phase 1 of test case.
 *
 * Tests whether all tryLock*() functions properly return some error when dealing with locked mutex.
 *
 * \param [in] type is the type of mutex
 * \param [in] protocol is the mutex protocol
 * \param [in] priorityCeiling is the priority ceiling of mutex, ignored when protocol != Protocol::PriorityProtect
 *
 * \return true if test succeeded, false otherwise
 */

bool phase1(const Mutex::Type type, const Mutex::Protocol protocol, const uint8_t priorityCeiling)
{
	Mutex mutex{type, protocol, priorityCeiling};
	bool sharedRet {};
	Mutex semaphoreMutex;
	semaphoreMutex.lock();

	auto lockUnlockThreadObject = makeStaticThread<testThreadStackSize>(testThreadPriority, lockUnlockThread,
			std::ref(mutex), std::ref(sharedRet), std::ref(semaphoreMutex));

	waitForNextTick();
	lockUnlockThreadObject.start();
	ThisThread::yield();
	if (sharedRet == false)
	{
		lockUnlockThreadObject.join();
		return false;
	}

	{
		const auto ret = mutexTestTryLockWhenLocked(mutex, testThreadPriority);
		if (ret != true)
			return ret;
	}

	{
		// mutex is locked, so tryLockFor() should time-out at expected time
		waitForNextTick();
		const auto start = TickClock::now();
		const auto ret = mutex.tryLockFor(singleDuration);
		const auto realDuration = TickClock::now() - start;
		if (ret != ETIMEDOUT || realDuration != singleDuration + decltype(singleDuration){1})
			return false;
	}

	{
		// mutex is locked, so tryLockUntil() should time-out at exact expected time
		waitForNextTick();
		const auto requestedTimePoint = TickClock::now() + singleDuration;
		const auto ret = mutex.tryLockUntil(requestedTimePoint);
		if (ret != ETIMEDOUT || requestedTimePoint != TickClock::now())
			return false;
	}

	waitForNextTick();
	semaphoreMutex.unlock();
	lockUnlockThreadObject.join();

	return sharedRet;
}

/**
 * \brief Phase 2 of test case.
 *
 * Tests whether all tryLock*() functions properly lock unlocked mutex.
 *
 * \param [in] type is the type of mutex
 * \param [in] protocol is the mutex protocol
 * \param [in] priorityCeiling is the priority ceiling of mutex, ignored when protocol != Protocol::PriorityProtect
 *
 * \return true if test succeeded, false otherwise
 */

bool phase2(const Mutex::Type type, const Mutex::Protocol protocol, const uint8_t priorityCeiling)
{
	Mutex mutex{type, protocol, priorityCeiling};

	{
		// mutex is unlocked, so tryLock() must succeed immediately
		waitForNextTick();
		const auto start = TickClock::now();
		const auto ret = mutex.tryLock();
		if (ret != 0 || start != TickClock::now())
			return false;
	}

	{
		const auto ret = mutexTestTryLockWhenLocked(mutex, testThreadPriority);
		if (ret != true)
			return ret;
	}

	{
		const auto ret = testUnlock(mutex);
		if (ret != true)
			return ret;
	}

	{
		// mutex is unlocked, so tryLockFor() should succeed immediately
		waitForNextTick();
		const auto start = TickClock::now();
		const auto ret = mutex.tryLockFor(singleDuration);
		if (ret != 0 || start != TickClock::now())
			return false;
	}

	{
		const auto ret = mutexTestTryLockWhenLocked(mutex, testThreadPriority);
		if (ret != true)
			return ret;
	}

	{
		const auto ret = testUnlock(mutex);
		if (ret != true)
			return ret;
	}

	{
		// mutex is unlocked, so tryLockUntil() should succeed immediately
		waitForNextTick();
		const auto start = TickClock::now();
		const auto ret = mutex.tryLockUntil(start + singleDuration);
		if (ret != 0 || start != TickClock::now())
			return false;
	}

	{
		const auto ret = mutexTestTryLockWhenLocked(mutex, testThreadPriority);
		if (ret != true)
			return ret;
	}

	{
		const auto ret = testUnlock(mutex);
		if (ret != true)
			return ret;
	}

	return true;
}

/**
 * \brief Runs the test case.
 *
 * \attention this function expects the priority of test thread to be testThreadPriority
 *
 * \return true if the test case succeeded, false otherwise
 */

bool testRunner()
{
	using Parameters = std::tuple<Mutex::Type, Mutex::Protocol, uint8_t>;
	static const Parameters parametersArray[]
	{
			Parameters{Mutex::Type::Normal, Mutex::Protocol::None, {}},
			Parameters{Mutex::Type::Normal, Mutex::Protocol::PriorityProtect, UINT8_MAX},
			Parameters{Mutex::Type::Normal, Mutex::Protocol::PriorityProtect, testThreadPriority},
			Parameters{Mutex::Type::Normal, Mutex::Protocol::PriorityInheritance, {}},
			Parameters{Mutex::Type::ErrorChecking, Mutex::Protocol::None, {}},
			Parameters{Mutex::Type::ErrorChecking, Mutex::Protocol::PriorityProtect, UINT8_MAX},
			Parameters{Mutex::Type::ErrorChecking, Mutex::Protocol::PriorityProtect, testThreadPriority},
			Parameters{Mutex::Type::ErrorChecking, Mutex::Protocol::PriorityInheritance, {}},
			Parameters{Mutex::Type::Recursive, Mutex::Protocol::None, {}},
			Parameters{Mutex::Type::Recursive, Mutex::Protocol::PriorityProtect, UINT8_MAX},
			Parameters{Mutex::Type::Recursive, Mutex::Protocol::PriorityProtect, testThreadPriority},
			Parameters{Mutex::Type::Recursive, Mutex::Protocol::PriorityInheritance, {}},
	};

	for (const auto& parameters : parametersArray)
	{
		const auto ret1 = phase1(std::get<0>(parameters), std::get<1>(parameters), std::get<2>(parameters));
		if (ret1 != true)
			return ret1;

		const auto ret2 = phase2(std::get<0>(parameters), std::get<1>(parameters), std::get<2>(parameters));
		if (ret2 != true)
			return ret2;
	}

	return true;
}

}	// namespace

/*---------------------------------------------------------------------------------------------------------------------+
| private functions
+---------------------------------------------------------------------------------------------------------------------*/

bool MutexOperationsTestCase::run_() const
{
	const auto thisThreadPriority = ThisThread::getPriority();
	ThisThread::setPriority(testThreadPriority);
	const auto ret = testRunner();
	ThisThread::setPriority(thisThreadPriority);
	return ret;
}

}	// namespace test

}	// namespace distortos
