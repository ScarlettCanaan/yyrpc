#ifndef __BOOST_INDEX_SEQUENCE_H_
#define __BOOST_INDEX_SEQUENCE_H_

namespace __boost {

template< std::size_t ... I >
struct index_sequence {
    using type = index_sequence;
    using value_type = std::size_t;
    static std::size_t size() {
        return sizeof ... (I);
    }
};

template< typename Seq1, typename Seq2 >
struct concat_sequence;

template< std::size_t ... I1, std::size_t ... I2 >
struct concat_sequence< index_sequence< I1 ... >, index_sequence< I2 ... > > : public index_sequence< I1 ..., (sizeof ... (I1)+I2) ... > {
};

template< std::size_t I >
struct make_index_sequence : public concat_sequence< typename make_index_sequence< I/2 >::type,
                                                     typename make_index_sequence< I-I/2 >::type > {
};

template<>
struct make_index_sequence< 0 > : public index_sequence<> {
};
template<>
struct make_index_sequence< 1 > : public index_sequence< 0 > {
};

template< typename ... T >
using index_sequence_for = make_index_sequence< sizeof ... (T) >;

}

#endif  //! #ifndef __BOOST_INDEX_SEQUENCE_H_

