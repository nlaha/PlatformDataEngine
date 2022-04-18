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

        struct AnimationFrame
        {
            int index;
            float duration;
            sf::IntRect frame;
            sf::IntRect sourceFrame;
            sf::Vector2u sourceSize;
        };
        struct Animation
        {
            std::vector<AnimationFrame> frames;
        };

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

        inline void setFlipFlag(AnimationController::FlipFlags flip) { this->m_flip = flip; };

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
