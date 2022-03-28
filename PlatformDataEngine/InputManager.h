#pragma once
#include <string>

namespace PlatformDataEngine {

	class InputManager
	{
	public:

		class Axis {
		public:
			virtual float getValue() = 0;
			virtual bool isPositive() = 0;
			virtual bool isNegative() = 0;

		private:
		};

		class Button {
		public:
			virtual bool getValue() = 0;

		private:
		};

		virtual Axis& getAxis(std::string axisName) = 0;
		virtual Button& getButton(std::string button) = 0;
	};
}
