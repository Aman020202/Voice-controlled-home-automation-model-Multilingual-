from flask import Flask, render_template, request, jsonify
from voice_processor import VoiceProcessor
from esp_controller import ESPController
import threading
import time

app = Flask(__name__)

# Initialize components
voice_processor = VoiceProcessor()
esp_controller = ESPController("10.201.5.253")  # Change to your ESP IP

# Current status
status = {'light1': False, 'light2': False, 'light3': False, 'light4': False}

def update_status():
    while True:
        try:
            esp_status = esp_controller.get_status()
            for i in range(1, 5):
                status[f'light{i}'] = esp_status.get(f'light{i}', False)
        except Exception as e:
            print(f"Status update error: {e}")
        time.sleep(2)

# Start status update thread
threading.Thread(target=update_status, daemon=True).start()

@app.route('/')
def home():
    supported_langs = voice_processor.get_supported_languages()
    return render_template('index.html', languages=supported_langs)

@app.route('/api/voice', methods=['POST'])
def voice_command():
    try:
        data = request.json
        command = data.get('command', '').strip()
        
        if not command:
            return jsonify({'success': False, 'message': 'Empty command'})
        
        print(f"🎤 Received voice command: {command}")
        
        result = voice_processor.process_command(command)
        
        if result['success']:
            print(f"✅ Command processed: {result['message']}")
            
            # Send to ESP8266
            control_result = esp_controller.control_multiple(result['commands'])
            
            # Update status
            for cmd in result['commands']:
                status[f'light{cmd["light"]}'] = cmd['state']
            
            response_data = {
                'success': True,
                'message': result['message'],
                'language': result['detected_language'],
                'translated': result.get('translated_text', '')
            }
            
            print(f"📤 Sending response: {response_data}")
            return jsonify(response_data)
        else:
            print(f"❌ Command failed: {result['message']}")
            return jsonify({
                'success': False,
                'message': result['message']
            })
            
    except Exception as e:
        print(f"🔥 Server error: {e}")
        return jsonify({'success': False, 'message': f'Server error: {str(e)}'})

@app.route('/api/control', methods=['POST'])
def control_light():
    try:
        data = request.json
        light = data.get('light')
        action = data.get('action')
        
        if light is None or action is None:
            return jsonify({'success': False, 'message': 'Missing parameters'})
        
        state = action == 'on'
        result = esp_controller.control_light(light, state)
        
        if result.get('success'):
            status[f'light{light}'] = state
            return jsonify({'success': True, 'message': f'Light {light} turned {action}'})
        else:
            return jsonify({'success': False, 'message': 'Control failed'})
            
    except Exception as e:
        return jsonify({'success': False, 'message': str(e)})

@app.route('/api/status')
def get_status():
    return jsonify(status)

@app.route('/api/all-on', methods=['POST'])
def all_on():
    try:
        commands = [{"light": i, "state": True} for i in range(1, 5)]
        result = esp_controller.control_multiple(commands)
        for i in range(1, 5):
            status[f'light{i}'] = True
        return jsonify({'success': True, 'message': 'All lights ON'})
    except Exception as e:
        return jsonify({'success': False, 'message': str(e)})

@app.route('/api/all-off', methods=['POST'])
def all_off():
    try:
        commands = [{"light": i, "state": False} for i in range(1, 5)]
        result = esp_controller.control_multiple(commands)
        for i in range(1, 5):
            status[f'light{i}'] = False
        return jsonify({'success': True, 'message': 'All lights OFF'})
    except Exception as e:
        return jsonify({'success': False, 'message': str(e)})

@app.route('/api/languages')
def get_languages():
    """Get supported languages"""
    languages = voice_processor.get_supported_languages()
    return jsonify(languages)

if __name__ == '__main__':
    print("🚀 Starting Enhanced Multilingual Voice Control System...")
    print("🔗 Testing ESP8266 connection...")
    
    if esp_controller.test_connection():
        print("✅ ESP8266 Connected!")
    else:
        print("❌ ESP8266 Not Found! Check IP address")
    
    print("🌐 Supported Languages:")
    languages = voice_processor.get_supported_languages()
    for code, name in languages.items():
        print(f"   {code}: {name}")
    
    print("💻 Web Interface: http://localhost:5001")
    app.run(host='0.0.0.0', port=5001, debug=True)