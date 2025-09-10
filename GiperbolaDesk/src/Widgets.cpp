#include "../include/Widgets.hpp"


// Button

Button::Button(
    const size_t category,
    const sf::Vector2f& position,
    const std::string& texNormalPath,
    const std::string& texHoverPath,
    const std::string& texPressedPath)
    : category(category), pressedInside_(false), clickedEdge_(false)
{
    texNormal_.loadFromFile(texNormalPath);
    texHover_.loadFromFile(texHoverPath);
    texPressed_.loadFromFile(texPressedPath);

    state_ = State::Normal;
    sprite_.setTexture(texNormal_, true);
    sprite_.setPosition(position);
}

size_t Button::get_category() const
{
    return category;
}

bool Button::set_category(const size_t category)
{
    this->category = category;
    return true;
}

void Button::handle_event(const sf::Event& event, const sf::RenderWindow& window)
{
    clickedEdge_ = false;

    auto containsMouse = [&](sf::Vector2f pt) {
        return sprite_.getGlobalBounds().contains(pt);
        };

    switch (event.type)
    {
    case sf::Event::MouseMoved:
    {
        if (!pressedInside_) {
            auto mp = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            set_state(containsMouse(mp) ? State::Hovered : State::Normal);
        }
        break;
    }
    case sf::Event::MouseButtonPressed:
    {
        if (event.mouseButton.button == sf::Mouse::Left) {
            auto mp = window.mapPixelToCoords({ event.mouseButton.x, event.mouseButton.y });
            if (containsMouse(mp)) {
                pressedInside_ = true;
                set_state(State::Pressed);
            }
        }
        break;
    }
    case sf::Event::MouseButtonReleased:
    {
        if (event.mouseButton.button == sf::Mouse::Left) {
            auto mp = window.mapPixelToCoords({ event.mouseButton.x, event.mouseButton.y });
            bool inside = containsMouse(mp);
            if (pressedInside_) {
                if (inside) clickedEdge_ = true;
                set_state(inside ? State::Hovered : State::Normal);
            }
            else {
                set_state(inside ? State::Hovered : State::Normal);
            }
            pressedInside_ = false;
        }
        break;
    }
    default: break;
    }
}

void Button::draw(sf::RenderWindow& window)
{
    window.draw(sprite_);
}

sf::Vector2f Button::get_position() const
{
    return sprite_.getPosition();
}

bool Button::set_position(const sf::Vector2f& position)
{
    sprite_.setPosition(position);
    return true;
}

State Button::get_state() const
{ 
    return state_;
}

bool Button::set_state(State new_state)
{
    if (state_ == new_state) return true;
    state_ = new_state;
    apply_texture_for_state();
    return true;
}

bool Button::is_clicked()
{
    bool out = clickedEdge_;
    clickedEdge_ = false;
    return out;
}

bool Button::is_hovered() const
{
    return state_ == State::Hovered;
}

sf::FloatRect Button::bounds() const
{ 
    return sprite_.getGlobalBounds(); 
}

void Button::apply_texture_for_state()
{
    switch (state_)
    {
    case State::Normal:     sprite_.setTexture(texNormal_, true); break;
    case State::Hovered:    sprite_.setTexture(texHover_, true); break;
    case State::Pressed:    sprite_.setTexture(texPressed_, true); break;
    }
}


// CheckButton

CheckButton::CheckButton(
    const size_t category,
    const sf::Vector2f& position,
    const std::string& texNormalPath,
    const std::string& texHoverPath,
    const std::string& texPressedPath
)
    : Button(category, position, texNormalPath, texHoverPath, texPressedPath) {
}

void CheckButton::handle_event(const sf::Event& event, const sf::RenderWindow& window)
{
    Button::handle_event(event, window);

    if (is_clicked()) {
        checked_ = !checked_;
        apply_texture_for_state();
    }
}

void CheckButton::apply_texture_for_state()
{
    if (checked_) {
        switch (get_state()) {
        case State::Normal:  sprite_.setTexture(texPressed_, true); break;
        case State::Hovered: sprite_.setTexture(texHover_, true);  break;
        case State::Pressed: sprite_.setTexture(texPressed_, true); break;
        }
        return;
    }

    switch (get_state()) {
    case State::Normal:  sprite_.setTexture(texNormal_, true); break;
    case State::Hovered: sprite_.setTexture(texHover_, true); break;
    case State::Pressed: sprite_.setTexture(texPressed_, true); break;
    }
}

bool CheckButton::is_checked() const noexcept
{ 
    return checked_; 
}

void CheckButton::set_checked(bool v)
{
    if (checked_ == v) return;
    checked_ = v;
    apply_texture_for_state();
}


// IpInput

IpInput::IpInput(
    size_t category,
    const sf::Vector2f& position,
    const std::string& texNormalPath,
    const std::string& texHoverPath,
    const std::string& texFocusedPath,
    const sf::Font& font,
    const std::string& firstText,
    unsigned int charSize,
    const sf::Vector2f& padding,
    sf::Color placeholderColor,
    sf::Color textColor
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

size_t IpInput::get_category() const
{ 
    return category_; 
}

bool IpInput::set_category(const size_t c)
{ 
    category_ = c; 
    return true; 
}

void IpInput::handle_event(const sf::Event& event, const sf::RenderWindow& window)
{
    auto contains = [&](sf::Vector2f pt) { return sprite_.getGlobalBounds().contains(pt); };

    switch (event.type) {
    case sf::Event::MouseMoved: {
        if (state_ != State::Pressed) {
            auto mp = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            setState(contains(mp) ? State::Hovered : State::Normal);
        }
        break;
    }
    case sf::Event::MouseButtonPressed: {
        if (event.mouseButton.button == sf::Mouse::Left) {
            auto mp = window.mapPixelToCoords({ event.mouseButton.x, event.mouseButton.y });
            if (contains(mp)) {
                setState(State::Pressed);
            }
            else {
                setState(State::Normal);
            }
        }
        break;
    }
    case sf::Event::KeyPressed: {
        if (state_ != State::Pressed) break;

        if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Escape) {
            setState(State::Normal);
        }
        if (event.key.control && (event.key.code == sf::Keyboard::V)) {
            pasteFromClipboard();
        }
        if (event.key.code == sf::Keyboard::Delete) {
            buffer_.clear();
            onBufferChanged();
        }
        break;
    }
    case sf::Event::TextEntered: {
        if (state_ != State::Pressed) break;
        uint32_t uni = event.text.unicode;

        if (uni == 8) {
            if (!buffer_.empty()) {
                buffer_.pop_back();
                onBufferChanged();
            }
            break;
        }

        if (uni >= 32 && uni < 127) {
            char ch = static_cast<char>(uni);
            if (std::isdigit(static_cast<unsigned char>(ch))) {
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
                    buffer_.push_back(ch);
                    onBufferChanged();
                }
                else {
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
                            if (startNewOctet()) {
                                buffer_.push_back(ch);
                                onBufferChanged();
                            }
                        }
                    }
                    else {
                        if (startNewOctet()) {
                            buffer_.push_back(ch);
                            onBufferChanged();
                        }
                    }
                }
            }
            else if (ch == '.') {
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

void IpInput::draw(sf::RenderWindow& window)
{
    window.draw(sprite_);
    window.draw(text_);
}

sf::Vector2f IpInput::get_position() const 
{ 
    return sprite_.getPosition();
}

bool IpInput::set_position(const sf::Vector2f& p)
{
    sprite_.setPosition(p);
    refreshTextPos();
    return true;
}

sf::FloatRect IpInput::bounds() const 
{ 
    return sprite_.getGlobalBounds(); 
}

std::string IpInput::get_value() const
{ 
    return buffer_; 
}

void IpInput::set_value(const std::string& v)
{
    buffer_ = v.substr(0, 15);
    onBufferChanged();
}

bool IpInput::is_valid_ipv4() const
{ 
    return validateIPv4(buffer_); 
}

void IpInput::setState(State s)
{
    state_ = s;
    switch (state_) {
    case State::Normal:  sprite_.setTexture(texNormal_, true);  break;
    case State::Hovered: sprite_.setTexture(texHover_, true);   break;
    case State::Pressed: sprite_.setTexture(texFocused_, true); break;
    }
    updateTextDrawable();
}

bool IpInput::isPlaceholder() const
{ 
    return buffer_.empty(); 
}

void IpInput::onBufferChanged()
{
    clampDots();
    updateTextDrawable();
}

void IpInput::updateTextDrawable()
{
    if (isPlaceholder()) {
        text_.setString(placeholder_);
        text_.setFillColor(placeholderColor_);
    }
    else {
        text_.setString(buffer_);
        text_.setFillColor(textColor_);
    }
}

void IpInput::refreshTextPos()
{
    auto pos = sprite_.getPosition();
    text_.setPosition(pos + padding_);
}

void IpInput::clampDots()
{
    int dots = 0;
    for (char c : buffer_) if (c == '.') ++dots;
    while (dots > 3) {
        if (buffer_.back() == '.') --dots;
        buffer_.pop_back();
    }
}

bool IpInput::validateIPv4(const std::string& s)
{
    if (s.empty()) return false;
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

bool IpInput::parsePart(const std::string& s, int& out)
{
    if (s.size() > 1 && s[0] == '0') return false;
    int v = 0;
    for (char c : s) v = v * 10 + (c - '0');
    if (v < 0 || v > 255) return false;
    out = v; return true;
}

void IpInput::pasteFromClipboard()
{
    std::string clip = sf::Clipboard::getString().toAnsiString();
    std::string filtered;
    filtered.reserve(clip.size());
    for (char c : clip) {
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') filtered.push_back(c);
    }
    for (char c : filtered) {
        if (buffer_.size() >= 15) break;
        if (c == '.' && (buffer_.empty() || buffer_.back() == '.')) continue;
        buffer_.push_back(c);
    }
    onBufferChanged();
}


// PortInput

PortInput::PortInput(
    size_t category,
    const sf::Vector2f& position,
    const std::string& texNormalPath,
    const std::string& texHoverPath,
    const std::string& texFocusedPath,
    const sf::Font& font,
    const std::string& firstText,
    unsigned int charSize,
    const sf::Vector2f& padding,
    sf::Color placeholderColor,
    sf::Color textColor
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

size_t PortInput::get_category() const { return category_; }

bool PortInput::set_category(const size_t c) { category_ = c; return true; }

void PortInput::handle_event(const sf::Event& event, const sf::RenderWindow& window) 
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

        if (uni == 8) {
            if (!buffer_.empty()) {
                buffer_.pop_back();
                onBufferChanged();
            }
            break;
        }

        if (uni >= 32 && uni < 127) {
            char ch = static_cast<char>(uni);
            if (std::isdigit(static_cast<unsigned char>(ch))) {
                if (buffer_.size() < 5) {
                    std::string cand = buffer_ + ch;
                    if (isValueOk(cand)) {
                        buffer_.push_back(ch);
                        onBufferChanged();
                    }
                }
            }
        }
        break;
    }
    default: break;
    }
}

void PortInput::draw(sf::RenderWindow& window)
{
    window.draw(sprite_);
    window.draw(text_);
}

sf::Vector2f PortInput::get_position() const { return sprite_.getPosition(); }

bool PortInput::set_position(const sf::Vector2f& p) {
    sprite_.setPosition(p);
    refreshTextPos();
    return true;
}

sf::FloatRect PortInput::bounds() const { return sprite_.getGlobalBounds(); }

std::string PortInput::get_value() const { return buffer_; }

void PortInput::set_value(const std::string& v) 
{
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

bool PortInput::is_valid_port() const 
{
    if (buffer_.empty()) return false;
    unsigned long val = 0;
    for (char c : buffer_) val = val * 10 + (c - '0');
    return val <= 65535;
}

unsigned short PortInput::value_or(uint16_t fallback) const 
{
    if (!is_valid_port()) return fallback;
    unsigned long val = 0;
    for (char c : buffer_) val = val * 10 + (c - '0');
    return static_cast<unsigned short>(val);
}

void PortInput::setState(State s) 
{
    state_ = s;
    switch (state_) {
    case State::Normal:  sprite_.setTexture(texNormal_, true);  break;
    case State::Hovered: sprite_.setTexture(texHover_, true);   break;
    case State::Focused: sprite_.setTexture(texFocused_, true); break;
    }
    updateTextDrawable();
}

bool PortInput::isPlaceholder() const { return buffer_.empty(); }

void PortInput::onBufferChanged() 
{
    updateTextDrawable();
}

void PortInput::updateTextDrawable() 
{
    if (isPlaceholder()) {
        text_.setString(placeholder_);
        text_.setFillColor(placeholderColor_);
    }
    else {
        text_.setString(buffer_);
        text_.setFillColor(textColor_);
    }
}

void PortInput::refreshTextPos() 
{
    text_.setPosition(sprite_.getPosition() + padding_);
}

bool PortInput::isValueOk(const std::string& s) 
{
    unsigned long val = 0;
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        val = val * 10 + (c - '0');
        if (val > 65535) return false;
    }
    return true;
}

void PortInput::pasteFromClipboard() 
{
    std::string clip = sf::Clipboard::getString().toAnsiString();

    for (char c : clip) {
        if (!std::isdigit(static_cast<unsigned char>(c))) continue;
        if (buffer_.size() >= 5) break;
        std::string cand = buffer_ + c;
        if (!isValueOk(cand)) break;
        buffer_.push_back(c);
    }
    onBufferChanged();
}