#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "Buffer.h"

namespace opendlv {
namespace proxy {
namespace miniature {

Buffer::Iterator::Iterator(Buffer const *a_outer_buffer):
    m_outer_buffer(a_outer_buffer),
    m_read_pos(0)
{
}

Buffer::Iterator::~Iterator()
{
}

void Buffer::Iterator::CheckOverflow(uint32_t a_data_length)
{
  if (!HasBytesLeft(a_data_length)) {
    throw std::runtime_error("Packet buffer overflow.");
  }
}

bool Buffer::Iterator::HasBytesLeft(uint32_t a_data_length)
{
  uint32_t bytes_len = m_outer_buffer->GetSize();

  return (bytes_len - m_read_pos + 1) > a_data_length; 
}

bool Buffer::Iterator::ReadBoolean()
{
  uint8_t byte = ReadByte();
  bool data = static_cast<bool>(byte);
  return data;
}


uint8_t Buffer::Iterator::ReadByte()
{
  uint8_t const data_length = 1;
  CheckOverflow(data_length);

  uint8_t data = m_outer_buffer->m_bytes[m_read_pos];
  m_read_pos += data_length;

  return data;
}

std::shared_ptr<std::vector<uint8_t>> Buffer::Iterator::ReadBytes()
{
  int16_t const data_length = ReadInteger16();
  CheckOverflow(data_length);
  
  uint32_t const start_pos = m_read_pos;
  uint32_t const end_pos = start_pos + data_length - 1;

  std::shared_ptr<std::vector<uint8_t>> data(
      new std::vector<uint8_t>(data_length));

  for (uint32_t i = start_pos, j = 0; end_pos >= i; ++i, ++j) {
    (*data)[j] = m_outer_buffer->m_bytes[i];
  }

  m_read_pos += data_length;

  return data;
}

float Buffer::Iterator::ReadFloat32()
{
  uint8_t const data_length = 4;
  CheckOverflow(data_length);

  float data = 0.0;
  memcpy(&data, &m_outer_buffer->m_bytes[m_read_pos], data_length);
  m_read_pos += data_length;

  return data;
}

double Buffer::Iterator::ReadFloat64()
{
  uint8_t const data_length = 8;
  CheckOverflow(data_length);

  double data = 0.0;
  memcpy(&data, &m_outer_buffer->m_bytes[m_read_pos], data_length);
  m_read_pos += data_length;

  return data;
}

int8_t Buffer::Iterator::ReadInteger8()
{
  uint8_t const data_length = 1;
  CheckOverflow(data_length);

  int8_t data = 0;
  memcpy(&data, &m_outer_buffer->m_bytes[m_read_pos], data_length);
  m_read_pos += data_length;

  return data;
}

int16_t Buffer::Iterator::ReadInteger16()
{
  uint8_t const data_length = 2;
  CheckOverflow(data_length);

  int16_t data = 0;
  memcpy(&data, &m_outer_buffer->m_bytes[m_read_pos], data_length);
  m_read_pos += data_length;

  return data;
}

int32_t Buffer::Iterator::ReadInteger32()
{
  uint8_t const data_length = 4;
  CheckOverflow(data_length);

  int32_t data = 0;
  memcpy(&data, &m_outer_buffer->m_bytes[m_read_pos], data_length);
  m_read_pos += data_length;

  return data;
}

int64_t Buffer::Iterator::ReadInteger64()
{
  uint8_t const data_length = 8;
  CheckOverflow(data_length);

  int64_t data = 0;
  memcpy(&data, &m_outer_buffer->m_bytes[m_read_pos], data_length);
  m_read_pos += data_length;

  return data;
}

std::string Buffer::Iterator::ReadString()
{
  int16_t data_length = ReadInteger16();
  CheckOverflow(data_length);

  uint32_t const start_pos = m_read_pos;
  uint32_t const end_pos = start_pos + data_length - 1;

  std::stringstream string_stream;
  for (uint32_t i = start_pos; end_pos >= i; ++i) {
    string_stream << m_outer_buffer->m_bytes[i];
  }
  std::string data = string_stream.str();

  m_read_pos += data_length;

  return data;
}
void Buffer::Iterator::Reset()
{
  m_read_pos = 0;
}

Buffer::Buffer():
    m_bytes()
{
}

Buffer::Buffer(std::vector<uint8_t> const &a_bytes):
    m_bytes(a_bytes)
{
}

Buffer::~Buffer()
{
}

void Buffer::Append(std::vector<uint8_t> const &a_data)
{
  m_bytes.insert(m_bytes.end(), a_data.begin(), a_data.end());
}

void Buffer::AppendBoolean(bool a_data)
{
  AppendByte(static_cast<uint8_t>(a_data));
}

void Buffer::AppendByte(uint8_t a_data)
{
  std::vector<uint8_t> append(1);
  append[0] = a_data;

  Append(append);
}

void Buffer::AppendBytes(std::vector<uint8_t> const &a_data)
{
  uint16_t data_length = a_data.size();
  AppendInteger16(data_length);

  Append(a_data);
}

void Buffer::AppendBytesRaw(std::vector<uint8_t> const &a_data)
{
  Append(a_data);
}

void Buffer::AppendFloat32(float a_data)
{
  uint8_t const data_length = 4;

  std::vector<uint8_t> append(data_length);
  memcpy(&append[0], &a_data, data_length);

  Append(append);
}

void Buffer::AppendFloat64(double a_data)
{
  uint8_t const data_length = 8;

  std::vector<uint8_t> append(data_length);
  memcpy(&append[0], &a_data, data_length);

  Append(append);
}

void Buffer::AppendInteger8(int8_t a_data)
{
  uint8_t const data_length = 1;

  std::vector<uint8_t> append(data_length);
  memcpy(&append[0], &a_data, data_length);

  Append(append);
}

void Buffer::AppendInteger16(int16_t a_data)
{
  uint8_t const data_length = 2;

  std::vector<uint8_t> append(data_length);
  memcpy(&append[0], &a_data, data_length);

  Append(append);
}

void Buffer::AppendInteger32(int32_t a_data)
{
  uint8_t const data_length = 4;

  std::vector<uint8_t> append(data_length);
  memcpy(&append[0], &a_data, data_length);

  Append(append);
}

void Buffer::AppendInteger64(int64_t a_data)
{
  uint8_t const data_length = 8;

  std::vector<uint8_t> append(data_length);
  memcpy(&append[0], &a_data, data_length);

  Append(append);
}

void Buffer::AppendString(std::string const &a_data)
{
  uint16_t data_length = a_data.length();
  AppendInteger16(data_length);

  AppendStringRaw(a_data);
}

void Buffer::AppendStringRaw(std::string const &a_data)
{
  std::vector<uint8_t> append(a_data.begin(), a_data.end());
  Append(append);
}

std::vector<uint8_t> const &Buffer::GetData() const
{
  return m_bytes;
}

std::string const Buffer::GetDataString() const
{
  std::stringstream string_stream;
  for (uint32_t i = 0; i < m_bytes.size(); ++i) {
    string_stream << m_bytes[i];
  }
  std::string data = string_stream.str();

  return data;
}

std::shared_ptr<Buffer::Iterator> Buffer::GetIterator() const
{
  std::shared_ptr<Buffer::Iterator> iterator(
      new Buffer::Iterator(this));
  return iterator; 
}

uint32_t Buffer::GetSize() const
{
  return m_bytes.size();
}


}
}
}
