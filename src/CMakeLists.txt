set(LIBRARY_NAME yyrpc)

set(SRC_GRP_TYPETRAITS
  type_traits/is_container.h
  type_traits/function_traits.h
)

set(SRC_GRP_JSON
  json/convert_def.h
  json/msgpack2json.h
  json/msgpack2json.cpp
  json/json2msgpack.h
  json/json2msgpack.cpp
  json/param2msgpack.h
  json/param2msgpack.cpp
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
  listenpoint/listenpoint_manager.h
  listenpoint/listenpoint_manager.cpp
)

set(SRC_GRP_TRANSPORT
  transport/protocal_def.h
  transport/rpc_tcp_server_transport.h
  transport/rpc_tcp_server_transport.cpp
  transport/rpc_tcp_client_transport.h
  transport/rpc_tcp_client_transport.cpp
  transport/transport.h
  transport/tcp_server_transport.cpp
  transport/tcp_server_transport.h
  transport/tcp_client_transport.h
  transport/tcp_client_transport.cpp
  transport/tcp_sync_client_transport.h
  transport/tcp_sync_client_transport.cpp
)

set(SRC_GRP_ACCEPTOR
  acceptor/acceptor.h
  acceptor/tcp_acceptor.h
  acceptor/tcp_acceptor.cpp
  acceptor/rpc_client_accept.h
  acceptor/rpc_client_accept.cpp
)

set(SRC_GRP_UTIL
  util/base_util.h
  util/base_util.cpp
  util/ini.hpp
  util/thread_safe_list.h
  util/fixed_memory_pool.h
)

set(SRC_GRP_THREAD
  thread/thread_worker.h
  thread/thread_worker.cpp
)

set(SRC_GRP_PACKET
  packet/packet.cpp
  packet/packet.h
  packet/tcp_packet_callback.h
  packet/tcp_packet_decoder.cpp
  packet/tcp_packet_decoder.h
)

set(SRC_GRP_STUB
  stub/call_result.h
  stub/call_result.inl
  stub/callback.h
  stub/callback.inl
  stub/caller_manager.h
  stub/caller_manager.cpp
  stub/fiber_caller_manager.h
  stub/fiber_caller_manager.cpp
  stub/callee_manager.h
  stub/callee_manager.cpp
  stub/fiber_callee_manager.h
  stub/fiber_callee_manager.cpp
  stub/connection_monitor.h
  stub/connection_monitor.cpp
  stub/method_auth.h
  stub/method_auth.cpp
)

set(SRC_GRP_MARSHAL
  marshal/serialization.h
  marshal/serialization.inl
  marshal/serialization.cpp
  marshal/unserialization.h
  marshal/unserialization.inl
  marshal/unserialization.cpp
)

set(LIBRARY_SRCS
  orpc.h
  rpc_application.h
  rpc_application.cpp
  error_def.h
  error_def.cpp
  build_config.h
  method.h
  method.inl
  event.h
  event.inl
  property.h
  id_generator.h
  id_generator.cpp
  index_sequence.hpp
  buff_stream.h
  buff_stream.inl
  ${SRC_GRP_TYPETRAITS}
  ${SRC_GRP_WORKER}
  ${SRC_GRP_ENDPOINT}
  ${SRC_GRP_TRANSPORT}
  ${SRC_GRP_LISTENPOINT}
  ${SRC_GRP_JSON}
  ${SRC_GRP_STUB}
  ${SRC_GRP_MARSHAL}
  ${SRC_GRP_PACKET}
  ${SRC_GRP_UTIL}
  ${SRC_GRP_THREAD}
  ${SRC_GRP_ACCEPTOR}
)

source_group("thread"
  FILES
    ${SRC_GRP_THREAD}
)

source_group("acceptor"
  FILES
    ${SRC_GRP_ACCEPTOR}
)

source_group("packet"
  FILES
    ${SRC_GRP_PACKET}
)

source_group("utilities"
  FILES
    ${SRC_GRP_UTIL}
)

source_group("stub"
  FILES
    ${SRC_GRP_STUB}
)

source_group("marshal"
  FILES
    ${SRC_GRP_MARSHAL}
)

source_group("type_traits"
  FILES
    ${SRC_GRP_TYPETRAITS}
)

source_group("json"
  FILES
    ${SRC_GRP_JSON}
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
  ${LIBUV_INCLUDE_DIR}
  ${GLOG_INCLUDE_DIR}
  ${GFLAGS_INCLUDE_DIR}
  ${BREAKPAD_INCLUDE_DIR}
  ${BOOST_INCLUDE_DIR}
  ${MSGPACK_INCLUDE_DIR}
  ${ST_INCLUDE_DIR}
  ${HTTP_PARSER_INCLUDE_DIR}
  ${RAPIDJSON_INCLUDE_DIR}
  ${BASE64_INCLUDE_DIR}
)

if(WIN32)
  include_directories(
  ${GETOPT_INCLUDE_DIRS}
  )
endif()

add_library(${LIBRARY_NAME} STATIC
  ${LIBRARY_SRCS}
)
