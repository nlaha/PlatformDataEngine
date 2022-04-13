#pragma once
#include <string>
#include <SFML/Graphics.hpp>

namespace PlatformDataEngine {

	/// <summary>
	/// The base class for an input manager, other classes 
	/// can inherit this to feed input to the game. For example,
	/// if you had some hardware like a motion controller, midi keyboard
	/// or other weird device, you could make an input manager for it.
	/// </summary>
	class InputManager
	{
	public:

		virtual sf::Vector2f getMouse() = 0;

		class Axis {
		public:
			virtual float getValue() const = 0;
			virtual bool isPositive() const = 0;
			virtual bool isNegative() const = 0;

		private:
		};

		class Button {
		public:
			virtual bool getValue() const = 0;

		private:
		};

		virtual Axis& getAxis(std::string axisName) = 0;
		virtual Button& getButton(std::string button) = 0;
	};
}
