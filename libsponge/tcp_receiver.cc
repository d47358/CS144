#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    //DUMMY_CODE(seg);
    if(seg.header().syn&&!_syn_flag){
        //if(!_syn_flag){
            _isn=seg.header().seqno;
            _syn_flag=true;
            //if(seg.length_in_sequence_space()-1==0)
            //    return;
        //}
        //return;
    }
    if(_syn_flag&&(seg.payload().str().size()>0||seg.header().fin)){
        //if(_fin_flag&&seg.header().fin) return;
        _fin_flag=seg.header().fin;
        //size_t leng=seg.payload().str().size();
        //if(leng>0){
            uint64_t abs_idx=unwrap(seg.header().seqno+(seg.header().syn?1:0),_isn,_recvd_bytes);
            _reassembler.push_substring(seg.payload().copy(),abs_idx-1,seg.header().fin);
            _recvd_bytes=_reassembler.get_head();
        //}
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const { 
    if(!_syn_flag)return nullopt;
    uint64_t temp=_recvd_bytes+(_syn_flag?1:0)+(_reassembler.stream_out().input_ended()?1:0);
    //uint64_t temp=_recvd_bytes+(_syn_flag?1:0)+(_fin_flag?1:0);
    return wrap(temp,_isn);
 }

size_t TCPReceiver::window_size() const { 
    return _capacity-_reassembler.stream_out().buffer_size();
 }
