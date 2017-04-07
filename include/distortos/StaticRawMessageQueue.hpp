/**
 * \file
 * \brief StaticRawMessageQueue and StaticRawMessageQueue2 classes header
 *
 * \author Copyright (C) 2015-2017 Kamil Szczygiel http://www.distortec.com http://www.freddiechopin.info
 *
 * \par License
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDE_DISTORTOS_STATICRAWMESSAGEQUEUE_HPP_
#define INCLUDE_DISTORTOS_STATICRAWMESSAGEQUEUE_HPP_

#include "distortos/RawMessageQueue.hpp"

#include "distortos/internal/memory/dummyDeleter.hpp"

namespace distortos
{

/**
 * \brief StaticRawMessageQueue2 class is a variant of RawMessageQueue that has automatic storage for queue's contents.
 *
 * This class is a replacement for StaticRawMessageQueue and StaticRawMessageQueueFromSize. To use this new API modify
 * your code in following way:
 * - `'StaticRawMessageQueue<T, QueueSize>'` -> `'StaticRawMessageQueue2<sizeof(T), QueueSize>'`;
 * - `'StaticRawMessageQueueFromSize<ElementSize, QueueSize>'` -> `'StaticRawMessageQueue2<ElementSize, QueueSize>'`;
 *
 * Transition schedule:
 * 1. v0.5.0 - `StaticRawMessageQueue<T, QueueSize>` and `StaticRawMessageQueueFromSize<ElementSize, QueueSize>` are
 * converted to deprecated aliases to `StaticRawMessageQueue2<ElementSize, QueueSize>`;
 * 2. v0.6.0 - "old" `StaticRawMessageQueue<T, QueueSize>` and `StaticRawMessageQueueFromSize<ElementSize, QueueSize>`
 * aliases are removed, `StaticRawMessageQueue2<ElementSize, QueueSize>` is renamed to
 * `StaticRawMessageQueue<ElementSize, QueueSize>`, deprecated `StaticRawMessageQueue2<ElementSize, QueueSize>` alias is
 * added;
 * 3. v0.7.0 - deprecated `StaticRawMessageQueue2<ElementSize, QueueSize>` alias is removed;
 *
 * \tparam ElementSize is the size of single queue element, bytes
 * \tparam QueueSize is the maximum number of elements in queue
 *
 * \ingroup queues
 */

template<size_t ElementSize, size_t QueueSize>
class StaticRawMessageQueue2 : public RawMessageQueue
{
public:

	/**
	 * \brief StaticRawMessageQueue2's constructor
	 */

	explicit StaticRawMessageQueue2() :
			RawMessageQueue{{entryStorage_.data(), internal::dummyDeleter<EntryStorage>},
					{valueStorage_.data(), internal::dummyDeleter<uint8_t>}, ElementSize, QueueSize}
	{

	}

private:

	/// storage for queue's entries
	std::array<EntryStorage, QueueSize> entryStorage_;

	/// storage for queue's contents
	std::array<uint8_t, ElementSize * QueueSize> valueStorage_;
};

/**
 * \brief StaticRawMessageQueue class is a variant of RawMessageQueue that has automatic storage for queue's contents.
 *
 * \tparam T is the type of data in queue
 * \tparam QueueSize is the maximum number of elements in queue
 *
 * \ingroup queues
 */

template<typename T, size_t QueueSize>
using StaticRawMessageQueue = StaticRawMessageQueue2<sizeof(T), QueueSize>;

/**
 * \brief StaticRawMessageQueueFromSize type alias is a variant of StaticRawMessageQueue which uses size of element
 * (instead of type) as template argument.
 *
 * \tparam ElementSize is the size of single queue element, bytes
 * \tparam QueueSize is the maximum number of elements in queue
 */

template<size_t ElementSize, size_t QueueSize>
using StaticRawMessageQueueFromSize = StaticRawMessageQueue2<ElementSize, QueueSize>;

}	// namespace distortos

#endif	// INCLUDE_DISTORTOS_STATICRAWMESSAGEQUEUE_HPP_
