import cv2
import time
import os
import numpy as np
import requests

from flask import Flask, request
from ultralytics import YOLO
import paho.mqtt.client as mqtt


# =========================
# CONFIG
# =========================

CONFIDENCE_THRESHOLD = 0.60

SEND_COOLDOWN = 10

DEVICE_ID = "camera1"


# Spring Boot API
BACKEND_URL = "http://localhost:8080/api/fire-alert"


# MQTT
mqtt_broker = "localhost"
mqtt_port = 1883
mqtt_topic = "esp32/fire_detection"


# Save images
IMAGE_SAVE_DIR = "fire_images"

os.makedirs(
    IMAGE_SAVE_DIR,
    exist_ok=True
)


fire_count = 0
last_fire_time = None
previous_alert_state = False
last_sent_time = 0



# =========================
# MQTT
# =========================

def on_connect(client, userdata, flags, rc):

    print(
        "MQTT Connected:",
        rc
    )


mqtt_client = mqtt.Client()

mqtt_client.on_connect = on_connect


try:

    mqtt_client.connect(
        mqtt_broker,
        mqtt_port,
        60
    )

    mqtt_client.loop_start()

    print(
        "MQTT ready"
    )


except Exception as e:

    print(
        "MQTT error:",
        e
    )



# =========================
# YOLO
# =========================

print("Loading YOLO...")

model = YOLO(
    "best_320.pt"
)

print(
    "YOLO loaded"
)

print(
    "Classes:",
    model.names
)



app = Flask(__name__)


session = requests.Session()



# =========================
# SEND FIRE ALERT
# =========================

def send_fire_alert(frame, confidence):

    try:

        filename = (
            f"fire_{int(time.time())}.jpg"
        )


        filepath = os.path.join(
            IMAGE_SAVE_DIR,
            filename
        )


        cv2.imwrite(
            filepath,
            frame
        )


        _, encoded = cv2.imencode(
            ".jpg",
            frame
        )


        files = {

            "image":
            (
                filename,
                encoded.tobytes(),
                "image/jpeg"
            )

        }


        data = {

            "deviceId": DEVICE_ID,

            "confidence": confidence,

            "imagePath": filepath

        }


        print("\n==========================")
        print("SENDING FIRE ALERT")
        print("Device:", DEVICE_ID)
        print("Confidence:", confidence)
        print("Image:", filepath)
        print("Backend:", BACKEND_URL)


        response = session.post(

            BACKEND_URL,

            files=files,

            data=data,

            timeout=5

        )


        print(
            "BACKEND STATUS:",
            response.status_code
        )


        print(
            "BACKEND RESPONSE:",
            response.text
        )


        print("==========================\n")


    except Exception as e:

        print(
            "BACKEND ERROR:",
            e
        )



# =========================
# ESP32 UPLOAD
# =========================

@app.route(
    "/upload",
    methods=["POST"]
)

def upload():

    global last_fire_time
    global previous_alert_state
    global last_sent_time


    image = request.data


    if len(image) == 0:

        return "No image",400



    npimg = np.frombuffer(
        image,
        np.uint8
    )


    frame = cv2.imdecode(
        npimg,
        cv2.IMREAD_COLOR
    )


    if frame is None:

        return "Decode error",400



    # =====================
    # YOLO DETECTION
    # =====================

    results = model(
        frame,
        conf=CONFIDENCE_THRESHOLD,
        imgsz=320,
        verbose=False
    )[0]


    fire_detected = False

    max_conf = 0


    annotated = frame.copy()



    for box in results.boxes:


        cls = int(
            box.cls[0]
        )


        conf = float(
            box.conf[0]
        )


        class_name = model.names[cls]


        print(
            f"Detection | class={cls} name={class_name} confidence={conf:.2f}"
        )


        max_conf = max(
            max_conf,
            conf
        )


        # FIRE DETECT
        if (
            "fire" in class_name.lower()
            or cls == 1
        ) and conf >= CONFIDENCE_THRESHOLD:


            fire_detected = True


            x1,y1,x2,y2 = map(
                int,
                box.xyxy[0]
            )


            cv2.rectangle(
                annotated,
                (x1,y1),
                (x2,y2),
                (0,0,255),
                2
            )


            cv2.putText(
                annotated,
                f"fire {conf:.2f}",
                (x1,y1-10),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.7,
                (0,0,255),
                2
            )



    # =====================
    # FIRE ALERT
    # =====================

    if fire_detected:


        last_fire_time = time.time()


        cv2.putText(
            annotated,
            "FIRE ALERT",
            (50,80),
            cv2.FONT_HERSHEY_SIMPLEX,
            1.5,
            (0,0,255),
            3
        )


        current_time = time.time()



        # SEND NGAY
        if current_time - last_sent_time >= SEND_COOLDOWN:


            mqtt_client.publish(
                mqtt_topic,
                "1"
            )


            print(
                "Sending fire alert..."
            )


            send_fire_alert(
                annotated,
                max_conf
            )


            last_sent_time = current_time


            print(
                "FIRE ALERT SENT"
            )


        previous_alert_state = True



    else:


        if previous_alert_state:


            if (
                last_fire_time
                and time.time() - last_fire_time > 10
            ):


                mqtt_client.publish(
                    mqtt_topic,
                    "0"
                )


                previous_alert_state = False


                print(
                    "SYSTEM SAFE"
                )



    cv2.imshow(
        "Fire Detection",
        annotated
    )


    cv2.waitKey(1)


    return "OK",200




# =========================
# MAIN
# =========================

if __name__ == "__main__":


    print(
        "Python Fire Server"
    )


    app.run(
        host="0.0.0.0",
        port=5000
    )