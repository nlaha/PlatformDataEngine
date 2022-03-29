#include "AnimationController.h"
#include "SpriteRenderer.h"
#include "GameObject.h"
#include <spdlog/spdlog.h>

using namespace PlatformDataEngine;

PlatformDataEngine::AnimationController::AnimationController()
{
    this->m_curFrame = nullptr;
    this->m_frameTimer = sf::Clock();
}

void AnimationController::init()
{
    std::shared_ptr<SpriteRenderer> spriteRender = this->m_parent->findComponentOfType<SpriteRenderer>();
    if (spriteRender.get() != nullptr)
    {
        this->m_spriteRenderer = spriteRender;
    }
    else
    {
        spdlog::critical("GameObject {} has a AnimationController so it must also have a SpriteRenderer", this->m_parent->getName());
    }
}

void AnimationController::update(const float &dt, const float &elapsedTime)
{
    sf::FloatRect bounds = this->m_spriteRenderer->getSprite()->getLocalBounds();

    // play animation
    if (this->m_currentAnim != "")
    {
        Animation &anim = this->m_animations[this->m_currentAnim];
        if (anim.frames.size() > 0)
        {
            if (this->m_curFrame == nullptr)
            {
                // starting at beginning
                this->m_curFrame = &anim.frames[0];
            }
            else
            {
                if (anim.frames.size() > this->m_curFrame->index)
                {
                    if (this->m_frameTimer.getElapsedTime().asMilliseconds() >= this->m_curFrame->duration / this->m_speed) {
                        // increment frame
                        this->m_curFrame = &anim.frames[this->m_curFrame->index + 1];
                        this->m_frameTimer.restart();
                    }
                }
                else
                {
                    // starting at beginning
                    this->m_curFrame = &anim.frames[0];
                }
            }
        }
    }
}

void AnimationController::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    // draw frame
    sf::IntRect frameRect = this->m_curFrame->frame;
    if (this->m_flip == AnimationController::FlipFlags::HORIZONTAL)
    {
        frameRect.width *= -1;
    }

    if (this->m_flip == AnimationController::FlipFlags::VERTICAL)
    {
        frameRect.height *= -1;
    }

    if (this->m_flip == AnimationController::FlipFlags::NONE)
    {
        frameRect =  this->m_curFrame->frame;
    }

    this->m_spriteRenderer->setRect(frameRect);
}

void AnimationController::setAnimation(std::string animName, float speed, bool loop)
{
    this->m_currentAnim = animName;
    this->m_loop = loop;
    this->m_speed = speed;
}

void AnimationController::loadDefinition(nlohmann::json object)
{
    // TODO: load and parse aesprite json file
    std::string filePath = object.at("aespriteFile");
    std::ifstream file(filePath);

    if (!file.is_open())
    {
        spdlog::error("Failed to open animation file: {}", filePath);
        return;
    }
    spdlog::info("Loading animations from: {}", filePath);

    nlohmann::json aespriteJson;
    file >> aespriteJson;

    std::vector<std::string> animations;
    for (auto &anim : aespriteJson.at("meta").at("layers"))
    {
        animations.push_back(anim.at("name"));
    }

    // lambda for parsing json IntRects
    auto parseIntRect = [](nlohmann::json rect)
    {
        return sf::IntRect(
            rect.at("x"),
            rect.at("y"),
            rect.at("w"),
            rect.at("h"));
    };

    // lambda for parsing json IntVectors
    auto parseSize = [](nlohmann::json rect)
    {
        return sf::Vector2u(
            rect.at("w"),
            rect.at("h"));
    };

    // load animation frames
    int frameIdx = 0;
    for (auto& frame : aespriteJson.at("frames").items())
    {
        auto it = std::find_if(
            begin(animations),
            end(animations),
            [&](const std::string& s)
            {
                return frame.key().find(s) != std::string::npos;
            });

        if (it != end(animations))
        {
            // found
            std::string animName = *it;

            if (this->m_animations.count(animName) == 0)
            {
                // first frame of this animation
                // create new animation
                frameIdx = 0;
                Animation anim = {
                    {{frameIdx,
                        frame.value().at("duration"),
                        parseIntRect(frame.value().at("frame")),
                        parseIntRect(frame.value().at("spriteSourceSize")),
                        parseSize(frame.value().at("sourceSize"))}} };
                this->m_animations.emplace(animName, anim);
            }
            else
            {
                frameIdx++;
                Animation* anim = &this->m_animations.at(animName);
                anim->frames.push_back({ frameIdx,
                                        frame.value().at("duration"),
                                        parseIntRect(frame.value().at("frame")),
                                        parseIntRect(frame.value().at("spriteSourceSize")),
                                        parseSize(frame.value().at("sourceSize")) });
            }
        }
    }

    this->m_currentAnim = object.at("animation");
    this->m_loop = object.at("loop");
    this->m_speed = object.at("speed");
    this->m_flip = AnimationController::FlipFlags::HORIZONTAL;
}