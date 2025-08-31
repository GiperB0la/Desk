#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <optional>
#include <opencv2/opencv.hpp>
#include "Network.hpp"

class ScreenViewer
{
public:
    ScreenViewer(unsigned width, unsigned height, const std::string& title);

public:
    bool is_open() const;
    bool poll_events(Network* network_);
    void display_frame(const std::optional<std::vector<uint8_t>>& frame);

private:
    sf::RenderWindow window_;
    sf::Texture texture_;
    sf::Sprite sprite_;
};