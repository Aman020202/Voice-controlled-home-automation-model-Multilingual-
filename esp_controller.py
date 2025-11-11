import requests
import time

class ESPController:
    def __init__(self, esp_ip):
        self.esp_ip = esp_ip
        self.base_url = f"http://{esp_ip}"
        self.timeout = 5
        
    def test_connection(self):
        try:
            response = requests.get(f"{self.base_url}/status", timeout=3)
            return response.status_code == 200
        except:
            return False
    
    def control_light(self, light_num, state):
        try:
            action = "on" if state else "off"
            url = f"{self.base_url}/control?relay={light_num}&action={action}"
            response = requests.get(url, timeout=self.timeout)
            return response.json()
        except Exception as e:
            return {"success": False, "error": str(e)}
    
    def control_multiple(self, commands):
        try:
            payload = []
            for cmd in commands:
                payload.append({
                    "relay": cmd["light"],
                    "action": "on" if cmd["state"] else "off"
                })
            
            response = requests.post(f"{self.base_url}/bulk", json=payload, timeout=self.timeout)
            return response.json()
        except Exception as e:
            return {"success": False, "error": str(e)}
    
    def get_status(self):
        try:
            response = requests.get(f"{self.base_url}/status", timeout=self.timeout)
            return response.json()
        except:
            return {}