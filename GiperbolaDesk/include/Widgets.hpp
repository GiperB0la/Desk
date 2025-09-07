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

class IpInput : public Widget {
public:
    enum class State { Normal, Hovered, Focused };

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
    )
        : category_(category),
        state_(State::Normal),
        placeholder_(firstText),
        placeholderColor_(placeholderColor),
        textColor_(textColor),
        charSize_(charSize),
        padding_(padding)
    {
        texNormal_.loadFromFile(texNormalPath);
        texHover_.loadFromFile(texHoverPath);
        texFocused_.loadFromFile(texFocusedPath);
        sprite_.setTexture(texNormal_, true);
        sprite_.setPosition(position);

        text_.setFont(font);
        text_.setCharacterSize(charSize_);
        text_.setFillColor(placeholderColor_);
        text_.setString(placeholder_);

        refreshTextPos();
    }

    // === Widget API ===
    size_t get_category() const override { return category_; }
    bool set_category(const size_t c) override { category_ = c; return true; }

    void handle_event(const sf::Event& event, const sf::RenderWindow& window) override
    {
        auto contains = [&](sf::Vector2f pt) { return sprite_.getGlobalBounds().contains(pt); };

        switch (event.type) {
        case sf::Event::MouseMoved: {
            if (state_ != State::Focused) {
                auto mp = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                setState(contains(mp) ? State::Hovered : State::Normal);
            }
            break;
        }
        case sf::Event::MouseButtonPressed: {
            if (event.mouseButton.button == sf::Mouse::Left) {
                auto mp = window.mapPixelToCoords({ event.mouseButton.x, event.mouseButton.y });
                if (contains(mp)) {
                    setState(State::Focused);
                }
                else {
                    setState(State::Normal);
                }
            }
            break;
        }
        case sf::Event::KeyPressed: {
            if (state_ != State::Focused) break;

            // удобства: Esc/Enter снимают фокус
            if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Escape) {
                setState(State::Normal);
            }
            // Вставка Ctrl+V
            if (event.key.control && (event.key.code == sf::Keyboard::V)) {
                pasteFromClipboard();
            }
            // Delete — очищает всё (по желанию)
            if (event.key.code == sf::Keyboard::Delete) {
                buffer_.clear();
                onBufferChanged();
            }
            break;
        }
        case sf::Event::TextEntered: {
            if (state_ != State::Focused) break;
            uint32_t uni = event.text.unicode;

            if (uni == 8) { // Backspace
                if (!buffer_.empty()) {
                    buffer_.pop_back();
                    onBufferChanged();
                }
                break;
            }

            if (uni >= 32 && uni < 127) {
                char ch = static_cast<char>(uni);
                if (std::isdigit(static_cast<unsigned char>(ch))) {
                    // ⬇️ АВТО-ТОЧКА при переполнении октета
                    auto dotCount = static_cast<int>(std::count(buffer_.begin(), buffer_.end(), '.'));
                    auto lastDot = buffer_.find_last_of('.');
                    std::string seg = (lastDot == std::string::npos) ? buffer_ : buffer_.substr(lastDot + 1);

                    auto startNewOctet = [&]() {
                        if (!buffer_.empty() && buffer_.back() != '.' && dotCount < 3) {
                            buffer_.push_back('.');
                            ++dotCount;
                            return true;
                        }
                        return false;
                        };

                    // если уже 4-й октет — допускаем цифры только пока валидно (<=3 символов и <=255)
                    if (dotCount == 3) {
                        if (seg.size() < 3) {
                            std::string cand = seg + ch;
                            int val = std::stoi(cand);
                            if (val <= 255) {
                                buffer_.push_back(ch);
                                onBufferChanged();
                            }
                        }
                        break;
                    }

                    if (seg.empty()) {
                        // первая цифра октета
                        buffer_.push_back(ch);
                        onBufferChanged();
                    }
                    else {
                        // опционально: запрет ведущих нулей вида "0x" — сразу уходим в новый октет
                        if (seg == "0") {
                            if (startNewOctet()) {
                                buffer_.push_back(ch);
                                onBufferChanged();
                            }
                            break;
                        }

                        std::string cand = seg + ch;
                        if (cand.size() <= 3) {
                            int val = std::stoi(cand);
                            if (val <= 255) {
                                buffer_.push_back(ch);
                                onBufferChanged();
                            }
                            else {
                                // превысили 255 -> ставим точку и начинаем новый октет с этой цифры
                                if (startNewOctet()) {
                                    buffer_.push_back(ch);
                                    onBufferChanged();
                                }
                            }
                        }
                        else {
                            // длина октета стала бы 4 -> переносим на новый октет
                            if (startNewOctet()) {
                                buffer_.push_back(ch);
                                onBufferChanged();
                            }
                        }
                    }
                }
                else if (ch == '.') {
                    // точка вручную: не первая, не подряд, не больше 3
                    if (!buffer_.empty() && buffer_.back() != '.' &&
                        std::count(buffer_.begin(), buffer_.end(), '.') < 3) {
                        buffer_.push_back('.');
                        onBufferChanged();
                    }
                }
            }
            break;
        }
        default: break;
        }
    }

    void draw(sf::RenderWindow& window) override
    {
        window.draw(sprite_);
        window.draw(text_);
    }

    sf::Vector2f get_position() const override { return sprite_.getPosition(); }

    bool set_position(const sf::Vector2f& p) override {
        sprite_.setPosition(p);
        refreshTextPos();
        return true;
    }

    sf::FloatRect bounds() const override { return sprite_.getGlobalBounds(); }

    // === Доп. API ===
    std::string get_value() const { return buffer_; }
    void set_value(const std::string& v) {
        buffer_ = v.substr(0, 15);
        onBufferChanged();
    }

    bool is_valid_ipv4() const { return validateIPv4(buffer_); }

private:
    // === внутреннее ===
    void setState(State s) {
        state_ = s;
        switch (state_) {
        case State::Normal:  sprite_.setTexture(texNormal_, true);  break;
        case State::Hovered: sprite_.setTexture(texHover_, true);   break;
        case State::Focused: sprite_.setTexture(texFocused_, true); break;
        }
        // цвет текста (плейсхолдер/обычный) зависит только от наличия буфера
        updateTextDrawable();
    }

    bool isPlaceholder() const { return buffer_.empty(); }

    void onBufferChanged() {
        // отрезаем всё, что явно ломает простой формат: максимум 3 точки
        clampDots();
        updateTextDrawable();
    }

    void updateTextDrawable() {
        if (isPlaceholder()) {
            text_.setString(placeholder_);
            text_.setFillColor(placeholderColor_);
        }
        else {
            text_.setString(buffer_);
            text_.setFillColor(textColor_);
        }
    }

    void refreshTextPos() {
        auto pos = sprite_.getPosition();
        text_.setPosition(pos + padding_);
        // вертикально можно чуть центрировать по высоте текстуры
        // но без метрик шрифта оставим простой отступ
    }

    void clampDots() {
        // максимум 3 точки
        int dots = 0;
        for (char c : buffer_) if (c == '.') ++dots;
        // если больше 3 — обрежем справа до валидного количества
        while (dots > 3) {
            if (buffer_.back() == '.') --dots;
            buffer_.pop_back();
        }
    }

    static bool validateIPv4(const std::string& s) {
        // Быстрая проверка формата: a.b.c.d где 0-255
        if (s.empty()) return false;
        // не должно начинаться/заканчиваться точкой
        if (s.front() == '.' || s.back() == '.') return false;

        std::array<int, 4> parts{}; int idx = 0;
        std::string cur;
        for (char c : s) {
            if (c == '.') {
                if (cur.empty()) return false;
                if (idx >= 4) return false;
                if (!parsePart(cur, parts[idx])) return false;
                ++idx; cur.clear();
            }
            else {
                if (!std::isdigit(static_cast<unsigned char>(c))) return false;
                cur.push_back(c);
                if (cur.size() > 3) return false;
            }
        }
        if (cur.empty()) return false;
        if (idx != 3) return false;
        return parsePart(cur, parts[idx]);
    }

    static bool parsePart(const std::string& s, int& out) {
        // запрещаем лидирующие нули вида "01" (но "0" ок)
        if (s.size() > 1 && s[0] == '0') return false;
        int v = 0;
        for (char c : s) v = v * 10 + (c - '0');
        if (v < 0 || v > 255) return false;
        out = v; return true;
    }

    void pasteFromClipboard() {
        std::string clip = sf::Clipboard::getString().toAnsiString();
        // оставим только цифры и точки
        std::string filtered;
        filtered.reserve(clip.size());
        for (char c : clip) {
            if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') filtered.push_back(c);
        }
        // впишем с учетом лимита и простого правила двойных точек
        for (char c : filtered) {
            if (buffer_.size() >= 15) break;
            if (c == '.' && (buffer_.empty() || buffer_.back() == '.')) continue;
            buffer_.push_back(c);
        }
        onBufferChanged();
    }

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


#include <SFML/Graphics.hpp>
#include <string>
#include <algorithm>
#include <cctype>

class PortInput : public Widget {
public:
    enum class State { Normal, Hovered, Focused };

    PortInput(
        size_t category,
        const sf::Vector2f& position,
        const std::string& texNormalPath,
        const std::string& texHoverPath,
        const std::string& texFocusedPath,
        const sf::Font& font,
        const std::string& firstText,                 // placeholder
        unsigned int charSize = 20,
        const sf::Vector2f& padding = { 17.f, 17.f },
        sf::Color placeholderColor = sf::Color(150, 150, 150),
        sf::Color textColor = sf::Color::White
    )
        : category_(category),
        state_(State::Normal),
        placeholder_(firstText),
        placeholderColor_(placeholderColor),
        textColor_(textColor),
        charSize_(charSize),
        padding_(padding)
    {
        texNormal_.loadFromFile(texNormalPath);
        texHover_.loadFromFile(texHoverPath);
        texFocused_.loadFromFile(texFocusedPath);

        sprite_.setTexture(texNormal_, true);
        sprite_.setPosition(position);

        text_.setFont(font);
        text_.setCharacterSize(charSize_);
        text_.setFillColor(placeholderColor_);
        text_.setString(placeholder_);

        refreshTextPos();
    }

    // === Widget API ===
    size_t get_category() const override { return category_; }
    bool set_category(const size_t c) override { category_ = c; return true; }

    void handle_event(const sf::Event& event, const sf::RenderWindow& window) override {
        auto contains = [&](sf::Vector2f pt) { return sprite_.getGlobalBounds().contains(pt); };

        switch (event.type) {
        case sf::Event::MouseMoved: {
            if (state_ != State::Focused) {
                auto mp = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                setState(contains(mp) ? State::Hovered : State::Normal);
            }
            break;
        }
        case sf::Event::MouseButtonPressed: {
            if (event.mouseButton.button == sf::Mouse::Left) {
                auto mp = window.mapPixelToCoords({ event.mouseButton.x, event.mouseButton.y });
                if (contains(mp)) setState(State::Focused);
                else              setState(State::Normal);
            }
            break;
        }
        case sf::Event::KeyPressed: {
            if (state_ != State::Focused) break;

            if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Escape) {
                setState(State::Normal);
            }
            if (event.key.control && event.key.code == sf::Keyboard::V) {
                pasteFromClipboard();
            }
            if (event.key.code == sf::Keyboard::Delete) {
                buffer_.clear();
                onBufferChanged();
            }
            break;
        }
        case sf::Event::TextEntered: {
            if (state_ != State::Focused) break;
            uint32_t uni = event.text.unicode;

            if (uni == 8) { // Backspace
                if (!buffer_.empty()) {
                    buffer_.pop_back();
                    onBufferChanged();
                }
                break;
            }

            if (uni >= 32 && uni < 127) {
                char ch = static_cast<char>(uni);
                if (std::isdigit(static_cast<unsigned char>(ch))) {
                    // кандидат с новой цифрой
                    if (buffer_.size() < 5) {
                        std::string cand = buffer_ + ch;
                        if (isValueOk(cand)) {
                            buffer_.push_back(ch);
                            onBufferChanged();
                        }
                    }
                }
                // любые нецифры игнорим
            }
            break;
        }
        default: break;
        }
    }

    void draw(sf::RenderWindow& window) override {
        window.draw(sprite_);
        window.draw(text_);
    }

    sf::Vector2f get_position() const override { return sprite_.getPosition(); }

    bool set_position(const sf::Vector2f& p) override {
        sprite_.setPosition(p);
        refreshTextPos();
        return true;
    }

    sf::FloatRect bounds() const override { return sprite_.getGlobalBounds(); }

    // === Доп. API ===
    std::string get_value() const { return buffer_; }

    void set_value(const std::string& v) {
        buffer_.clear();
        for (char c : v) {
            if (!std::isdigit(static_cast<unsigned char>(c))) break;
            if (buffer_.size() == 5) break;
            std::string cand = buffer_ + c;
            if (!isValueOk(cand)) break;
            buffer_.push_back(c);
        }
        onBufferChanged();
    }

    bool is_valid_port() const {
        if (buffer_.empty()) return false;
        unsigned long val = 0;
        for (char c : buffer_) val = val * 10 + (c - '0');
        return val <= 65535;
    }

    // если нужен числовой порт:
    unsigned short value_or(uint16_t fallback = 0) const {
        if (!is_valid_port()) return fallback;
        unsigned long val = 0;
        for (char c : buffer_) val = val * 10 + (c - '0');
        return static_cast<unsigned short>(val);
    }

private:
    // === внутреннее ===
    void setState(State s) {
        state_ = s;
        switch (state_) {
        case State::Normal:  sprite_.setTexture(texNormal_, true);  break;
        case State::Hovered: sprite_.setTexture(texHover_, true);   break;
        case State::Focused: sprite_.setTexture(texFocused_, true); break;
        }
        updateTextDrawable();
    }

    bool isPlaceholder() const { return buffer_.empty(); }

    void onBufferChanged() {
        updateTextDrawable();
    }

    void updateTextDrawable() {
        if (isPlaceholder()) {
            text_.setString(placeholder_);
            text_.setFillColor(placeholderColor_);
        }
        else {
            text_.setString(buffer_);
            text_.setFillColor(textColor_);
        }
    }

    void refreshTextPos() {
        text_.setPosition(sprite_.getPosition() + padding_);
    }

    static bool isValueOk(const std::string& s) {
        // пустоту сюда не передаём
        // длина уже проверена вызывающим
        unsigned long val = 0;
        for (char c : s) {
            if (!std::isdigit(static_cast<unsigned char>(c))) return false;
            val = val * 10 + (c - '0');
            if (val > 65535) return false; // раннее отсечение
        }
        return true;
    }

    void pasteFromClipboard() {
        std::string clip = sf::Clipboard::getString().toAnsiString();

        // оставим только цифры и добавляем по одной, пока валидно
        for (char c : clip) {
            if (!std::isdigit(static_cast<unsigned char>(c))) continue;
            if (buffer_.size() >= 5) break;
            std::string cand = buffer_ + c;
            if (!isValueOk(cand)) break;
            buffer_.push_back(c);
        }
        onBufferChanged();
    }

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