#ifndef TYPE_TRAITS_IS_CONTAINER_H_
#define TYPE_TRAITS_IS_CONTAINER_H_

#include <typeinfo>
#include <map>
#include <string>
#include <list>
#include <vector>
#include <set>

// Basic is_container template; specialize to derive from std::true_type for all desired container types
template<typename T> struct is_container : public std::false_type { };

// Mark vector as a container
template<typename T, typename TAllocator> struct is_container<std::vector<T, TAllocator> > : public std::true_type{};

// Mark list as a container
template<typename T, typename TAllocator> struct is_container<std::list<T, TAllocator> > : public std::true_type{};

// Mark set as a container
template<typename T, typename TTraits, typename TAllocator> struct is_container<std::set<T, TTraits, TAllocator> > : public std::true_type{};

// Mark map as a container
template<typename TKey, typename TValue, typename TTraits, typename TAllocator> struct is_container<std::map<TKey, TValue, TTraits, TAllocator> > : public std::true_type{};


#endif  //! #ifndef TYPE_TRAITS_IS_CONTAINER_H_
