#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>
#include <string>
#include <sstream>
#include <cctype>

// Widget

class Widget
{
public:
	virtual ~Widget() = default;
    virtual size_t get_category() const = 0;
    virtual bool set_category(const size_t category) = 0;
    virtual void handle_event(const sf::Event& event, const sf::RenderWindow& window) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual sf::Vector2f get_position() const = 0;
    virtual bool set_position(const sf::Vector2f& position) = 0;
    virtual sf::FloatRect bounds() const = 0;
};


// Button

enum class State { Normal, Hovered, Pressed };

class Button : public Widget
{
public:
    Button(
        const size_t category,
        const sf::Vector2f& position,
        const std::string& texNormalPath,
        const std::string& texHoverPath,
        const std::string& texPressedPath
    );

public:
    size_t get_category() const override;
    bool set_category(const size_t category) override;
    virtual void handle_event(const sf::Event& event, const sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;
    sf::Vector2f get_position() const override;
    bool set_position(const sf::Vector2f& position) override;
    sf::FloatRect bounds() const override;
    State get_state() const;
    bool set_state(State new_state);
    bool is_clicked();
    bool is_hovered() const;
    virtual void apply_texture_for_state();

protected:
    size_t category;
    sf::Sprite sprite_;
    sf::Texture texNormal_;
    sf::Texture texHover_;
    sf::Texture texPressed_;

    State state_{ State::Normal };
    bool pressedInside_{ false };
    bool clickedEdge_{ false };
};


// CheckButton

class CheckButton : public Button
{
public:
    CheckButton(
        const size_t category,
        const sf::Vector2f& position,
        const std::string& texNormalPath,
        const std::string& texHoverPath,
        const std::string& texPressedPath
    );

public:
    void handle_event(const sf::Event& event, const sf::RenderWindow& window) override;
    void apply_texture_for_state() override;
    bool is_checked() const noexcept;
    void set_checked(bool v);

protected:
    bool checked_{ false };
};


// IpInput

class IpInput : public Widget 
{
public:
    IpInput(
        size_t category,
        const sf::Vector2f& position,
        const std::string& texNormalPath,
        const std::string& texHoverPath,
        const std::string& texFocusedPath,
        const sf::Font& font,
        const std::string& firstText,
        unsigned int charSize = 20,
        const sf::Vector2f& padding = { 17.f, 17.f },
        sf::Color placeholderColor = sf::Color(150, 150, 150),
        sf::Color textColor = sf::Color::White
    );

public:
    size_t get_category() const override;
    bool set_category(const size_t c) override;
    void handle_event(const sf::Event& event, const sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;
    sf::Vector2f get_position() const override;
    bool set_position(const sf::Vector2f& p) override;
    sf::FloatRect bounds() const override;
    std::string get_value() const;
    void set_value(const std::string& v);
    bool is_valid_ipv4() const;

private:
    void setState(State s);
    bool isPlaceholder() const;
    void onBufferChanged();
    void updateTextDrawable();
    void refreshTextPos();
    void clampDots();
    static bool validateIPv4(const std::string& s);
    static bool parsePart(const std::string& s, int& out);
    void pasteFromClipboard();

private:
    size_t category_;
    State state_;

    sf::Texture texNormal_, texHover_, texFocused_;
    sf::Sprite  sprite_;

    sf::Text text_;
    std::string buffer_;
    std::string placeholder_;
    sf::Color placeholderColor_;
    sf::Color textColor_;
    unsigned int charSize_;
    sf::Vector2f padding_;
};


// PortInput

class PortInput : public Widget 
{
public:
    enum class State { Normal, Hovered, Focused };

    PortInput(
        size_t category,
        const sf::Vector2f& position,
        const std::string& texNormalPath,
        const std::string& texHoverPath,
        const std::string& texFocusedPath,
        const sf::Font& font,
        const std::string& firstText,
        unsigned int charSize = 20,
        const sf::Vector2f& padding = { 17.f, 17.f },
        sf::Color placeholderColor = sf::Color(150, 150, 150),
        sf::Color textColor = sf::Color::White
    );

public:
    size_t get_category() const override;
    bool set_category(const size_t c) override;
    void handle_event(const sf::Event& event, const sf::RenderWindow& window) override;
    void draw(sf::RenderWindow& window) override;
    sf::Vector2f get_position() const override;
    bool set_position(const sf::Vector2f& p) override;
    sf::FloatRect bounds() const override;
    std::string get_value() const;
    void set_value(const std::string& v);
    bool is_valid_port() const;
    unsigned short value_or(uint16_t fallback = 0) const;

private:
    void setState(State s);
    bool isPlaceholder() const;
    void onBufferChanged();
    void updateTextDrawable();
    void refreshTextPos();
    static bool isValueOk(const std::string& s);
    void pasteFromClipboard();

private:
    size_t category_;
    State state_;

    sf::Texture texNormal_, texHover_, texFocused_;
    sf::Sprite  sprite_;

    sf::Text text_;
    std::string buffer_;
    std::string placeholder_;
    sf::Color placeholderColor_;
    sf::Color textColor_;
    unsigned int charSize_;
    sf::Vector2f padding_;
};