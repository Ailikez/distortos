#
# file: distortos-sources.cmake
#
# author: Copyright (C) 2018 Kamil Szczygiel http://www.distortec.com http://www.freddiechopin.info
#
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
# distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

if(CONFIG_CHIP_STM32_SPIV1)

	target_sources(distortos PRIVATE
			${CMAKE_CURRENT_LIST_DIR}/STM32-SPIv1-ChipSpiMasterLowLevel.cpp
			${CMAKE_CURRENT_LIST_DIR}/STM32-SPIv1-spiLowLevelInitializer.cpp
			${CMAKE_CURRENT_LIST_DIR}/STM32-SPIv1-spis.cpp)

	doxygen(INPUT ${CMAKE_CURRENT_LIST_DIR} INCLUDE_PATH ${CMAKE_CURRENT_LIST_DIR}/include)

endif(CONFIG_CHIP_STM32_SPIV1)
