set(LIBRARY_NAME yyrpc)

set(SRC_GRP_TYPETRAITS
  type_traits/is_container.h
  type_traits/function_traits.h
)

set(SRC_GRP_WORKER
  worker/client_worker.h
  worker/client_worker.cpp
  worker/client_worker_pool.h
  worker/client_worker_pool.cpp
  worker/server_worker.h
  worker/server_worker.cpp
  worker/server_worker_pool.h
  worker/server_worker_pool.cpp
)

set(SRC_GRP_ENDPOINT
  endpoint/endpoint.h
  endpoint/endpoint.cpp
  endpoint/endpoint_manager.h
  endpoint/endpoint_manager.cpp
)

set(SRC_GRP_LISTENPOINT
  listenpoint/listenpoint.h
  listenpoint/listenpoint.cpp
  listenpoint/rpc_client_accept.h
  listenpoint/rpc_client_accept.cpp
  listenpoint/listenpoint_manager.h
  listenpoint/listenpoint_manager.cpp
)

set(SRC_GRP_TRANSPORT
  transport/protocal_def.h
  transport/rpc_tcp_server_transport.h
  transport/rpc_tcp_server_transport.cpp
  transport/rpc_tcp_client_transport.h
  transport/rpc_tcp_client_transport.cpp
)

set(LIBRARY_SRCS
  yyrpc.h
  yyrpc.cpp
  method.h
  property.h
  serialization.h
  unserialization.h
  async_result.h
  id_generator.h
  id_generator.cpp
  callback.h
  caller_manager.h
  caller_manager.cpp
  callee_manager.h
  callee_manager.cpp
  index_sequence.hpp
  ${SRC_GRP_TYPETRAITS}
  ${SRC_GRP_WORKER}
  ${SRC_GRP_ENDPOINT}
  ${SRC_GRP_TRANSPORT}
  ${SRC_GRP_LISTENPOINT}
)

source_group("type_traits"
  FILES
    ${SRC_GRP_TYPETRAITS}
)

source_group("worker"
  FILES
    ${SRC_GRP_WORKER}
)

source_group("endpoint"
  FILES
    ${SRC_GRP_ENDPOINT}
)

source_group("transport"
  FILES
    ${SRC_GRP_TRANSPORT}
)

source_group("listenpoint"
  FILES
    ${SRC_GRP_LISTENPOINT}
)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/uv_util
  ${LIBUV_INCLUDE_DIR}
  ${GLOG_INCLUDE_DIR}
  ${GFLAGS_INCLUDE_DIR}
  ${BREAKPAD_INCLUDE_DIR}
  ${BOOST_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/src/yyrpc/src
  ${MSGPACK_INCLUDE_DIR}
)

add_library(${LIBRARY_NAME} STATIC
  ${LIBRARY_SRCS}
)
