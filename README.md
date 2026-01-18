# SentinelLite – Ransomware Early Detection System

SentinelLite is a lightweight Windows-based ransomware early detection system developed as a Data Structures project.  
It monitors real-time file system activity and detects ransomware behavior patterns using a sliding window queue mechanism.

The project includes a **main detection system** and a **simulator** for testing purposes.

---

```
SentinelLite-Ransomware-Detection/
├── src/
│ └── SentinelLite.cpp 
├── simulator/
│ └── FileActivitySimulator.cpp 
├── README.md

```

---

## Features

- Real-time directory monitoring using Windows API
- Sliding window event analysis (queue-based)
- Detection of ransomware behavior patterns:
  - High-frequency file modifications
  - Mass rename and modify attacks
  - Random encryption-like file extensions
  - Ransom note identification
- GUI-based alert and status monitoring
- Multi-threaded file monitoring

---

## Data Structures Used

- Circular Queue
- Sliding Time Window
- Counters and Flags for Pattern Detection

These structures enable efficient real-time analysis without heavy resource usage.

---

## Detection Logic Overview

- File events are pushed into a queue with timestamps
- Events older than a fixed time window are removed
- Detection engine analyzes patterns such as:
  - Number of modifications
  - Rename frequency
  - Suspicious extensions
  - Presence of ransom notes
- Alerts are triggered when thresholds are exceeded

---

## Simulator Overview

The `simulator` folder contains a **FileActivitySimulator**, which can generate controlled file activity to test the detection engine:

- Simulates file modifications, renaming, and suspicious file creations
- Helps validate that alerts trigger correctly
- Designed to run separately from the main detection system

---

## How to Run

### Main Detection System

1. Open the project in a Windows-supported C++ compiler (Visual Studio recommended)
2. Ensure the monitored directory path exists: C:\Users\hp\Documents\TestMonitor
3. 3. Compile and run `SentinelLite.cpp`
4. Perform file operations in the monitored folder to trigger alerts

### Simulator 

1. Compile `FileActivitySimulator.cpp` in Visual Studio
2. Run the simulator to generate sample file events in the monitored folder
3. Observe alerts in the SentinelLite GUI

---

## Future Improvements

- Process-based detection
- Automatic response actions
- Log export functionality
- MITRE ATT&CK mapping
- Support for multiple monitored directories

---

## Technologies Used

- C++
- Windows API (Win32)
- Multithreading
- File System Monitoring
- GUI using Win32 controls

---

## Author

Rumasa Rasool  
Cybersecurity & Blue Team Enthusiast


