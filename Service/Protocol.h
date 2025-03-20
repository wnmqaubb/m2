#pragma once
#if 0
#if defined(GATE_EXPORT) || defined(ANTICHEAT_CLIENT_API)
#define PROTOCOL_IMPL
#endif
#if defined(PROTOCOL_IMPL)
#include <zlib.h>
#define PROTOCOL_EXPORT __declspec(dllexport)
#else
#define PROTOCOL_EXPORT
#endif
#else
#ifndef __ASIO2_HPP__
#include <asio2/asio2.hpp>
#endif
#ifndef MSGPACK_OBJECT_HPP
#include <msgpack.hpp>
#endif
#ifndef INCLUDE_NLOHMANN_JSON_HPP_
#include <json/json.hpp>
using json = nlohmann::json;
#endif
#include <zlib.h>
#define PROTOCOL_EXPORT
#define PROTOCOL_IMPL
#endif
class RawProtocolHead;
class RawProtocolBody;
const unsigned char kRawProtocolHeadVersion = 1;
const unsigned int kRawProtocolMaxSize = (std::numeric_limits<unsigned int>::max)();
const size_t kCompressBound = 1024 * 10;
//const unsigned int kProtocolXorKey[4] = { 0x6432A2DF, 0xE6A253B6, 0x6F62F83C, 0x8B7BEFA4 };
const unsigned int kProtocolXorKey[4] = { 0x6431A2DF, 0xE6A953B6, 0x6F64F83C, 0x8A7BEFA4 };

#pragma pack(push, 1)
class RawProtocolHead
{
public:
	RawProtocolHead(unsigned int sz_ = sizeof(RawProtocolHead), unsigned char step_ = 0)
		: sz(sz_)
	{
		tag = kRawProtocolHeadVersion;
		step = step_;
        flag = 0;
        reserved = 0;
        session_id = 0;
	}
	RawProtocolHead(RawProtocolHead&&) = default;
	RawProtocolHead(RawProtocolHead const&) = default;
	RawProtocolHead& operator=(RawProtocolHead&&) = default;
	RawProtocolHead& operator=(RawProtocolHead const&) = default;

	inline std::size_t size() const
	{
		return sizeof(RawProtocolHead);
	}
	union
	{
		unsigned int desc;
		struct
		{
			unsigned char tag;
			unsigned char step;
            union
            {
                unsigned char flag;
                struct
                {
                    unsigned char compressed : 1;
                    unsigned char crypted : 1;
                };
            };
			unsigned char reserved;
		};
	};
    unsigned int session_id = 0;
	unsigned int sz;
};
#pragma pack(pop)

inline std::size_t bkdr_hash(const unsigned char * const p, std::size_t size)
{
    std::size_t v = 0;
    for (std::size_t i = 0; i < size; ++i)
    {
        v = v * 131 + static_cast<std::size_t>(p[i]);
    }
    return v;
}

inline void xor_buffer(void* data, size_t size, const unsigned int xor_key[4])
{
    const int length = size / sizeof(uint32_t);
    uint32_t* buffer = (uint32_t*)data;
    for (int n = 0; n < length; n++)
    {
        buffer[n] ^= xor_key[n % 4];
    }
    const int last_bytes_size = size % sizeof(uint32_t);
    if (last_bytes_size)
    {
        uint32_t last_dword = 0;
        __movsb((unsigned char*)&last_dword, (unsigned char*)&buffer[length], last_bytes_size);
        last_dword ^= xor_key[length % 4];
        __movsb((unsigned char*)&buffer[length], (unsigned char*)&last_dword, last_bytes_size);
    }
}
class RawProtocolBody
{
public:
	RawProtocolBody(unsigned int sz)
		: hash(0), buffer(sz) {}; 
    RawProtocolBody(const RawProtocolBody& other)
        : hash(other.hash), buffer(other.buffer) {
    }
	RawProtocolBody(const void* src, unsigned int sz)
	{
		copy(src, sz);
	};
	RawProtocolBody(RawProtocolBody&&) = default;
	RawProtocolBody& operator=(RawProtocolBody&&) = default;
    RawProtocolBody& operator=(const RawProtocolBody& other) {
        if (this != &other) {
            hash = other.hash;
            buffer = other.buffer; // std::vector 自动深拷贝
        }
        return *this;
    }
	inline std::size_t size()
	{
		return sizeof(hash) + buffer.size();
	}

    template <bool compressed = false>
    void copy(const void* src, unsigned int sz);

    PROTOCOL_EXPORT virtual void copy_compress(const void* src, unsigned int sz)
#if defined(PROTOCOL_IMPL)
    {
        unsigned long dst_sz = compressBound(sz);
        buffer.resize(dst_sz);
        if (compress(buffer.data(), &dst_sz, (unsigned char*)src, sz) != Z_OK)
        {
            buffer.clear();
            hash = 0;
            return;
        }
        buffer.resize(dst_sz);
        hash = bkdr_hash(buffer.data(), dst_sz);
    }
#else
        ;
#endif
    PROTOCOL_EXPORT virtual void copy_nocompress(const void* src, unsigned int sz)
#if defined(PROTOCOL_IMPL)
    {
        buffer.resize(sz);
        std::copy((unsigned char*)src, (unsigned char*)src + sz, buffer.data());
        hash = bkdr_hash(buffer.data(), sz);
    }
#else
        ;
#endif

    template<>
	void copy<false>(const void* src, unsigned int sz)
	{
        copy_nocompress(src, sz);
	}

    template<>
    void copy<true>(const void* src, unsigned int sz)
    {
        copy_compress(src, sz);
    }

	void dump(std::vector<unsigned char>& dst)
	{
		dst.resize(buffer.size());
		std::copy(buffer.begin(), buffer.end(), dst.data());
	}
	unsigned int hash;
	std::vector<unsigned char> buffer;
};


template <typename Elem>
class istream : public std::basic_istream<Elem, std::char_traits<Elem>>
{
public:
	using self = istream<Elem>;
	using super = std::basic_istream<Elem, std::char_traits<Elem>>;
	using super::super;
	template <typename T>
	self& operator >> (T& dst)
	{
		if constexpr (std::is_pod_v<T>)
		{
			return (self&)read(reinterpret_cast<Elem*>(&dst), sizeof(T));
		}
		else
		{
			static_assert("unsupport no pod data");
		}

	}
};


template <typename Elem>
class ostream : public std::basic_ostream<Elem, std::char_traits<Elem>>
{
public:
	using self = ostream<Elem>;
	using super = std::basic_ostream<Elem, std::char_traits<Elem>>;
	using super::super;
	template <typename T>
	self& operator << (T& src)
	{
		if constexpr (std::is_pod_v<T>)
		{
			return (self&)write(reinterpret_cast<Elem*>(&src), sizeof(T));
		}
		else
		{
			return (self&)write(reinterpret_cast<Elem*>(src.data()), src.size());
		}
	}
};

template <typename T>
class streambuf : public std::basic_streambuf<T, std::char_traits<T>>
{
public:
	using istream_t = istream<T>;
	using ostream_t = std::basic_ostream<T, std::char_traits<T>>;
	streambuf(const void* buffer, std::size_t sz)
		: istream_(this),
		ostream_(this)
	{
		setg((T*)buffer, (T*)buffer, (T*)buffer + sz);
		setp((T*)buffer, (T*)buffer, (T*)buffer + sz);
	}
	istream_t& istream() { return istream_; }
	ostream_t& ostream() { return ostream_; }
	ostream_t ostream_;
	istream_t istream_;
};


using bytes_streambuf = streambuf<unsigned char>;

template <class HeadType, class BodyType, bool EnableCrypt = true, bool EnableCompress = true, bool EnableValidate = true>
class RawProtocol
{
public:
	RawProtocol()
		: head(sizeof(HeadType), 0), body(0)
	{};
	RawProtocol(RawProtocol&&) = default;
	RawProtocol(RawProtocol const&) = default;
	RawProtocol& operator=(RawProtocol&&) = default;
	RawProtocol& operator=(RawProtocol const&) = default;

	inline std::size_t size()
	{
		return head.size() + body.size();
	}
    PROTOCOL_EXPORT virtual std::string release() const
#if defined(PROTOCOL_IMPL)
    {
        std::string result;
        result.resize(head.sz);
        memcpy(result.data(), &head, head.size());
        if constexpr (EnableCrypt)
        {
            xor_buffer(result.data(), head.size(), kProtocolXorKey);
        }
        memcpy(result.data() + head.size(), &body.hash, sizeof(body.hash));
        memcpy(result.data() + head.size() + sizeof(body.hash), body.buffer.data(), body.buffer.size());
        return result;
    }
#else
        ;
#endif
    void encode(const std::vector<unsigned char>& src) { encode((void*)src.data(), src.size()); }
    PROTOCOL_EXPORT virtual void encode(void* src_, std::size_t size_)
#if defined(PROTOCOL_IMPL)
    {
        if (size_ > kCompressBound)
        {
            if constexpr (EnableCompress)
            {
                body.copy<true>(src_, size_);
                head.compressed = 1;
            }
            else
            {
                body.copy<false>(src_, size_);
            }
        }
        else
        {
            body.copy<false>(src_, size_);
        }
        head.sz = head.size() + body.size();
        if constexpr (EnableCrypt)
        {
            if (head.crypted == 0)
            {
                xor_buffer(body.buffer.data(), body.buffer.size(), kProtocolXorKey);
                head.crypted = 1;
            }
        }
    }
#else
        ;
#endif
    PROTOCOL_EXPORT virtual bool decode(std::string_view sv)
#if defined(PROTOCOL_IMPL)
    {
        try {
            if (sv.size() < sizeof(HeadType))
                return false;
            bytes_streambuf bs(sv.data(), sv.size());
            bs.istream() >> head.desc >> head.session_id >> head.sz;
            if constexpr (EnableCrypt)
            {
                xor_buffer(&head, sizeof(HeadType), kProtocolXorKey);
            }
            if (sv.size() < head.size() + sizeof(body.hash))
                return false;
            unsigned int recv_hash = 0;
            bs.istream() >> recv_hash;
            std::size_t left_size = sv.size() - (head.size() + sizeof(body.hash));
            if (left_size <= 0)
                return false;
            body.buffer.resize(left_size);
            bs.istream().read(body.buffer.data(), left_size);
            if constexpr (EnableCrypt)
            {
                if (head.crypted)
                {
                    xor_buffer(body.buffer.data(), left_size, kProtocolXorKey);
                    head.crypted = 0;
                }
            }
            if constexpr (EnableValidate == false)
            {
                body.hash = recv_hash;
                return true;
            }
            body.hash = bkdr_hash(body.buffer.data(), left_size);
            if (body.hash != recv_hash)
                return false;
            if (head.compressed)
            {
                auto raw_size = left_size;
                while (true)
                {
                    BodyType uncompressed_body(raw_size * 8);
                    unsigned long dst_sz = raw_size * 8;
                    auto status = uncompress(uncompressed_body.buffer.data(), &dst_sz, body.buffer.data(), body.buffer.size());
                    if (status == Z_OK)
                    {
                        uncompressed_body.buffer.resize(dst_sz);
                        body = uncompressed_body;
                        head.compressed = 0;
                        head.sz = head.size() + body.size();
                        body.hash = bkdr_hash(body.buffer.data(), body.buffer.size());
                        return true;
                    }
                    else if (status == Z_BUF_ERROR)
                    {
                        raw_size *= 2;
                        continue;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            return true;
        }
        catch (...) {
            // ❌ 修复：失败时清空 buffer
            body.buffer.clear(); // 或 body.buffer = std::vector<char>();
            return false;
        }
    }
#else
        ;
#endif
	
	template <typename Iterator>
	std::pair<Iterator, bool> operator()(Iterator begin, Iterator end) const
	{
		do
		{
			const int recv_sz = end - begin;
			if (recv_sz < sizeof(HeadType))
				break;
			HeadType head_;
			std::copy(begin, begin + sizeof(HeadType), (unsigned char*)&head_);
            
            if constexpr (EnableCrypt)
            {
                xor_buffer(&head_, sizeof(HeadType), kProtocolXorKey);
            }
            if (head_.tag != kRawProtocolHeadVersion)
                return std::pair(begin, true);
			if (head_.sz == sizeof(HeadType))
				return std::pair(begin + sizeof(HeadType), true);
			if (end - begin < head_.sz)
				break;
			if (end - begin > kRawProtocolMaxSize)
				return std::pair(begin, true);
			return std::pair(begin + head_.sz, true);
		} while (0);
		return std::pair(begin, false);
	}
	HeadType head;
	BodyType body;
};

using RawProtocolImpl = RawProtocol<RawProtocolHead, RawProtocolBody>;
using RawProtocolImplNoCompress = RawProtocol<RawProtocolHead, RawProtocolBody, true, false>;
using RawProtocolImplNoValidate = RawProtocol<RawProtocolHead, RawProtocolBody, true, true, false>;
using RawProtocolImplBase = RawProtocol<RawProtocolHead, RawProtocolBody, false, false, false>;

namespace asio
{
	template <> struct is_match_condition<RawProtocolImpl> : public std::true_type {};
}

#include "Package.h"