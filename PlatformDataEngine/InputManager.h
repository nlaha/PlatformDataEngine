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

		/// <summary>
		/// Defines an axis, this is usually something like a joystick
		/// slider or knob
		/// </summary>
		class Axis {
		public:
			virtual float getValue() const = 0;
			virtual bool isPositive() const = 0;
			virtual bool isNegative() const = 0;

		private:
		};

		/// <summary>
		/// Defines a button (ie. something with a true/false state)
		/// </summary>
		class Button {
		public:
			virtual bool getValue() const = 0;

		private:
		};

		virtual Axis& getAxis(const std::string& axisName) = 0;
		virtual Button& getButton(const std::string& button) = 0;
	};
}
