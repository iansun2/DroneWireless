#ifndef __BUFFER_HELPER_HPP__
#define __BUFFER_HELPER_HPP__

#include <Arduino.h>



class BufferHelper{
private:
    size_t buffer_size;
    size_t full_threshold;
    size_t tail_index;
    size_t head_index;
    // 0: empty (read finish)
    // 1: writing
    // 2: full (write finish)
    // 3: reading
    uint8_t status;

public:

    uint8_t buffer_id;
    void* buffer_ptr;

    bool is_overflow;
    bool is_read_garbage;

    BufferHelper(void* buffer_ptr, size_t buffer_size, size_t full_threshold, uint8_t buffer_id);
    ~BufferHelper(){};


    // input data count that you want to write
    // if can write all data, return input len
    // if need split, return writeable len
    size_t getWriteableCount(size_t data_count);

    // input data count that you want to read
    // if has enough data, return input len
    // if data enough, return all data count in the buffer
    size_t getReadableCount(size_t data_count);


    // if can write, return write start index
    // else return 65535 eg: full
    size_t getWriteIndex();

    // if can read, return read start index
    // else return 65535 eg: empty
    size_t getReadIndex();


    // input written data count
    void markWriteCount(size_t data_count);

    // input read data count
    void markReadCount(size_t data_count);
    

    // mark status empty
    // reset head, tail index
    void setFlagEmpty();

    // mark status writing
    void setFlagWriting();

    // mark status writing
    void setFlagFull();

    // mark status reading
    void setFlagReading();


    bool isFull();

    bool isEmpty();

    bool isWriting();

    bool isReading();

    template<class T>
    T* getDataPtr(){
        return (T*)buffer_ptr;
    }

    void debug() {
        Serial.printf("id %d, size %d, th %d, of %d, rg %d, status %d, head %u, tail %u\n",
            buffer_id,
            buffer_size,
            full_threshold,
            is_overflow,
            is_read_garbage,
            status,
            head_index,
            tail_index
        );
    }
};


#endif