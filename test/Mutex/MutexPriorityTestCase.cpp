/**
 * \file
 * \brief MutexPriorityTestCase class implementation
 *
 * \author Copyright (C) 2014-2016 Kamil Szczygiel http://www.distortec.com http://www.freddiechopin.info
 *
 * \par License
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "MutexPriorityTestCase.hpp"

#include "priorityTestPhases.hpp"
#include "SequenceAsserter.hpp"

#include "distortos/architecture/InterruptMaskingLock.hpp"

#include "distortos/DynamicThread.hpp"
#include "distortos/Mutex.hpp"
#include "distortos/ThisThread.hpp"

namespace distortos
{

namespace test
{

namespace
{

/*---------------------------------------------------------------------------------------------------------------------+
| local constants
+---------------------------------------------------------------------------------------------------------------------*/

/// size of stack for test thread, bytes
constexpr size_t testThreadStackSize {512};

/*---------------------------------------------------------------------------------------------------------------------+
| local functions
+---------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Test thread
 *
 * Locks the mutex, marks the sequence point in SequenceAsserter and unlocks the mutex.
 *
 * \param [in] sequenceAsserter is a reference to SequenceAsserter shared object
 * \param [in] sequencePoint is the sequence point of this instance
 * \param [in] mutex is a reference to shared mutex
 */

void thread(SequenceAsserter& sequenceAsserter, const unsigned int sequencePoint, Mutex& mutex)
{
	mutex.lock();
	sequenceAsserter.sequencePoint(sequencePoint);
	mutex.unlock();
}

/**
 * \brief Builder of test threads
 *
 * \param [in] threadParameters is a reference to ThreadParameters object
 * \param [in] sequenceAsserter is a reference to SequenceAsserter shared object
 * \param [in] mutex is a reference to shared mutex
 *
 * \return constructed DynamicThread object
 */

DynamicThread makeTestThread(const ThreadParameters& threadParameters, SequenceAsserter& sequenceAsserter, Mutex& mutex)
{
	return makeDynamicThread({testThreadStackSize, threadParameters.first}, thread, std::ref(sequenceAsserter),
			threadParameters.second, std::ref(mutex));
}

}	// namespace

/*---------------------------------------------------------------------------------------------------------------------+
| private functions
+---------------------------------------------------------------------------------------------------------------------*/

bool MutexPriorityTestCase::run_() const
{
	using Parameters = std::tuple<Mutex::Type, Mutex::Protocol, uint8_t>;
	static const Parameters parametersArray[]
	{
			Parameters{Mutex::Type::Normal, Mutex::Protocol::none, {}},
			Parameters{Mutex::Type::Normal, Mutex::Protocol::priorityProtect, UINT8_MAX},
			Parameters{Mutex::Type::Normal, Mutex::Protocol::priorityInheritance, {}},
			Parameters{Mutex::Type::ErrorChecking, Mutex::Protocol::none, {}},
			Parameters{Mutex::Type::ErrorChecking, Mutex::Protocol::priorityProtect, UINT8_MAX},
			Parameters{Mutex::Type::ErrorChecking, Mutex::Protocol::priorityInheritance, {}},
			Parameters{Mutex::Type::Recursive, Mutex::Protocol::none, {}},
			Parameters{Mutex::Type::Recursive, Mutex::Protocol::priorityProtect, UINT8_MAX},
			Parameters{Mutex::Type::Recursive, Mutex::Protocol::priorityInheritance, {}},
	};

	for (const auto& parameters : parametersArray)
		for (const auto& phase : priorityTestPhases)
		{
			SequenceAsserter sequenceAsserter;
			Mutex mutex {std::get<0>(parameters), std::get<1>(parameters), std::get<2>(parameters)};

			std::array<DynamicThread, totalThreads> threads
			{{
					makeTestThread(phase.first[phase.second[0]], sequenceAsserter, mutex),
					makeTestThread(phase.first[phase.second[1]], sequenceAsserter, mutex),
					makeTestThread(phase.first[phase.second[2]], sequenceAsserter, mutex),
					makeTestThread(phase.first[phase.second[3]], sequenceAsserter, mutex),
					makeTestThread(phase.first[phase.second[4]], sequenceAsserter, mutex),
					makeTestThread(phase.first[phase.second[5]], sequenceAsserter, mutex),
					makeTestThread(phase.first[phase.second[6]], sequenceAsserter, mutex),
					makeTestThread(phase.first[phase.second[7]], sequenceAsserter, mutex),
					makeTestThread(phase.first[phase.second[8]], sequenceAsserter, mutex),
					makeTestThread(phase.first[phase.second[9]], sequenceAsserter, mutex),
			}};

			mutex.lock();

			{
				architecture::InterruptMaskingLock interruptMaskingLock;

				for (auto& thread : threads)
				{
					thread.start();
					// make sure each test threads blocks on mutex in the order of starting, even if current thread
					// inherits test thread's high priority
					ThisThread::sleepFor(TickClock::duration{1});
				}
			}

			bool invalidState {};
			for (const auto& thread : threads)
				if (thread.getState() != ThreadState::BlockedOnMutex)
					invalidState = true;

			mutex.unlock();

			for (auto& thread : threads)
				thread.join();

			if (invalidState != false)
				return false;

			if (sequenceAsserter.assertSequence(totalThreads) == false)
				return false;
		}

	return true;
}

}	// namespace test

}	// namespace distortos
