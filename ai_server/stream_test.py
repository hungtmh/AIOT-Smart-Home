import cv2
import numpy as np

# TODO: Replace with the actual IP address printed in the Arduino Serial Monitor
STREAM_URL = "http://192.168.1.100:81/"

print(f"Connecting to ESP32-CAM stream at: {STREAM_URL}")
cap = cv2.VideoCapture(STREAM_URL)

if not cap.isOpened():
    print(f"Error: Cannot open stream at {STREAM_URL}")
    print("Please check your Wi-Fi connection and ensure the ESP32-CAM is running.")
    exit()

print("Stream opened successfully. Press 'q' to quit.")

while True:
    ret, frame = cap.read()
    if not ret:
        print("Failed to grab frame from stream.")
        break
    
    # Show the raw frame from ESP32-CAM
    cv2.imshow("ESP32-CAM Video Stream", frame)
    
    # Press 'q' to exit
    if cv2.waitKey(1) & 0xFF == ord('q'):
        print("Exiting stream...")
        break

cap.release()
cv2.destroyAllWindows()
