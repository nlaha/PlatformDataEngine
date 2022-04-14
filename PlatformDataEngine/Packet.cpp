#include "Packet.h"

using namespace PlatformDataEngine;

#include <cstring>


PDEPacket::PDEPacket() :
    m_flag(None)
{

}


PDEPacket::PDEPacket(PFlag flag) :
    m_flag(flag)
{

}


void PDEPacket::setFlag(PFlag flag)
{
    m_flag = flag;
}



void PDEPacket::clear()
{
    static_cast<sf::Packet*>(this)->clear();
    m_flag = None;
}



void PDEPacket::onReceive(const void* data, std::size_t size)
{
    std::string uncompressed;
    snappy::Uncompress(((char*)data), size, &uncompressed);

    std::size_t so_flag = sizeof m_flag;

    std::memcpy(&m_flag, ((char*)uncompressed.data()) + uncompressed.size() - so_flag, so_flag);

    append(uncompressed.data(), uncompressed.size() - so_flag);
}



const void* PDEPacket::onSend(std::size_t& size)
{
    *this << m_flag;

    std::string compressed;
    snappy::Compress(((char*)getData()), getDataSize(), &compressed);

    size = compressed.size();

    clear();
    append(compressed.data(), compressed.size());

    return getData();
}



bool operator== (const PDEPacket& p1, const PDEPacket::PFlag& p2)
{
    return p1.flag() == p2;
}

bool operator!= (const PDEPacket& p1, const PDEPacket::PFlag& p2)
{
    return p1.flag() != p2;
}