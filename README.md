# 🖥️ GiperbolaDesk — Remote Desktop over UDP

**GiperbolaDesk** is a lightweight remote desktop application built on top of **UDP**.  

One client takes screenshots of its desktop, encodes them into **JPEG**, and sends them to another client.  
The second client receives these frames and displays them in an **SFML window** in real time.  

At the same time, user input (mouse and keyboard events) from the second client is sent back through the network.  
The first client receives these events and reproduces them with **WinAPI**, turning the app into a fully functional  
**remote desktop solution** — both screen sharing and full control are always enabled.  

---

## ✨ Features

- 🖥️ User-friendly UI for configuring IPs and ports  
- ⚡ Frame transmission over **UDP** for minimal latency  
- 🔗 Simple client ↔ client architecture, where roles are defined at startup  
- 🎮 Combined **screen streaming + remote control** in one mode (no switching required)  

---

## 📸 Screenshots

<p align="center">
  <img src="https://raw.githubusercontent.com/GiperB0la/Desk/main/Screens/Screen1.jpg" alt="Screen1" width="45%">
  <img src="https://raw.githubusercontent.com/GiperB0la/Desk/main/Screens/Screen2.jpg" alt="Screen2" width="45%">
</p>

<p align="center">
  <img src="https://raw.githubusercontent.com/GiperB0la/Desk/main/Screens/Screen3.jpg" alt="Screen3" width="45%">
  <img src="https://raw.githubusercontent.com/GiperB0la/Desk/main/Screens/Screen4.jpg" alt="Screen4" width="45%">
</p>

---

## 🛠️ Build

### 🔹 Prerequisites
- **CMake ≥ 3.16**  
- **C++17 compiler** (MSVC, g++, clang++)  
- **SFML 2.6.2** ([download](https://www.sfml-dev.org/download/sfml/2.6.2/))
- **OpenCV** (install via [vcpkg](https://github.com/microsoft/vcpkg) or from [official site](https://opencv.org/releases/))

---

```powershell
# Clone project
git clone https://github.com/GiperB0la/GiperbolaDesk.git
cd GiperbolaDesk
mkdir build && cd build

# Configure
cmake .. -DCMAKE_TOOLCHAIN_FILE="path/to/vcpkg/scripts/buildsystems/vcpkg.cmake" `-DSFML_DIR="path/to/SFML/lib/cmake/SFML"

# Build (Release version)
cmake --build . --config Release
```
