# Arduino Prayer Times LCD

This project displays **Islamic prayer times** on a 16x2 LCD connected to an Arduino Uno.  
A companion **PowerShell 7 script** fetches daily prayer times from the [Aladhan API](https://aladhan.com/prayer-times-api) and streams them to the Arduino over USB.  

The Arduino shows:
- **Line 1** → Current time (HH:MM:SS)  
- **Line 2** → Next prayer name, its time, and a countdown (T-hh:mm)  

---

## 📂 Files

- **PrayerTimes.ino**  
  Arduino sketch for the Uno. Handles LCD display, parsing serial input, and showing prayer times with countdowns.  

- **prayertimes.ps1**  
  PowerShell 7 script. Fetches today’s prayer times for your city, sends them to Arduino, and updates the current clock every second.  

---

## ⚙️ Hardware Requirements

- Arduino Uno (or compatible board)  
- 16x2 LCD (HD44780-compatible)  
- Breadboard + jumper wires  
- Potentiometer (≈10kΩ) for contrast adjustment  
- Resistor (≈220Ω) for LCD backlight (optional but recommended)  

---

## 💻 Software Requirements

- [Arduino IDE](https://www.arduino.cc/en/software)  
- [PowerShell 7](https://github.com/PowerShell/PowerShell)  
- Internet connection (to fetch prayer times)  

---

## 🚀 Setup & Usage

1. **Upload the Arduino sketch**  
   - Open `PrayerTimes.ino` in Arduino IDE  
   - Select your board & port → Upload  

2. **Edit the PowerShell script**  
   - Open `prayertimes.ps1` in a text editor  
   - Change these lines to match your setup:  
     ```powershell
     $comPort = "COM5"       # your Arduino port (check Arduino IDE → Tools → Port)
     $city    = "London"     # your city
     $country = "United Kingdom"
     $method  = 3            # calculation method (e.g., 2=ISNA, 3=MWL, 5=Egyptian, etc.)
     ```

3. **Run the script in PowerShell 7**  
   ```powershell
   pwsh -ExecutionPolicy Bypass -File .\prayertimes.ps1
