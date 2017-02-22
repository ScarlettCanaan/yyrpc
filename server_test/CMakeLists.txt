set(EXECUTABLE_NAME yyrpc_server_test)

set(EXECUTABLE_SRCS
  ../proto/common_api.h
  ../proto/test_api.h
  server.h
  server.cpp
  main.cpp
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/uv_util
  ${LIBUV_INCLUDE_DIR}
  ${GLOG_INCLUDE_DIR}
  ${GFLAGS_INCLUDE_DIR}
  ${BREAKPAD_INCLUDE_DIR}
  ${BOOST_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/src/yyrpc/proto
  ${CMAKE_SOURCE_DIR}/src/yyrpc/src
  ${MSGPACK_INCLUDE_DIR}
)

add_executable(${EXECUTABLE_NAME}
  ${EXECUTABLE_SRCS}
)

add_dependencies(${EXECUTABLE_NAME} yyrpc uv_util)

target_link_libraries(${EXECUTABLE_NAME}
  yyrpc
  uv_util
)

target_link_libraries(${EXECUTABLE_NAME}
  ${GLOG_LIBRARIES}
  ${GFLAGS_LIBRARIES}
  ${LIBUV_LIBRARIES}
  ${BREAKPAD_LIBRARIES}
)

if(WIN32)

target_link_libraries(${EXECUTABLE_NAME}
  advapi32
  iphlpapi
  shell32
  user32
)

elseif(UNIX)

target_link_libraries(${EXECUTABLE_NAME}
  m 
  dl
)

endif()

if(UNIX)
  set(EXECUTABLE_LINK_FLAGS "-pthread ${EXECUTABLE_LINK_FLAGS} -rdynamic")
endif()

if(APPLE)
  set(EXECUTABLE_LINK_FLAGS "-framework Carbon ${EXECUTABLE_LINK_FLAGS}")
endif()

set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS
  "${EXECUTABLE_LINK_FLAGS}"
)