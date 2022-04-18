#include "AnimationController.h"
#include "SpriteRenderer.h"
#include "GameObject.h"
#include <spdlog/spdlog.h>
#include "PlatformDataEngineWrapper.h"

using namespace PlatformDataEngine;

AnimationController::AnimationController()
{
    this->m_curFrame = nullptr;
    this->m_frameTimer = sf::Clock();
    this->m_loop = false;
    this->m_flip = AnimationController::FlipFlags::NONE;
    this->m_speed = 1.0f;
    this->m_animations.clear();
}

void AnimationController::init()
{
    Component::init();

    SpriteRenderer* spriteRender = this->m_parent->findComponentOfType<SpriteRenderer>().get();
    if (spriteRender != nullptr)
    {
        this->m_spriteRenderer = spriteRender;
    }
    else
    {
        spdlog::critical("GameObject {} has a AnimationController so it must also have a SpriteRenderer", this->m_parent->getId());
    }

    // init to first frame of animation until we start updating
    if (this->m_currentAnim != "")
    {
        Animation& anim = this->m_animations[this->m_currentAnim];
        if (this->m_curFrame == nullptr)
        {
            // starting at beginning
            this->m_curFrame = &anim.frames[0];
        }
    }
}

void AnimationController::update(const float &dt, const float &elapsedTime)
{
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
                if (this->m_frameTimer.getElapsedTime().asMilliseconds() >= this->m_curFrame->duration / this->m_speed) {

                    if (this->m_curFrame->index < anim.frames.size() - 1)
                    {
                        // increment frame
                        this->m_curFrame = &anim.frames[this->m_curFrame->index + 1];
                    }
                    else
                    {
                        if (this->m_loop) {
                            // starting at beginning
                            this->m_curFrame = &anim.frames[0];
                        }
                        else {
                            this->m_curFrame = &this->m_animations[this->m_lastAnim].frames[0];
                            this->m_currentAnim = m_lastAnim;
                        }
                    }
                    this->m_frameTimer.restart();
                }
            }
        }
    }

    if (!this->m_parent->getNetworked() || !PlatformDataEngineWrapper::getIsClient()) {
        if (this->m_curFrame != nullptr) {
            // draw frame
            sf::IntRect frameRect = this->m_curFrame->frame;
            if (this->m_flip == AnimationController::FlipFlags::HORIZONTAL)
            {
                frameRect.width *= -1;
                frameRect.left -= frameRect.width;
            }

            if (this->m_flip == AnimationController::FlipFlags::VERTICAL)
            {
                frameRect.height *= -1;
                frameRect.top -= frameRect.height;
            }

            this->m_spriteRenderer->setRect(frameRect);
        }
    }
}

void AnimationController::draw(sf::RenderTarget &target, sf::RenderStates states) const
{

}

void AnimationController::copy(std::shared_ptr<Component> otherCompPtr)
{
    std::shared_ptr<AnimationController> other = std::dynamic_pointer_cast<AnimationController>(otherCompPtr);

    *this = *other;

}

void AnimationController::networkSerialize(PDEPacket& output)
{
    //output << this->m_curFrame->index << this->m_currentAnim << static_cast<sf::Uint8>(this->m_flip);
}

void AnimationController::networkDeserialize(PDEPacket& input)
{
    //int index = 0;
    //std::string animName = "";
    //sf::Uint8 flip;
    //input >> index >> animName >> flip;
    //this->m_flip = static_cast<FlipFlags>(flip);

    //if (animName != "") {
    //    Animation& anim = this->m_animations[animName];
    //    this->m_curFrame = &anim.frames[index];
    //}
}

void AnimationController::setAnimation(const std::string animName, float speed, bool loop)
{
    this->m_lastAnim = this->m_currentAnim;
    this->m_currentAnim = animName;
    this->m_loop = loop;
    this->m_speed = speed;
    if (!loop) {
        Animation& anim = this->m_animations[this->m_currentAnim];
        this->m_curFrame = &anim.frames[0];
    }
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
    this->m_flip = AnimationController::FlipFlags::NONE;
}