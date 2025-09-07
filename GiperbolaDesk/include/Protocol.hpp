#pragma once
#include <iostream>
#include <variant>

enum class EventType : uint8_t
{
    MouseMove = 0x01,
    MouseLeftClick = 0x02,
    MouseRightClick = 0x03,
    MouseWheel = 0x04,
    KeyPress = 0x05
};

struct MouseMoveData { int x, y; };
struct MouseClickData { int x, y; };
struct MouseWheelData { int x, y, delta; };
struct KeyPressData { int keycode; };

using EventPayload = std::variant<
    std::monostate,
    MouseMoveData,
    MouseClickData,
    MouseWheelData,
    KeyPressData
>;