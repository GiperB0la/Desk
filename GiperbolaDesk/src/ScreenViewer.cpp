#include "../include/ScreenViewer.hpp"


ScreenViewer::ScreenViewer()
{
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    window_.create(desktop, "GiperbolaDesk", sf::Style::Default);
    window_.setPosition({ 0, 0 });
    window_.setFramerateLimit(60);
}

bool ScreenViewer::is_open() const
{
    return window_.isOpen();
}

bool ScreenViewer::poll_events(Network* network_)
{
    sf::Event event;
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
            return false;
        }

        // Перемещение мыши
        if (event.type == sf::Event::MouseMoved) {
            int x = event.mouseMove.x;
            int y = event.mouseMove.y;

            if (network_) {
                static auto last_send = std::chrono::steady_clock::now();
                auto now = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_send).count() > 1000) {
                    network_->send_event(EventType::MouseMove, MouseMoveData{ x, y });
                    last_send = now;
                }
            }
        }

        // Клики мыши
        if (event.type == sf::Event::MouseButtonPressed) {
            int x = event.mouseButton.x;
            int y = event.mouseButton.y;

            if (network_) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    network_->send_event(EventType::MouseLeftClick, MouseClickData{ x, y });
                }
                else if (event.mouseButton.button == sf::Mouse::Right) {
                    network_->send_event(EventType::MouseRightClick, MouseClickData{ x, y });
                }
            }
        }

        // Колесо мыши
        if (event.type == sf::Event::MouseWheelScrolled) {
            int x = static_cast<int>(event.mouseWheelScroll.x);
            int y = static_cast<int>(event.mouseWheelScroll.y);
            int delta = static_cast<int>(event.mouseWheelScroll.delta);

            if (network_) {
                network_->send_event(EventType::MouseWheel, MouseWheelData{ x, y, delta });
            }
        }

        // Нажатие клавиш
        if (event.type == sf::Event::KeyPressed) {
            int keycode = static_cast<int>(event.key.code);
            if (network_) {
                network_->send_event(EventType::KeyPress, KeyPressData{ keycode });
            }
        }
    }

    return true;
}

void ScreenViewer::display_frame(const std::optional<std::vector<uint8_t>>& frame)
{
    texture_.loadFromMemory(frame->data(), frame->size());
    sprite_.setTexture(texture_, true);

    window_.clear();
    window_.draw(sprite_);
    window_.display();
}