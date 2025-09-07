#include "../include/Desk.hpp"


Desk::Desk(const std::string& local_ip_, unsigned int local_port_)
	: 
    local_ip_(local_ip_), local_port_(local_port_),
    window_(sf::VideoMode(400, 600), "GiperbolaDesk", sf::Style::None),
    main_path_(std::filesystem::current_path().string())
{
    window_.setFramerateLimit(60);

    int w = 400;
    int h = 600;
    int r = 15;
    HWND hwnd = window_.getSystemHandle();
    UINT dpi = GetDpiForWindow(hwnd);
    auto px = [&](int v) { return MulDiv(v, dpi, 96); };

    HRGN region = CreateRoundRectRgn(0, 0, px(w) + 1, px(h) + 1, px(2 * r), px(2 * r));
    SetWindowRgn(hwnd, region, TRUE);
}

Desk::~Desk()
{
    if (thread_.joinable()) {
        thread_.join();
    }
}

void Desk::init()
{
    std::array<std::string, 5> paths = {
            main_path_ + "\\Graphics\\Textures\\Background\\Background1.png",
            main_path_ + "\\Graphics\\Textures\\Background\\Background2.png",
            main_path_ + "\\Graphics\\Textures\\Background\\Background3.png",
            main_path_ + "\\Graphics\\Textures\\Background\\Background4.png",
            main_path_ + "\\Graphics\\Textures\\Background\\Background5.png"
    };

    for (auto& path : paths) {
        sf::Texture texture;
        texture.loadFromFile(path);
        backgrounds_T_.push_back(texture);
    }

    state_ = WindowState::MainMenu;

    background_S_.setTexture(backgrounds_T_[static_cast<size_t>(state_)]);

    font_.loadFromFile(main_path_ + "\\Graphics\\Fonts\\Inter\\Inter-Regular.otf");

    buttons_.emplace_back(
        static_cast<size_t>(CategoryButton::Close),
        sf::Vector2f(362.f, 4.f),
        main_path_ + "\\Graphics\\Textures\\Buttons\\Close\\Close1.png",
        main_path_ + "\\Graphics\\Textures\\Buttons\\Close\\Close2.png",
        main_path_ + "\\Graphics\\Textures\\Buttons\\Close\\Close3.png"
    );
    buttons_.emplace_back(
        static_cast<size_t>(CategoryButton::RollUp),
        sf::Vector2f(318.f, 4.f),
        main_path_ + "\\Graphics\\Textures\\Buttons\\RollUp\\RollUp1.png",
        main_path_ + "\\Graphics\\Textures\\Buttons\\RollUp\\RollUp2.png",
        main_path_ + "\\Graphics\\Textures\\Buttons\\RollUp\\RollUp3.png"
    );
    buttons_.emplace_back(
        static_cast<size_t>(CategoryButton::Connect),
        sf::Vector2f(117.f, 208.f),
        main_path_ + "\\Graphics\\Textures\\Buttons\\Connect\\Connect1.png",
        main_path_ + "\\Graphics\\Textures\\Buttons\\Connect\\Connect2.png",
        main_path_ + "\\Graphics\\Textures\\Buttons\\Connect\\Connect3.png"
    );
    buttons_.emplace_back(
        static_cast<size_t>(CategoryButton::Demonstration),
        sf::Vector2f(84.f, 317.f),
        main_path_ + "\\Graphics\\Textures\\Buttons\\Demonstration\\Demonstration1.png",
        main_path_ + "\\Graphics\\Textures\\Buttons\\Demonstration\\Demonstration2.png",
        main_path_ + "\\Graphics\\Textures\\Buttons\\Demonstration\\Demonstration3.png"
    );
    buttons_.emplace_back(
        static_cast<size_t>(CategoryButton::Settings),
        sf::Vector2f(280.f, 442.f),
        main_path_ + "\\Graphics\\Textures\\Buttons\\Settings\\Settings1.png",
        main_path_ + "\\Graphics\\Textures\\Buttons\\Settings\\Settings2.png",
        main_path_ + "\\Graphics\\Textures\\Buttons\\Settings\\Settings3.png"
    );
    buttons_.emplace_back(
        static_cast<size_t>(CategoryButton::Back),
        sf::Vector2f(30.f, 521.f),
        main_path_ + "\\Graphics\\Textures\\Buttons\\Back\\Back1.png",
        main_path_ + "\\Graphics\\Textures\\Buttons\\Back\\Back2.png",
        main_path_ + "\\Graphics\\Textures\\Buttons\\Back\\Back3.png"
    );
    buttons_.emplace_back(
        static_cast<size_t>(CategoryButton::Stop),
        sf::Vector2f(134.f, 467.f),
        main_path_ + "\\Graphics\\Textures\\Buttons\\Stop\\Stop1.png",
        main_path_ + "\\Graphics\\Textures\\Buttons\\Stop\\Stop2.png",
        main_path_ + "\\Graphics\\Textures\\Buttons\\Stop\\Stop3.png"
    );
    buttons_.emplace_back(
        static_cast<size_t>(CategoryButton::Apply),
        sf::Vector2f(216.f, 442.f),
        main_path_ + "\\Graphics\\Textures\\Buttons\\Apply\\Apply1.png",
        main_path_ + "\\Graphics\\Textures\\Buttons\\Apply\\Apply2.png",
        main_path_ + "\\Graphics\\Textures\\Buttons\\Apply\\Apply3.png"
    );

    for (auto& button : buttons_) {
        button.apply_texture_for_state();
    }

    input_ip_ = std::make_shared<IpInput>(
        1, 
        sf::Vector2f(40.f, 296.f),
        main_path_ + "\\Graphics\\Textures\\TextInput\\Ip\\InputIP1.png",
        main_path_ + "\\Graphics\\Textures\\TextInput\\Ip\\InputIP2.png",
        main_path_ + "\\Graphics\\Textures\\TextInput\\Ip\\InputIP3.png",
        font_,
        "IP address.."
    );

    input_port_ = std::make_shared<PortInput>(
        1,
        sf::Vector2f(40.f, 364.f),
        main_path_ + "\\Graphics\\Textures\\TextInput\\Port\\InputPort1.png",
        main_path_ + "\\Graphics\\Textures\\TextInput\\Port\\InputPort2.png",
        main_path_ + "\\Graphics\\Textures\\TextInput\\Port\\InputPort3.png",
        font_,
        "Port.."
    );

    text_local_ip_.setFont(font_);
    text_local_ip_.setCharacterSize(20);
    text_local_ip_.setPosition(sf::Vector2f(152.f, 492.f));
    text_local_ip_.setString(local_ip_);

    text_local_port_.setFont(font_);
    text_local_port_.setCharacterSize(20);
    text_local_port_.setPosition(sf::Vector2f(152.f, 526.f));
    text_local_port_.setString(std::to_string(local_port_));

    text_remote_ip_.setFont(font_);
    text_remote_ip_.setCharacterSize(20);
    text_remote_ip_.setPosition(sf::Vector2f(191.f, 382.f));
    text_remote_ip_.setString("None");

    text_remote_port_.setFont(font_);
    text_remote_port_.setCharacterSize(20);
    text_remote_port_.setPosition(sf::Vector2f(191.f, 416.f));
    text_remote_port_.setString("None");

    arrowCursor_.loadFromSystem(sf::Cursor::Arrow);
    handCursor_.loadFromSystem(sf::Cursor::Hand);
}

void Desk::run()
{
    init();

    while (window_.isOpen()) {
        handle_events();
        render();
    }
}

void Desk::handle_events()
{
    sf::Event event_;
    while (window_.pollEvent(event_)) {
        if (event_.type == sf::Event::Closed) {
            window_.close();
        }

        handle_events_buttons(event_);
        dragging_window(event_);
        if (state_ == WindowState::Connect || 
            state_ == WindowState::Demonstration ||
            state_ == WindowState::Settings) {
            input_ip_->handle_event(event_, window_);
            input_port_->handle_event(event_, window_);
        }
    }
}

void Desk::handle_events_buttons(const sf::Event& event_)
{
    for (auto& button : buttons_) {
        if (state_ == WindowState::MainMenu) {
            if (button.get_category() == static_cast<size_t>(CategoryButton::Settings)) {
                if (button.is_hovered()) {
                    window_.setMouseCursor(handCursor_);
                }
                else {
                    window_.setMouseCursor(arrowCursor_);
                }
            }
        }
        if (state_ == WindowState::Connect ||
            state_ == WindowState::Demonstration ||
            state_ == WindowState::Settings) {
            if (button.get_category() == static_cast<size_t>(CategoryButton::Back)) {
                if (button.is_hovered()) {
                    window_.setMouseCursor(handCursor_);
                }
                else {
                    window_.setMouseCursor(arrowCursor_);
                }
            }
        }

        bool click_ = false;
        button.handle_event(event_, window_);
        if (button.is_clicked()) {
            if (state_ == WindowState::MainMenu) {
                if (button.get_category() == static_cast<size_t>(CategoryButton::Close)) {
                    window_.close();
                    click_ = true;
                }
                else if (button.get_category() == static_cast<size_t>(CategoryButton::RollUp)) {
                    ShowWindow(window_.getSystemHandle(), SW_MINIMIZE);
                    click_ = true;
                }
                if (button.get_category() == static_cast<size_t>(CategoryButton::Connect)) {
                    is_connect_window();
                    click_ = true;
                }
                else if (button.get_category() == static_cast<size_t>(CategoryButton::Demonstration)) {
                    is_demonstration_window();
                    click_ = true;
                }
                else if (button.get_category() == static_cast<size_t>(CategoryButton::Settings)) {
                    is_settings_window();
                    click_ = true;
                }
            }
            else if (state_ == WindowState::Connect) {
                if (button.get_category() == static_cast<size_t>(CategoryButton::Close)) {
                    window_.close();
                    click_ = true;
                }
                else if (button.get_category() == static_cast<size_t>(CategoryButton::RollUp)) {
                    ShowWindow(window_.getSystemHandle(), SW_MINIMIZE);
                    click_ = true;
                }
                if (button.get_category() == static_cast<size_t>(CategoryButton::Connect)) {
                    network_thread(false);
                    is_session_window();
                    click_ = true;
                }
                else if (button.get_category() == static_cast<size_t>(CategoryButton::Back)) {
                    is_main_window();
                    click_ = true;
                }
            }
            else if (state_ == WindowState::Demonstration) {
                if (button.get_category() == static_cast<size_t>(CategoryButton::Close)) {
                    window_.close();
                    click_ = true;
                }
                else if (button.get_category() == static_cast<size_t>(CategoryButton::RollUp)) {
                    ShowWindow(window_.getSystemHandle(), SW_MINIMIZE);
                    click_ = true;
                }
                if (button.get_category() == static_cast<size_t>(CategoryButton::Demonstration)) {
                    network_thread(true);
                    is_session_window();
                    click_ = true;
                }
                else if (button.get_category() == static_cast<size_t>(CategoryButton::Back)) {
                    is_main_window();
                    click_ = true;
                }
            }
            else if (state_ == WindowState::Settings) {
                if (button.get_category() == static_cast<size_t>(CategoryButton::Close)) {
                    window_.close();
                    click_ = true;
                }
                else if (button.get_category() == static_cast<size_t>(CategoryButton::RollUp)) {
                    ShowWindow(window_.getSystemHandle(), SW_MINIMIZE);
                    click_ = true;
                }
                if (button.get_category() == static_cast<size_t>(CategoryButton::Apply)) {
                    apply();
                    click_ = true;
                }
                else if (button.get_category() == static_cast<size_t>(CategoryButton::Back)) {
                    is_main_window();
                    click_ = true;
                }
            }
            else if (state_ == WindowState::Session) {
                if (button.get_category() == static_cast<size_t>(CategoryButton::Stop)) {
                    if (thread_.joinable()) {
                        if (network_) network_->stop();
                        thread_.join();
                        network_.reset();
                    }
                    is_main_window();
                    click_ = true;
                }
            }
            button.set_state(State::Normal);
            if (click_) {
                return;
            }
        }
    }
}

void Desk::render()
{
    window_.clear(sf::Color::Black);

    window_.draw(background_S_);

    for (auto& button : buttons_) {
        if (button.get_category() == static_cast<size_t>(CategoryButton::Connect)) {
            if (state_ == WindowState::MainMenu || state_ == WindowState::Connect) {
                button.draw(window_);
            }
        }
        else if (button.get_category() == static_cast<size_t>(CategoryButton::Demonstration)) {
            if (state_ == WindowState::MainMenu || state_ == WindowState::Demonstration) {
                button.draw(window_);
            }
        }
        else if (button.get_category() == static_cast<size_t>(CategoryButton::Settings)) {
            if (state_ == WindowState::MainMenu) {
                button.draw(window_);
            }
        }
        else if (button.get_category() == static_cast<size_t>(CategoryButton::Back)) {
            if (state_ == WindowState::Connect || 
                state_ == WindowState::Demonstration ||
                state_ == WindowState::Settings) {
                button.draw(window_);
            }
        }
        else if (button.get_category() == static_cast<size_t>(CategoryButton::Stop)) {
            if (state_ == WindowState::Session) {
                button.draw(window_);
            }
        }
        else if (button.get_category() == static_cast<size_t>(CategoryButton::Apply)) {
            if (state_ == WindowState::Settings) {
                button.draw(window_);
            }
        }
        else {
            button.draw(window_);
        }
    }

    if (state_ == WindowState::Connect ||
        state_ == WindowState::Demonstration || 
        state_ == WindowState::Settings) {
        input_ip_->draw(window_);
        input_port_->draw(window_);
    }

    if (state_ == WindowState::MainMenu ||
        state_ == WindowState::Session) {
        window_.draw(text_local_ip_);
        window_.draw(text_local_port_);
        if (state_ == WindowState::Session) {
            window_.draw(text_remote_ip_);
            window_.draw(text_remote_port_);
        }
    }

    window_.display();
}

void Desk::dragging_window(const sf::Event& event_)
{
    if (event_.type == sf::Event::MouseButtonPressed &&
        event_.mouseButton.button == sf::Mouse::Left) {
        bool isButtonCursor = false;
        for (auto& button : buttons_) {
            if (button.is_hovered()) {
                isButtonCursor = true;
            }
        }

        if (!isButtonCursor && event_.mouseButton.y < 41) {
            dragging_window_ = true;
            sf::Vector2i mousePos = sf::Mouse::getPosition();
            HWND hwnd = window_.getSystemHandle();

            RECT rect;
            GetWindowRect(hwnd, &rect);

            dragOffset_window_.x = mousePos.x - rect.left;
            dragOffset_window_.y = mousePos.y - rect.top;
        }
    }

    if (event_.type == sf::Event::MouseButtonReleased &&
        event_.mouseButton.button == sf::Mouse::Left) {
        dragging_window_ = false;
    }

    if (event_.type == sf::Event::MouseMoved && dragging_window_) {
        sf::Vector2i mousePos = sf::Mouse::getPosition();
        HWND hwnd = window_.getSystemHandle();

        MoveWindow(
            hwnd,
            mousePos.x - dragOffset_window_.x,
            mousePos.y - dragOffset_window_.y,
            400, 600,
            TRUE
        );
    }
}

void Desk::is_main_window()
{
    input_ip_->set_value("");
    input_port_->set_value("");
    state_ = WindowState::MainMenu;
    background_S_.setTexture(backgrounds_T_[static_cast<size_t>(state_)]);
    for (auto& button : buttons_) {
        if (button.get_category() == static_cast<size_t>(CategoryButton::Connect)) {
            button.set_position(sf::Vector2f(117.f, 208.f));
        }
        else if (button.get_category() == static_cast<size_t>(CategoryButton::Demonstration)) {
            button.set_position(sf::Vector2f(84.f, 317.f));
        }
    }
    text_local_ip_.setPosition(sf::Vector2f(152.f, 492.f));
    text_local_port_.setPosition(sf::Vector2f(152.f, 526.f));
}

void Desk::is_connect_window()
{
    state_ = WindowState::Connect;
    background_S_.setTexture(backgrounds_T_[static_cast<size_t>(state_)]);
    for (auto& button : buttons_) {
        if (button.get_category() == static_cast<size_t>(CategoryButton::Connect)) {
            button.set_position(sf::Vector2f(193.f, 442.f));
            break;
        }
    }
}

void Desk::is_demonstration_window()
{
    state_ = WindowState::Demonstration;
    background_S_.setTexture(backgrounds_T_[static_cast<size_t>(state_)]);
    for (auto& button : buttons_) {
        if (button.get_category() == static_cast<size_t>(CategoryButton::Demonstration)) {
            button.set_position(sf::Vector2f(128.f, 442.f));
            break;
        }
    }
}

void Desk::is_settings_window()
{
    state_ = WindowState::Settings;
    background_S_.setTexture(backgrounds_T_[static_cast<size_t>(state_)]);
}

void Desk::is_session_window()
{
    state_ = WindowState::Session;
    background_S_.setTexture(backgrounds_T_[static_cast<size_t>(state_)]);
    text_local_ip_.setPosition(sf::Vector2f(192.f, 314.f));
    text_local_port_.setPosition(sf::Vector2f(192.f, 348.f));
}

void Desk::apply() 
{
    std::string ipValue = input_ip_->get_value();
    std::string portValue = input_port_->get_value();

    if (!ipValue.empty()) {
        local_ip_ = ipValue;
        text_local_ip_.setString(ipValue);
    }

    if (!portValue.empty()) {
        local_port_ = portValue.empty() ? 0 : std::stoi(portValue);
        text_local_port_.setString(portValue);
    }
}

void Desk::network_thread(bool demonstration)
{
    if (thread_.joinable()) {
        if (network_) network_->stop();
        thread_.join();
        network_.reset();
    }

    remote_ip_ = input_ip_->get_value();
    remote_port_ = std::atoi(input_port_->get_value().c_str());

    if (remote_ip_.empty() || remote_port_ == 0) {
        text_remote_ip_.setString("None");
        text_remote_port_.setString("None");
        return;
    }

    text_remote_ip_.setString(remote_ip_);
    text_remote_port_.setString(std::to_string(remote_port_));

    network_ = std::make_shared<Network>();
    thread_ = std::thread(
        &Network::start,
        network_,
        demonstration,
        local_ip_,
        local_port_,
        remote_ip_,
        remote_port_
    );
}