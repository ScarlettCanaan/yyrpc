#include <msgpack.hpp>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>

#define LIST_SIZE 3

struct MapRequest {
    using maptype = std::map<std::string, std::vector<std::string>>;
    int id_;
    std::string initialName;
    std::map<std::string, std::vector<std::string>> map_;
    bool operator==(MapRequest const &obj) {
        if (id_ != obj.id_)
            return false;
        if (initialName.compare(obj.initialName) != 0)
            return false;
        return true;
    }
    MSGPACK_DEFINE(id_, map_);
}; 

namespace msgpack {
    MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
        namespace adaptor {
            template<>
            struct as<MapRequest> {
                MapRequest operator()(msgpack::object const& o) const {
                    if (o.type != msgpack::type::ARRAY) throw msgpack::type_error();
                    if (o.via.array.size != 1) throw msgpack::type_error();
                    MapRequest mr;
                    mr.id_ = o.via.array.ptr[0].as<int>();
                    mr.initialName = o.via.array.ptr[1].as<std::string>();
                    mr.map_ = o.via.array.ptr[2].as<MapRequest::maptype>();
                    return mr;
                }
            };
        }
    }
}
int main() {
    MapRequest my_MapReq1, my_MapReq2;
    std::map<std::string, std::vector<std::string>> my_map1, my_map2; 
    std::string key1 = "Ada";
    std::string key2 = "Bob";
    std::vector<std::string> value1, value2;
    std::pair<std::string, std::vector<std::string>> pair1, pair2;

    for (int i = 0; i < LIST_SIZE; ++i) {
        std::string s1 = std::to_string(i);
        std::string s2 = std::to_string(LIST_SIZE - i);
        value1.push_back(s1);
        value2.push_back(s2);
    }
    pair1.first = key1;
    pair2.first = key2;
    pair1.second = value1;
    pair2.second = value2;
    
    my_map1.insert(pair1);
    my_map2.insert(pair2);
    my_MapReq1.id_ = 1;
    my_MapReq2.id_ = 2;
    my_MapReq1.initialName = "A";
    my_MapReq2.initialName = "B";
    my_MapReq1.map_ = my_map1;
    my_MapReq2.map_ = my_map2;

    msgpack::zone z;
    msgpack::object obj1(my_map1, z);
    msgpack::object obj2(my_map2, z);
    std::cout << "Object builder:" << std::endl;
    std::cout << obj1 << std::endl;
    std::cout << obj2 << std::endl;
    //auto mr1 = obj1.as<MapRequest>();
    //auto mr2 = obj2.as<MapRequest>();
    //assert(mr1 == obj1);
    //assert(mr2 == obj2);
    std::stringstream ss;
    msgpack::pack(ss, my_map1);
    msgpack::pack(ss, my_map2);
    std::size_t offset = 0;
    msgpack::object_handle oh1 = msgpack::unpack(ss.str().data(), ss.str().size(), offset);
    msgpack::object_handle oh2 = msgpack::unpack(ss.str().data(), ss.str().size(), offset);
    msgpack::object packobj1 = oh1.get();
    msgpack::object packobj2 = oh2.get();
    std::cout << "pack builder:" << std::endl;
    std::cout << packobj1 << std::endl;
    std::cout << packobj2 << std::endl;

    int crashNum = 0;
    msgpack::pack(ss, crashNum);
    msgpack::object_handle oh = msgpack::unpack(ss.str().data(), ss.str().size(), 0);
    msgpack::object crashobj = oh.get();
    double d = crashobj.as<double>();
    std::cout << "d:" << d << std::endl;
    MapRequest mr = crashobj.as<MapRequest>();
    //auto mr3 = packobj1.as<MapRequest>();
    //auto mr4 = packobj2.as<MapRequest>();
    //assert(mr3 == obj1);
    //assert(mr4 == obj2);
}
