#pragma once
#include <random>
#include <sstream>
#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>

namespace PlatformDataEngine {

    /// <summary>
    /// Utility class for holding helper functions
    /// </summary>
    class Utility {
    public:
        static inline std::random_device              rd;
        static inline std::mt19937                    gen = std::mt19937(Utility::rd());
        static inline std::uniform_int_distribution<> dis = std::uniform_int_distribution<>(0, 15);
        static inline std::uniform_int_distribution<> dis2 = std::uniform_int_distribution<>(8, 11);

        static std::string generate_uuid_v4() {
            std::stringstream ss;
            int i;
            ss << std::hex;
            for (i = 0; i < 8; i++) {
                ss << Utility::dis(Utility::gen);
            }
            ss << "-";
            for (i = 0; i < 4; i++) {
                ss << Utility::dis(Utility::gen);
            }
            ss << "-4";
            for (i = 0; i < 3; i++) {
                ss << Utility::dis(Utility::gen);
            }
            ss << "-";
            ss << Utility::dis2(Utility::gen);
            for (i = 0; i < 3; i++) {
                ss << Utility::dis(Utility::gen);
            }
            ss << "-";
            for (i = 0; i < 12; i++) {
                ss << Utility::dis(Utility::gen);
            };
            return ss.str();
        }

        // lerp vector2f
        static sf::Vector2f lerp(sf::Vector2f a, sf::Vector2f b, float t) {
            return sf::Vector2f(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
        }

        // lerp b2Vec2
        static b2Vec2 lerp(b2Vec2 a, b2Vec2 b, float t) {
            return b2Vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
        }
    };
}