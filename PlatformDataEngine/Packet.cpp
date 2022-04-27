#include "Packet.h"

using namespace PlatformDataEngine;

#include <cstring>

PDEPacket::PDEPacket() : m_flag(None)
{
    this->m_compressed = false;
}

PDEPacket::PDEPacket(PFlag flag) : m_flag(flag)
{
    this->m_compressed = false;
}

void PDEPacket::setFlag(PFlag flag)
{
    m_flag = flag;
}

void PDEPacket::clear()
{
    static_cast<sf::Packet *>(this)->clear();
    m_flag = None;
}

// void PDEPacket::onReceive(const void* data, std::size_t size)
//{
//     std::string uncompressed;
//     snappy::Uncompress(((char*)data), size, &uncompressed);
//
//     std::size_t so_flag = sizeof m_flag;
//
//     std::memcpy(&m_flag, ((char*)uncompressed.data()) + uncompressed.size() - so_flag, so_flag);
//
//     append(uncompressed.data(), uncompressed.size() - so_flag);
// }
//
// const void* PDEPacket::onSend(std::size_t& size)
//{
//     *this << m_flag;
//
//     std::string compressed;
//     snappy::Compress(((char*)getData()), getDataSize(), &compressed);
//
//     size = compressed.size();
//
//     clear();
//     append(compressed.data(), compressed.size());
//
//     return getData();
// }

////////////////////////////////////////////////////////////
const void *PDEPacket::onSend(std::size_t &size)
{
    *this << m_flag;

    size = getDataSize();
    return getData();

    //*this << m_flag;

    //std::string compressed;
    //snappy::Compress(((char*)getData()), getDataSize(), &compressed);

    //size = compressed.size();

    //clear();
    //append(compressed.data(), compressed.size());

    //return getData();
}

////////////////////////////////////////////////////////////
void PDEPacket::onReceive(const void *data, std::size_t size)
{
    std::size_t so_flag = sizeof m_flag;
    std::memcpy(&m_flag, ((char*)data) + size - so_flag, so_flag);

    // print data as hex string
    /*std::stringstream ss;
    ss << std::hex;

    for (int i(0); i < size; ++i)
        ss << std::setw(2) << std::setfill('0') << (int)((char*)data)[i];*/

    //spdlog::warn("Packet size: {} bytes, type: {}", size, this->m_flag);

    append(data, size - so_flag);

    //std::string uncompressed;
    //snappy::Uncompress(((char*)data), size, &uncompressed);

    //std::size_t so_flag = sizeof m_flag;

    //std::memcpy(&m_flag, ((char*)uncompressed.data()) + uncompressed.size() - so_flag, so_flag);

    //append(uncompressed.data(), uncompressed.size() - so_flag);
}

bool operator==(const PDEPacket &p1, const PDEPacket::PFlag &p2)
{
    return p1.flag() == p2;
}

bool operator!=(const PDEPacket &p1, const PDEPacket::PFlag &p2)
{
    return p1.flag() != p2;
}