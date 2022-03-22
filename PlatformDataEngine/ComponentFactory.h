#pragma once
#include <map>
#include <string>
#include <memory>

#include "Component.h"
#include "AnimationController.h"
#include "CharacterController.h"
#include "SpriteRenderer.h"
#include "PhysicsBody.h"

namespace PlatformDataEngine {

	/// <summary>
	/// COMPONENT FACTORY
    /// This is a class that is responsible for creating components
    /// of different types from a string representation of the type.
    /// For example, if the component with type "AnimationController"
    /// is added in the json file, we need a way to make an object of
    /// type AnimationController so we'd call the following:
    /// ComponentFactory::create("AnimationController")
	/// </summary>
	class ComponentFactory {
    public:
        static inline std::shared_ptr<Component> create(const std::string& type)
        {
            return factoryMap.at(type);
        }

    private:
        static inline const std::map<std::string, std::shared_ptr<Component>> factoryMap = {
            {"AnimationController", std::make_shared<AnimationController>()},
            {"CharacterController", std::make_shared<CharacterController>()},
            {"SpriteRenderer", std::make_shared<SpriteRenderer>()},
            {"PhysicsBody", std::make_shared<PhysicsBody>()}
        };
	};
}

