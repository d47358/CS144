#ifndef SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH
#define SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH

#include "byte_stream.hh"

#include <cstdint>
#include <string>
#include<set>
#include<algorithm>
#include<memory>
//! \brief A class that assembles a series of excerpts from a byte stream (possibly out of order,
//! possibly overlapping) into an in-order byte stream.
class StreamReassembler {
  private:
    // Your code here -- add private members as necessary.
    struct segment{
      std::string _data;
      size_t _size;
      size_t _begin;
      bool operator<(const segment& t){return _begin<t._begin;}
      segment(const std::string &data):_data(data),_size(data.size()),_begin(0){}
      friend bool operator<(const std::shared_ptr<segment>t1,const std::shared_ptr<segment>t2){return t1->_begin<t2->_begin;}
    };
    /*
    struct cmp{
      bool operator()(segment* t1,segment* t2){
        return t1->_begin<t2->_begin;
      }
    };
    */
    std::set<std::shared_ptr<segment>>_segments{};
    size_t _head{0};
    size_t _unassembled_bytes{0};
    ByteStream _output;  //!< The reassembled in-order byte stream
    size_t _capacity;    //!< The maximum number of bytes
    bool _EOF{false};
    long merge(std::shared_ptr<segment> t1,std::shared_ptr<segment> t2);
    long merge_pre(std::shared_ptr<segment> t1,std::shared_ptr<segment> t2);
  public:
    //! \brief Construct a `StreamReassembler` that will store up to `capacity` bytes.
    //! \note This capacity limits both the bytes that have been reassembled,
    //! and those that have not yet been reassembled.
    StreamReassembler(const size_t capacity);

    //! \brief Receive a substring and write any newly contiguous bytes into the stream.
    //!
    //! The StreamReassembler will stay within the memory limits of the `capacity`.
    //! Bytes that would exceed the capacity are silently discarded.
    //!
    //! \param data the substring
    //! \param index indicates the index (place in sequence) of the first byte in `data`
    //! \param eof the last byte of `data` will be the last byte in the entire stream
    void push_substring(const std::string &data, const uint64_t index, const bool eof);

    //! \name Access the reassembled byte stream
    //!@{
    const ByteStream &stream_out() const { return _output; }
    ByteStream &stream_out() { return _output; }
    //!@}

    //! The number of bytes in the substrings stored but not yet reassembled
    //!
    //! \note If the byte at a particular index has been pushed more than once, it
    //! should only be counted once for the purpose of this function.
    size_t unassembled_bytes() const;

    //! \brief Is the internal state empty (other than the output stream)?
    //! \returns `true` if no substrings are waiting to be assembled
    bool empty() const;
    size_t get_head() const { return _head; }
};

#endif  // SPONGE_LIBSPONGE_STREAM_REASSEMBLER_HH