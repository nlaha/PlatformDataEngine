#pragma once
#include "Packet.h"

namespace PlatformDataEngine {
	class Networkable
	{
	public:

		/// <summary>
		/// Called when an object is sent to the clients
		/// for the first time
		/// </summary>
		/// <param name="output"></param>
		virtual void networkSerializeInit(PDEPacket& output) = 0;
		
		/// <summary>
		/// Called when an object is recieved on a client
		/// for the first time
		/// </summary>
		/// <param name="input"></param>
		virtual void networkDeserializeInit(PDEPacket& input) = 0;

		/// <summary>
		/// Called every time an object is sent to a client
		/// </summary>
		/// <param name="output"></param>
		virtual void networkSerialize(PDEPacket& output) = 0;

		/// <summary>
		/// Called every time an object is recieved from a client
		/// </summary>
		/// <param name="input"></param>
		virtual void networkDeserialize(PDEPacket& input) = 0;
	};
}