#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) 
    , _Timer(retx_timeout){}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {
    if(!_syn_flag){
        TCPSegment seg;
        seg.header().syn=true;
        send_segment(seg);
        _syn_flag=true;
        return;
    }
    if (!_segments_outstanding.empty() && _segments_outstanding.front().header().syn)
        return;
    if (!_stream.buffer_size() && !_stream.eof())
        return;
    if(_fin_flag) return;
    if(_window_size){
        size_t capacity;
        while(_window_size>_bytes_in_flight&&!_fin_flag){
            capacity=_window_size-_bytes_in_flight;
            TCPSegment seg;
            string text=_stream.read(min(capacity,TCPConfig::MAX_PAYLOAD_SIZE));
            seg.payload()=Buffer(move(text));
            if(_stream.eof()&&_window_size>seg.length_in_sequence_space()){
                seg.header().fin=true;
                _fin_flag=true;
            }
            send_segment(seg);
            if(_stream.buffer_empty()){
                break;
            }
            
        }
    }else{
        if(_bytes_in_flight==0){
            TCPSegment seg;
        if(_stream.eof()){
            seg.header().fin=true;
            _fin_flag=true;
        }else{
            string text=_stream.read(1);
            seg.payload()=Buffer(move(text));
        }
        send_segment(seg);
        }
        
    }
    
    

}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    //DUMMY_CODE(ackno, window_size); 
    uint64_t abs_ackno=unwrap(ackno,_isn,_next_seqno);
    _window_size=window_size;
    if(abs_ackno>_next_seqno||abs_ackno<=_recvd_ackno) return;
    _recvd_ackno=abs_ackno;
    while(!_segments_outstanding.empty()){
        TCPSegment seg=_segments_outstanding.front();
        if(unwrap(seg.header().seqno,_isn,_next_seqno)+seg.length_in_sequence_space()<=abs_ackno){
            _bytes_in_flight-=seg.length_in_sequence_space();
            _segments_outstanding.pop();
        }else break;
    }
    
    _Timer.reset(_initial_retransmission_timeout);
    _consecutive_retransmissions=0;
    if(!_segments_outstanding.empty()) _Timer.start();
    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    //DUMMY_CODE(ms_since_last_tick);
    if(!_Timer.is_running()) return;
    if(_Timer.decline(ms_since_last_tick)&&!_segments_outstanding.empty()){
        _consecutive_retransmissions++;
        if(_window_size>=_bytes_in_flight||_segments_outstanding.front().header().syn)
            _Timer.powtimer();
        else 
            _Timer.resume();
        _Timer.start();
        _segments_out.push(_segments_outstanding.front());
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(seg);
}

void TCPSender::send_segment(TCPSegment& seg){
    seg.header().seqno=wrap(_next_seqno,_isn);
    _bytes_in_flight+=seg.length_in_sequence_space();
    _next_seqno+=seg.length_in_sequence_space();
    _segments_out.push(seg);
    _segments_outstanding.push(seg);
    if(!_Timer.is_running()){
        _Timer.start();
    }
}