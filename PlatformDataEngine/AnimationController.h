#pragma once
#include <fstream>

#include "Component.h"
namespace PlatformDataEngine
{

    class SpriteRenderer;

    /// <summary>
    /// A component that manages playing animations on a gameObject.
    /// Precondition: the game object must have a sprite renderer component.
    /// </summary>
    class AnimationController : public Component
    {
    public:
        AnimationController();

        /// <summary>
        /// Represents a frame in an animation
        /// </summary>
        struct AnimationFrame
        {
            int index;
            float duration;
            sf::IntRect frame;
            sf::IntRect sourceFrame;
            sf::Vector2u sourceSize;
        };

        /// <summary>
        /// Represents an animation (a collection of frames)
        /// </summary>
        struct Animation
        {
            std::vector<AnimationFrame> frames;
        };

        /// <summary>
        /// Enum for flipping a sprite
        /// </summary>
        enum FlipFlags
        {
            NONE,
            HORIZONTAL,
            VERTICAL
        };

        void init();

        void update(const float &dt, const float &elapsedTime);

        void draw(sf::RenderTarget &target, sf::RenderStates states) const;

        void copy(std::shared_ptr<Component> otherCompPtr);

        void networkSerialize(PDEPacket& output);
        void networkDeserialize(PDEPacket& input);

        void loadDefinition(nlohmann::json object);

        /// <summary>
        /// Sets the flip flags for this animation controller
        /// </summary>
        /// <param name="flip">the new flip flag</param>
        inline void setFlipFlag(AnimationController::FlipFlags flip) { this->m_flip = flip; };

        inline std::string getAnimName() const { return this->m_currentAnim; };
        void setAnimation(const std::string animName, float speed = 1.0f, bool loop = true);

    private:
        SpriteRenderer* m_spriteRenderer;

        std::map<std::string, Animation> m_animations;
        AnimationFrame* m_curFrame;

        std::string m_currentAnim;
        std::string m_lastAnim;
        sf::Clock m_frameTimer;

        bool m_loop;
        float m_speed;
        FlipFlags m_flip;
    };
}
