#include "buffer_helper.hpp"


BufferHelper::BufferHelper(void* buffer_ptr, size_t buffer_size, size_t full_threshold, uint8_t buffer_id):
    buffer_size{buffer_size},
    tail_index{0},
    head_index{0},
    status{0},
    is_overflow{false},
    is_read_garbage{false},
    buffer_ptr{buffer_ptr},
    buffer_id{buffer_id},
    full_threshold{full_threshold}
{}




size_t BufferHelper::getWriteableCount(size_t data_count) {
    // if can write all data, return data_count
    if( buffer_size - tail_index >= data_count ) {
        return data_count;
    // else return left space
    }else {
        return buffer_size - tail_index;
    }
}




size_t BufferHelper::getReadableCount(size_t data_count) {
    // if has enough data, return data_count
    if( tail_index - head_index >= data_count ) {
        return data_count;
    // else return left data count
    }else {
        return tail_index - head_index;
    }
}




size_t BufferHelper::getWriteIndex() {
    // if can write, return head_index
    if(tail_index < buffer_size) {
        return tail_index;
    // else return 65535
    }else {
        return 65535;
    }
}




size_t BufferHelper::getReadIndex() {
    // if can read, return head_index
    if(head_index < tail_index) {
        return head_index;
    // else return 65535
    }else {
        return 65535;
    }
}




void BufferHelper::markWriteCount(size_t data_count) {
    // if not overflow, increase tail_index
    if( buffer_size - tail_index >= data_count ) {
        tail_index += data_count;
        if(tail_index >= full_threshold) {
            setFlagFull();
        }
    // else clear buffer and mark overflow true
    }else {
        is_overflow = true;
        setFlagEmpty();
    }
}




void BufferHelper::markReadCount(size_t data_count) {
    // if not read garbage, increase head_index
    if( tail_index - head_index >= data_count ) {
        head_index += data_count;
        if(head_index == tail_index) {
            setFlagEmpty();
        }
    // else mark read garbage
    }else {
        is_read_garbage = true;
    }
}





void BufferHelper::setFlagEmpty() {
    tail_index = 0;
    head_index = 0;
    status = 0;
}




void BufferHelper::setFlagWriting() {
    status = 1;
}




void BufferHelper::setFlagFull() {
    status = 2;
}




void BufferHelper::setFlagReading() {
    status = 3;
}




bool BufferHelper::isFull() {
    return (status == 2)? true : false;
}




bool BufferHelper::isEmpty(){
    return (status == 0)? true : false;
}




bool BufferHelper::isWriting() {
    return (status == 1)? true : false;
}




bool BufferHelper::isReading(){
    return (status == 3)? true : false;
}