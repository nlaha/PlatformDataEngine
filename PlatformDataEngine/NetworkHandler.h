#pragma once
#include "GameWorld.h"

namespace PlatformDataEngine {
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

		inline std::shared_ptr<Connection> getConnection() { return this->m_clientConnection; };
		inline void setConnection(std::shared_ptr<Connection> conn) { this->m_clientConnection = conn; };

	protected:
		std::shared_ptr<Connection> m_clientConnection;

	};
}
