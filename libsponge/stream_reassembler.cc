#include "stream_reassembler.hh"
#include<iostream>
// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.

long StreamReassembler::merge(shared_ptr<segment> t1,shared_ptr<segment> t2){
    if(t1->_begin>t2->_begin){
        t1.swap(t2);
    }
    long temp=t1->_begin-t2->_begin+t1->_size;
    if(temp<=0){
        return -1;
    }
    if(size_t(temp)>=t2->_size){
        return t2->_size;
    }
    //cout<<temp<<endl<<"t2->_data.size()"<<t2->_data.size()<<endl<<"t2->_size"<<t2->_size<<endl;
    t1->_data+=t2->_data.substr(temp);
    t1->_size=t1->_data.size();
    return temp;
    
}
/*
long StreamReassembler::merge_pre(shared_ptr<segment> t1,shared_ptr<segment> t2){
    long temp=t1->_begin+t1->_size-t2->_begin;
    if(temp<=0){
        return -1;
    }
    else if(t1->_begin+t1->_size>=t2->_begin+t2->_size+1){
        return t2->_size;
    }
    else{
        t2->_data=t1->_data+t2->_data.substr(t2->_size-temp);
        t2->_size=t2->_data.size();
        t2->_begin=t1->_begin;
        return temp;
    }
}
*/
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    //cout<<"index"<<index<<"data_size"<<data.size()<<"head"<<_head<<endl;
    if(index>=_head+_capacity){
        return;
    }
    bool flag = true;
    if(index+data.size()<=_head){
        if(eof&&empty()){
            _output.end_input();
        }
        return;
    }
    if(!data.empty()){
        shared_ptr<segment>node(new segment(data));
        node->_begin=index;
        if(index<_head){
            size_t offset=_head-index;
            //node->_data.assign(data.begin()+offset,data.end());
            node->_data=data.substr(offset);
            node->_begin=_head;
            node->_size=node->_data.size();
    }
        if(node->_begin+node->_size>_head+_output.remaining_capacity()){
            flag=false;
    }
        _unassembled_bytes+=node->_size;

        long merged;
        auto it=lower_bound(_segments.begin(),_segments.end(),node,[&](shared_ptr<segment> t1,shared_ptr<segment> t2){return *t1<*t2;});
        auto pre=_segments.end();
        if(it!=_segments.begin())    
            pre=prev(it,1);
        while(it!=_segments.end()&&(merged=merge(node,*it))>0){
                //cout<<"index"<<(*it)->_begin<<"merged"<<merged<<endl;
                _unassembled_bytes-=merged;
                shared_ptr<segment>temp=*it;
                it++;
                _segments.erase(temp);
    }
            /*
                while(pre!=_segments.end()&&(merged=merge_pre(*pre,node))>=0){
                    _unassembled_bytes-=merged;
                    shared_ptr<segment>temp=*pre;
                    
                    
                    if (pre == _segments.begin()) {
                        break;
                    }
                    pre--;
                    _segments.erase(temp);
                }
            */
            if(pre!=_segments.end()){
                merged=merge(*pre,node);
                if(merged>0){
                    _unassembled_bytes-=merged;
                }
            }
        _segments.insert(node);
    }

    while(!_segments.empty()&&(*_segments.begin())->_begin==_head){
        size_t written=_output.write((*_segments.begin())->_data);
        _head+=written;
        _unassembled_bytes-=written;
        _segments.erase(_segments.begin());
    }

    if(flag&&eof) _EOF=true;
    //if(index+data.size()>_head+_output.remaining_capacity()) _EOF=false;
    if(_EOF&&empty()){
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled_bytes==0; }
