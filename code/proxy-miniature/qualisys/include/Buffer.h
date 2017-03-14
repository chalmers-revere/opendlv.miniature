
#ifndef PROXY_MINIATURE_BUFFER_H
#define PROXY_MINIATURE_BUFFER_H

#include <memory>
#include <string>
#include <vector>

namespace opendlv {
namespace proxy {
namespace miniature {

class Buffer {
  public:
    class Iterator {
      public:
        explicit Iterator(Buffer const *);
        Iterator(Iterator const &) = delete;
        Iterator &operator=(Iterator const &) = delete;
        virtual ~Iterator();
        bool ReadBoolean();
        uint8_t ReadByte();
        std::shared_ptr<std::vector<uint8_t>> ReadBytes();
        float ReadFloat32();
        double ReadFloat64();
        int8_t ReadInteger8();
        int16_t ReadInteger16();
        int32_t ReadInteger32();
        int64_t ReadInteger64();
        std::string ReadString();
        void Reset();

      private:
        void CheckOverflow(uint32_t);
        bool HasBytesLeft(uint32_t);
   
        Buffer const *m_outer_buffer;
        uint32_t m_read_pos;
    };



    Buffer();
    explicit Buffer(std::vector<uint8_t> const &);
    Buffer(Buffer const &) = delete;
    Buffer &operator=(Buffer const &) = delete;
    virtual ~Buffer();
    void AppendBoolean(bool);
    void AppendByte(uint8_t);
    void AppendBytes(std::vector<uint8_t> const &);
    void AppendBytesRaw(std::vector<uint8_t> const &);
    void AppendFloat32(float);
    void AppendFloat64(double);
    void AppendInteger8(int8_t);
    void AppendInteger16(int16_t);
    void AppendInteger32(int32_t);
    void AppendInteger64(int64_t);
    void AppendString(std::string const &);
    void AppendStringRaw(std::string const &);
    std::vector<uint8_t> const &GetData() const;
    std::string const GetDataString() const;
    std::shared_ptr<Buffer::Iterator> GetIterator() const;
    uint32_t GetSize() const;

  private:
    void Append(std::vector<uint8_t> const &);

    std::vector<uint8_t> m_bytes;
};

}
}
}
#endif
