# BCI-EMG-Project - Wireless EMG-Based Armband for Wheelchair Control  

## 🏥 Project Overview <a href="https://drive.google.com/file/d/1yrmmO3o-nB0ciMY7XeSuRiaIwhBL2Qsh/view?usp=sharing" target="_blank">(Link to Project Report)</a>
This project focuses on developing a **wireless, portable EMG-based armband** designed to control a wheelchair, providing an **intuitive and accessible Human-Machine Interface (HMI)**. The goal is to enhance **independence and mobility** for individuals with physical disabilities.  

By utilizing **electromyographic (EMG) signals**, the system captures electrical activity from **skeletal muscles** and processes these signals to control a **robotic wheelchair**.  


<video src="./setup-video.mov" width="320" height="480" controls></video>

[Download Setup-video.mp4](https://github.com/Smritigit1202/BCI-EMG-Project/raw/refs/heads/main/setup-video.mov)

## Video Outputs




https://github.com/user-attachments/assets/da2c0cc7-18c0-4018-9d33-387b8fa58bd6



https://github.com/user-attachments/assets/d63f5274-631b-4779-89f4-2e5aad58c1cd


## 🔧 Key Components & Technology  
- **EMG Signal Acquisition:** Custom-designed armband using **Dry Electrodes**.  
- **Signal Processing:**  
  - **Bioamp** for amplifying EMG signals.  
  - **Band-pass filter** to isolate EMG signals from unwanted noise.  
  - **Envelope smoothing algorithm** to enhance signal accuracy.  
- **Wireless Communication:** **ESP32 microcontrollers** for real-time data transmission.  
- **Motor Control:** Translated muscle signals provide **directional commands** for wheelchair movement.  
<img width="341" alt="image" src="https://github.com/user-attachments/assets/de6cd603-c5a8-4f9b-ba6e-f16c89ba65b0" />

## 🎯 System Capabilities  
- **Hands-free wheelchair control** for users with limited mobility.  
- **Real-time, wireless EMG signal transmission** using ESP32.  
- **Adaptive & Customizable:** Can be tuned to individual muscle movements.  

## 🚗 Initial Prototype Testing  
- A **toy car prototype** was used to validate:  
  - **Signal processing accuracy**  
  - **Noise filtering techniques**  
  - **Smooth motor control implementation**  

## 🔬 Future Work  
- **Advanced noise reduction techniques** for improved signal clarity.  
- **Integration of adaptive control algorithms** to learn user-specific muscle signals over time.  
- **Real-world testing** in diverse environments (indoor & outdoor).  
- **Scalability & Affordability:** Making the solution accessible for **global deployment**.  

## 🌍 Impact & Vision  
This project aims to **Empower individuals with disabilities** by providing them with a **scalable, cost-effective, and accessible** mobility solution. It serves as a strong foundation for **future advancements in assistive technology**, showcasing the potential of **EMG-based control systems** in creating a more inclusive world.  

---
