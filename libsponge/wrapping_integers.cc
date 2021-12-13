#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    //DUMMY_CODE(n, isn);
    uint32_t t=(n%(1ul<<32)+isn.raw_value())%(1ul<<32);
    return WrappingInt32{t};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    //DUMMY_CODE(n, isn, checkpoint);
    uint64_t num=1ul<<32;
    uint64_t offset=n.raw_value()>isn.raw_value()?n.raw_value()-isn.raw_value():num-isn.raw_value()+n.raw_value();
    uint64_t temp=(checkpoint&0xFFFFFFFF00000000)+offset;
    uint64_t ans=temp;
    if(abs(int64_t(temp+num-checkpoint))<abs(int64_t(temp-checkpoint)))
        ans=temp+num;
    if(temp>=num&&(abs(int64_t(temp-num-checkpoint))<abs(int64_t(temp-checkpoint))))
        ans=temp-num;
    return ans;
}
