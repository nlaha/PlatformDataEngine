#pragma once
#include "Packet.h"

namespace PlatformDataEngine {
	class Networkable
	{
	public:

		virtual void networkSerializeInit(PDEPacket& output) = 0;
		virtual void networkDeserializeInit(PDEPacket& input) = 0;

		virtual void networkSerialize(PDEPacket& output) = 0;
		virtual void networkDeserialize(PDEPacket& input) = 0;
	};
}