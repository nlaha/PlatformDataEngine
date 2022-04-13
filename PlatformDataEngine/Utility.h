#pragma once
#include <random>
#include <sstream>
#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>

#include "Globals.h"

namespace PlatformDataEngine
{

    /// <summary>
    /// Utility class for holding helper functions
    /// </summary>
    class Utility
    {
    public:
        static inline std::random_device rd;
        static inline std::mt19937 gen = std::mt19937(Utility::rd());
        static inline std::uniform_int_distribution<> dis = std::uniform_int_distribution<>(0, 15);
        static inline std::uniform_int_distribution<> dis2 = std::uniform_int_distribution<>(8, 11);

        static std::string generate_uuid_v4()
        {
            std::stringstream ss;
            int i;
            ss << std::hex;
            for (i = 0; i < 8; i++)
            {
                ss << Utility::dis(Utility::gen);
            }
            ss << "-";
            for (i = 0; i < 4; i++)
            {
                ss << Utility::dis(Utility::gen);
            }
            ss << "-4";
            for (i = 0; i < 3; i++)
            {
                ss << Utility::dis(Utility::gen);
            }
            ss << "-";
            ss << Utility::dis2(Utility::gen);
            for (i = 0; i < 3; i++)
            {
                ss << Utility::dis(Utility::gen);
            }
            ss << "-";
            for (i = 0; i < 12; i++)
            {
                ss << Utility::dis(Utility::gen);
            };
            return ss.str();
        }

        // lerp vector2f
        static sf::Vector2f lerp(sf::Vector2f a, sf::Vector2f b, float t)
        {
            return sf::Vector2f(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
        }

        // lerp b2Vec2
        static b2Vec2 lerp(b2Vec2 a, b2Vec2 b, float t)
        {
            return b2Vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
        }

        // lerp float
        static float lerp(float a, float b, float t)
        {
            return a + (b - a) * t;
        }

        // 2d LookAt
        static float lookAt(sf::Vector2f objPos, sf::Vector2f targetPos)
        {
            float angle = atan2f((targetPos.y - objPos.y), (targetPos.x - objPos.x)) * (180.0f / 3.141592f);

            return angle;
        }

        // rad to deg
        static float radToDeg(float rad)
        {
            return rad * (180.0f / 3.141592f);
        }

        // deg to rad
        static float degToRad(float deg)
        {
            return deg * (3.141592f / 180.0f);
        }

        // get direction vector
        static sf::Vector2f directionVec(sf::Vector2f point1, sf::Vector2f point2)
        {
            return normalize(sf::Vector2f(
                point1.x - point2.x,
                point1.y - point2.y));
        }

        // normalize
        static sf::Vector2f normalize(const sf::Vector2f source)
        {
            // normalize the input vector and return it
            float length = sqrt(source.x * source.x + source.y * source.y);
            return sf::Vector2f(source.x / length, source.y / length);
        }

        static b2Vec2 normalize(const b2Vec2 source)
        {
            // normalize the input vector and return it
            float length = sqrt(source.x * source.x + source.y * source.y);
            return b2Vec2(source.x / length, source.y / length);
        }

        // distance
        static float distance(sf::Vector2f point1, sf::Vector2f point2)
        {
            return std::sqrt(powf(point1.x - point2.x, 2) + powf(point1.y - point2.y, 2));
        }

        static float distance(b2Vec2 point1, b2Vec2 point2)
        {
            return std::sqrt(powf(point1.x - point2.x, 2) + powf(point1.y - point2.y, 2));
        }

        static b2Vec2 fromSf(sf::Vector2f in)
        {
            return b2Vec2(in.x / Constants::PHYS_SCALE, in.y / Constants::PHYS_SCALE);
        }

        static sf::Vector2f fromB2(b2Vec2 in)
        {
            return sf::Vector2f(in.x, in.y) * 8.0f;
        }
    };
}