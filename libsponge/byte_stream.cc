#include "byte_stream.hh"

#include <iostream>
// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : _capacity(capacity), _front(0), _rear(0), _buffer(vector<char>(_capacity)), _read_count(0), _write_count(0) {}

size_t ByteStream::write(const string &data) {
    size_t len = min(data.size(), remaining_capacity());
    size_t index = 0;
    while (index != len) {
        _buffer[_rear] = data[index++];
        _rear = (_rear + 1) % (_capacity + 1);
    }
    _write_count += len;
    return len;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    size_t peek_len = min(len, buffer_size());
    string peek_str;
    size_t cur = _front;
    while (peek_len > 0) {
        peek_str.push_back(_buffer[cur]);
        cur = (cur + 1) % (_capacity + 1);
        peek_len--;
    }
    return peek_str;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    size_t rm_len = min(len, buffer_size());
    _front = (_front + rm_len) % (_capacity + 1);
    _read_count += rm_len;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    size_t read_len = min(len, buffer_size());
    string read_str;
    while (read_len > 0) {
        read_str.push_back(_buffer[_front]);
        _front = (_front + 1) % (_capacity + 1);
        read_len--, _read_count++;
    }
    return read_str;
}

void ByteStream::end_input() { _eof = true; }

bool ByteStream::input_ended() const { return _eof; }

size_t ByteStream::buffer_size() const { return buffer_used(); }

bool ByteStream::buffer_empty() const { return _front == _rear; }

bool ByteStream::eof() const { return _eof && buffer_empty(); }

size_t ByteStream::bytes_written() const { return _write_count; }

size_t ByteStream::bytes_read() const { return _read_count; }

size_t ByteStream::remaining_capacity() const { return _capacity - buffer_used(); }
