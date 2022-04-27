#pragma once
#include "GameWorld.h"
#include <GameNetworkingSockets/steam/isteamnetworkingsockets.h>
#include <GameNetworkingSockets/steam/isteamnetworkingutils.h>
#include <GameNetworkingSockets/steam/steamnetworkingsockets.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

#include <spdlog/spdlog.h>
#include <exception>

namespace PlatformDataEngine {

	static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg)
	{
		try {
			if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug)
			{
				spdlog::error(pszMsg);
			}
			else {
				//spdlog::info(pszMsg);
			}
		}
		catch (std::exception e)
		{
			spdlog::error(e.what());
		}
	}

	static void InitSteamDatagramConnectionSockets()
	{
#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
		SteamDatagramErrMsg errMsg;
		if (!GameNetworkingSockets_Init(nullptr, errMsg))
			spdlog::error("GameNetworkingSockets_Init failed.  %s", errMsg);
#else
		SteamDatagram_SetAppID(570); // Just set something, doesn't matter what
		SteamDatagram_SetUniverse(false, k_EUniverseDev);

		SteamDatagramErrMsg errMsg;
		if (!SteamDatagramClient_Init(errMsg))
			spdlog::error("SteamDatagramClient_Init failed.  %s", errMsg);

		// Disable authentication when running with Steam, for this
		// example, since we're not a real app.
		//
		// Authentication is disabled automatically in the open-source
		// version since we don't have a trusted third party to issue
		// certs.
		SteamNetworkingUtils()->SetGlobalConfigValueInt32(k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1);
#endif
		SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
	}

	static void ShutdownSteamDatagramConnectionSockets()
	{
		// Give connections time to finish up.  This is an application layer protocol
		// here, it's not TCP.  Note that if you have an application and you need to be
		// more sure about cleanup, you won't be able to do this.  You will need to send
		// a message and then either wait for the peer to close the connection, or
		// you can pool the connection to see if any reliable data is pending.
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
		GameNetworkingSockets_Kill();
#else
		SteamDatagramClient_Kill();
#endif
	}

	/// <summary>
	/// Base class for a network handler, responsible
	/// for handling network operations
	/// Example: Server, Client, etc.
	/// </summary>
	class NetworkHandler
	{
	private:
		std::string m_id;

	public:

		inline ~NetworkHandler()
		{
			ShutdownSteamDatagramConnectionSockets();
		}

		inline const std::string& getId() { return this->m_id; };
		inline void setId(const std::string& name) { this->m_id = name; };

		virtual void start() = 0;

		virtual void stop() = 0;

		/// <summary>
		/// Process is responsible for sending packets, limited in rate
		/// </summary>
		/// <param name="world"></param>
		virtual void process(GameWorld* world) = 0;

		/// <summary>
		/// Recieve checks for new packets, not limited in rate
		/// </summary>
		/// <param name="world"></param>
		virtual void recieve(GameWorld* world) = 0;

		/// <summary>
		/// Gets the current process connection
		/// </summary>
		/// <returns></returns>
		inline std::shared_ptr<Connection> getConnection() { 
			return this->m_clientConnection; 
		};

		/// <summary>
		/// Sets the current process connection
		/// </summary>
		/// <param name="conn"></param>
		inline void setConnection(std::shared_ptr<Connection> conn) { this->m_clientConnection = conn; };

	protected:
		std::shared_ptr<Connection> m_clientConnection;

	};
}
