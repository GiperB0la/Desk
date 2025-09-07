#pragma once
#include "Network.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <windows.h>
#include <filesystem>
#include <memory>
#include "Widgets.hpp"

enum class WindowState 
{
    MainMenu,
    Connect,
    Demonstration,
    Settings,
    Session
};

enum class CategoryButton
{
    Close,
    RollUp,
    Connect,
    Demonstration,
    Settings,
    Back,
    Stop,
    Apply
};

class Desk 
{
public:
    Desk(const std::string& local_ip_ = "127.0.0.1", unsigned int local_port_ = 8888);
    ~Desk();

public:
    void run();

private:
    void init();
    void handle_events();
    void handle_events_buttons(const sf::Event& event_);
    void render();
    void dragging_window(const sf::Event& event_);
    void is_main_window();
    void is_connect_window();
    void is_demonstration_window();
    void is_settings_window();
    void is_session_window();
    void apply();
    void network_thread(bool demonstration);

private:
    sf::RenderWindow window_;
    std::vector<sf::Texture> backgrounds_T_;
    sf::Sprite background_S_;
    sf::Font font_;
    sf::Text text_local_ip_, text_local_port_, text_remote_ip_, text_remote_port_;
    sf::Cursor arrowCursor_, handCursor_;

    bool dragging_window_ = false;
    sf::Vector2i dragOffset_window_;

    std::vector<Button> buttons_;
    std::shared_ptr<IpInput> input_ip_;
    std::shared_ptr<PortInput> input_port_;

    const std::string main_path_;

    WindowState state_;

    std::shared_ptr<Network> network_;

    std::string local_ip_, remote_ip_;
    unsigned int local_port_, remote_port_;

    std::thread thread_;
};