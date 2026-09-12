import time
import requests
import numpy as np
from sklearn.linear_model import SGDClassifier

BLYNK_TOKEN = "fk1gMjx0aaEO_OPHAAmAzrhXPidIc_ng"
BASE_URL = "https://fra1.blynk.cloud/external/api"

classes = np.array([0, 1])
model = SGDClassifier(loss='log_loss', random_state=42)

baseline_X = np.array([
    [24.0, 7.2, 5.0, 150.0],
    [25.0, 7.6, 12.0, 220.0],
    [34.0, 10.5, 80.0, 950.0],
    [31.0, 4.8, 65.0, 800.0]
])
baseline_y = np.array([0, 0, 1, 1])
model.partial_fit(baseline_X, baseline_y, classes=classes)
print("✔ Model initialized in cloud.")

def read_sensor(pin):
    try:
        url = f"{BASE_URL}/get?token={BLYNK_TOKEN}&{pin}"
        res = requests.get(url, timeout=5)
        if res.status_code == 200:
            return float(res.text)
    except:
        return None
    return None

def send_to_dashboard(pin, val):
    try:
        url = f"{BASE_URL}/update?token={BLYNK_TOKEN}&{pin}={val}"
        requests.get(url, timeout=5)
    except Exception as e:
        print(f"Error sending {pin}: {e}")

print("🚀 Cloud AI Engine running 24/7...")

while True:
    try:
        ph = read_sensor("V1")
        tds = read_sensor("V2")
        turb = read_sensor("V3")
        temp = read_sensor("V4")

        if None not in (ph, tds, turb, temp):
            sample = np.array([[temp, ph, turb, tds]])
            probabilities = model.predict_proba(sample)[0]
            risk_percent = int(probabilities[1] * 100)

            if risk_percent >= 70:
                decision = "Critical: Stagnant Water / High Bacterial Risk"
            elif risk_percent >= 35:
                decision = "Moderate: Water parameters abnormal"
            else:
                decision = "Normal: Water conditions are optimal and safe"

            label = 1 if (ph < 6.5 or ph > 8.5 or turb > 40 or tds > 500 or temp > 32) else 0
            model.partial_fit(sample, [label])

            send_to_dashboard("V5", risk_percent)
            send_to_dashboard("V6", decision)

        time.sleep(5)
    except Exception as err:
        print(f"Loop error: {err}")
        time.sleep(5)
