cmake_minimum_required(VERSION 3.0)

set(EXEC_NAME lslpub_LabJack)

project(${EXEC_NAME})

set (CMAKE_CXX_STANDARD 11)

#get the sources and headers
file(GLOB SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "src/*.c")
file(GLOB HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h" "include/*.hpp")

#add the include libraries directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)

#create our exec file
add_executable(${EXEC_NAME} ${SRCS} ${HEADERS})

#link LSL libraries
if(UNIX)
	target_link_libraries (${EXEC_NAME} lsl64)
	ADD_CUSTOM_COMMAND (	TARGET ${EXEC_NAME} POST_BUILD
						#copy the dll files 
						COMMENT "Saving exec in bin/ directory..." VERBATIM
						COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/bin/UNIX/${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_VERSION}
						COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/${EXEC_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/bin/UNIX/${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_VERSION}
						)
endif(UNIX)

if(WIN32)
	target_link_libraries (${EXEC_NAME} liblsl64)
	target_link_libraries (${EXEC_NAME} wsock32 ws2_32)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	  # using Clang
	  ADD_CUSTOM_COMMAND (	TARGET ${EXEC_NAME} POST_BUILD
							#copy the dll files 
							COMMENT "Copying LSL binaries..." VERBATIM
							COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/liblsl64.dll ${CMAKE_BINARY_DIR}/Release
							#copy the dll files 
							COMMENT "Saving exec in bin/ directory..." VERBATIM
							COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/bin/WIN32/${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_VERSION}
							COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/Release ${CMAKE_CURRENT_SOURCE_DIR}/bin/WIN32/${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_VERSION}
							)
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	  # using GCC
	  ADD_CUSTOM_COMMAND (	TARGET ${EXEC_NAME} POST_BUILD
							#copy the dll files 
							COMMENT "Saving exec in bin/ directory..." VERBATIM
							COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/bin/WIN32/${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_VERSION}
							COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/lib/liblsl64.dll ${CMAKE_BINARY_DIR}/
							COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/${EXEC_NAME}.exe ${CMAKE_CURRENT_SOURCE_DIR}/bin/WIN32/${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_VERSION}
							COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/liblsl64.dll ${CMAKE_CURRENT_SOURCE_DIR}/bin/WIN32/${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_VERSION}
							)
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	  # using Intel C++
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	  # using Visual Studio C++
	endif()
  

endif(WIN32)


