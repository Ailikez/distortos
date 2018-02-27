#
# file: distortos-sources.cmake
#
# author: Copyright (C) 2018 Kamil Szczygiel http://www.distortec.com http://www.freddiechopin.info
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
# distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

if(CONFIG_ARCHITECTURE_ARMV6_M OR CONFIG_ARCHITECTURE_ARMV7_M)

	target_sources(distortos PRIVATE
			${CMAKE_CURRENT_LIST_DIR}/ARMv6-M-ARMv7-M-architectureLowLevelInitializer.cpp
			${CMAKE_CURRENT_LIST_DIR}/ARMv6-M-ARMv7-M-coreVectors.cpp
			${CMAKE_CURRENT_LIST_DIR}/ARMv6-M-ARMv7-M-disableInterruptMasking.cpp
			${CMAKE_CURRENT_LIST_DIR}/ARMv6-M-ARMv7-M-enableInterruptMasking.cpp
			${CMAKE_CURRENT_LIST_DIR}/ARMv6-M-ARMv7-M-getMainStack.cpp
			${CMAKE_CURRENT_LIST_DIR}/ARMv6-M-ARMv7-M-initializeStack.cpp
			${CMAKE_CURRENT_LIST_DIR}/ARMv6-M-ARMv7-M-isInInterruptContext.cpp
			${CMAKE_CURRENT_LIST_DIR}/ARMv6-M-ARMv7-M-PendSV_Handler.cpp
			${CMAKE_CURRENT_LIST_DIR}/ARMv6-M-ARMv7-M-requestContextSwitch.cpp
			${CMAKE_CURRENT_LIST_DIR}/ARMv6-M-ARMv7-M-requestFunctionExecution.cpp
			${CMAKE_CURRENT_LIST_DIR}/ARMv6-M-ARMv7-M-Reset_Handler.cpp
			${CMAKE_CURRENT_LIST_DIR}/ARMv6-M-ARMv7-M-restoreInterruptMasking.cpp
			${CMAKE_CURRENT_LIST_DIR}/ARMv6-M-ARMv7-M-startScheduling.cpp
			${CMAKE_CURRENT_LIST_DIR}/ARMv6-M-ARMv7-M-supervisorCall.cpp
			${CMAKE_CURRENT_LIST_DIR}/ARMv6-M-ARMv7-M-SVC_Handler.cpp
			${CMAKE_CURRENT_LIST_DIR}/ARMv6-M-ARMv7-M-SysTick_Handler.cpp)

endif()