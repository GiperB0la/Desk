# 🚀 GiperbolaDesk — Remote Desktop over UDP

**GiperbolaDesk** is a lightweight remote desktop application built on top of **UDP**.  

One client takes screenshots of its desktop, encodes them into **JPEG**, and sends them to another client.  
The second client receives these frames and displays them in an **SFML window** in real time.  

But the magic doesn’t stop there:  
- when the second client’s user moves the mouse, clicks, or types, those events are sent back through the network;  
- the first client receives them and reproduces the actions using **WinAPI**;  
- the result is a fully functional **remote desktop control**, similar to AnyDesk or RDP.  

---

## ✨ Features

- 🖥️ User-friendly UI for configuring IPs and ports  
- ⚡ Frame transmission over **UDP** for minimal latency  
- 🔗 Simple client ↔ client architecture, where roles are defined at startup  
- 🎥 Two modes:  
  - **Screen Demonstration** — screen sharing only  
  - **Full Control** — remote mouse and keyboard control  

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
