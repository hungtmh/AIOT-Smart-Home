import os
import requests
import numpy as np
import cv2
from flask import Flask, request, jsonify
from ultralytics import YOLO

app = Flask(__name__)

# Load YOLOv8 model (ensure you have a model trained for fire detection, e.g., best.pt)
# For demonstration, we use yolov8n.pt, but you should replace it with a fire-specific model.
MODEL_PATH = "yolov8n.pt" 
try:
    model = YOLO(MODEL_PATH)
except Exception as e:
    print(f"Warning: Could not load YOLO model from {MODEL_PATH}. Exception: {e}")
    model = None

SPRING_BOOT_URL = "http://localhost:8080/api/alerts/fire"

@app.route('/upload', methods=['POST'])
def upload_image():
    if request.content_type != 'image/jpeg':
        return jsonify({"error": "Invalid content type"}), 400

    image_bytes = request.data
    if not image_bytes:
        return jsonify({"error": "Empty body"}), 400

    if model is None:
        return jsonify({"error": "Model not loaded"}), 500

    # Convert bytes to numpy array for cv2
    nparr = np.frombuffer(image_bytes, np.uint8)
    img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)

    if img is None:
        return jsonify({"error": "Invalid image"}), 400

    # Run YOLOv8 inference
    results = model(img)
    fire_detected = False

    # Check if fire class is detected 
    # (Assuming class 'fire' is in the model's names dict. Adjust class ID as needed).
    for result in results:
        for box in result.boxes:
            class_id = int(box.cls[0])
            class_name = result.names[class_id].lower()
            if 'fire' in class_name or 'flame' in class_name:
                fire_detected = True
                break
        if fire_detected:
            break

    # If you don't have a specific fire model yet and want to test, you can mock this:
    # fire_detected = True 

    if fire_detected:
        print("Fire detected! Sending to Spring Boot backend...")
        try:
            # Send image to Spring Boot backend
            files = {'image': ('fire_alert.jpg', image_bytes, 'image/jpeg')}
            response = requests.post(SPRING_BOOT_URL, files=files)
            if response.status_code == 200:
                print("Successfully notified backend.")
            else:
                print(f"Backend responded with {response.status_code}: {response.text}")
        except requests.exceptions.RequestException as e:
            print(f"Failed to connect to backend: {e}")
    else:
        print("No fire detected. Ignoring image.")

    return jsonify({"status": "processed", "fire_detected": fire_detected}), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
