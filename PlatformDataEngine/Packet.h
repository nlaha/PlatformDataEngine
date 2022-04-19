#pragma once
#include <vector>
#include <iostream>
#include <SFML/Network.hpp>
#include <spdlog/spdlog.h>
#include <snappy.h>
#include <memory>

namespace PlatformDataEngine {

	/// <summary>
	/// How networking works:
	/// The server will spawn the entire world and start a simulation,
	/// it will send out spawn game object packets to all of it's clients
	/// to replicate the same world on their computers, the server
	/// will then spawn player game objects for each client that is connected and
	/// send out "BindPlayer" packets to the clients, this will tell the client which game object
	/// it is controlling (for things like the camera controller)
	/// 
	/// When a client gets user input, instead of updating the position of it's local game object,
	/// since it's a client, it will send a "UserInput" packet to the server, the server will simulate
	/// 1 step of input and send the resulting game object positions and any other relavent data to the client
	/// 
	/// So in this system, the server is completely authoritative, the clients will simply be puppeted
	/// by the server and the inputs they send to the server will simply be Button or Axis values that
	/// the server will interperate and send position data back for.
	/// </summary>
	class PDEPacket : public sf::Packet
	{
	public:
		/** Enumerates the possible flags a packet can have.
			It will be automatically sent and received when using this class for sharing data across sockets.
			Feel free to extend this enumeration to match your need.  */
		enum PFlag {
			None,
			Ping,
			Pong,
			Connect,
			Disconnect,
			Connected,
			Disconnected,
			Text,

			SetObjectHealth,
			RequestUpdates,
			SendUpdates,
			UserInput,
		};

		/** Default constructor. */
		PDEPacket();
		/** Overloaded constructor. Constructs a Packet and sets the flag as specified. */
		PDEPacket(PFlag);

		/** Sets the flag to the packet */
		void setFlag(PFlag);

		/** Reads the flag. */
		inline PFlag        flag() const { return static_cast<PFlag>(m_flag); }
		/** Reads the flag as an 8 bytes unsigned int. */
		inline sf::Uint8 flagAsU() const { return m_flag; }
		/** Checks if a flag has been set on this Packet. Returns false if flag equals to PDEPacket::None. */
		inline bool    isFlagged() const { return m_flag == PDEPacket::None ? false : true; }

		/** Hiding base class clear() function.
			Clears the packet as usual and sets the flag to 'None', so isFlagged() will return false.  */
		virtual void clear();


	private:
		sf::Uint8    m_flag;

		virtual void onReceive(const void*, std::size_t);
		virtual const void* onSend(std::size_t&);

	};


	/** Overloading of comparison operator. Checks a PDEPacket object against a PFlag enumeration type instance (PDEPacket::PFlag). */
	bool operator== (const PDEPacket& p1, const PDEPacket::PFlag& p2);
	/** Overloading of comparison operator. Checks a Packet object against a PFlag enumeration type instance (PDEPacket::PFlag). */
	bool operator!= (const PDEPacket& p1, const PDEPacket::PFlag& p2);

}

