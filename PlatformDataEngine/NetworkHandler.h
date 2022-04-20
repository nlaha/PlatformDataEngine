#pragma once
#include "GameWorld.h"

namespace PlatformDataEngine {

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
		inline std::shared_ptr<Connection> getConnection() { return this->m_clientConnection; };

		/// <summary>
		/// Sets the current process connection
		/// </summary>
		/// <param name="conn"></param>
		inline void setConnection(std::shared_ptr<Connection> conn) { this->m_clientConnection = conn; };

	protected:
		std::shared_ptr<Connection> m_clientConnection;

	};
}
