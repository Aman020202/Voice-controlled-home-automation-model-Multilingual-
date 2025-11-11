import requests
import json

class VoiceProcessor:
    def __init__(self):
        # Expanded language support
        self.supported_languages = {
            'en': 'English',
            'hi': 'Hindi', 
            'es': 'Spanish',
            'fr': 'French',
            'de': 'German',
            'it': 'Italian',
            'pt': 'Portuguese',
            'ru': 'Russian',
            'ja': 'Japanese',
            'ko': 'Korean',
            'zh': 'Chinese',
            'ar': 'Arabic'
        }
        
        # Command patterns in multiple languages
        self.command_patterns = {
            'on': {
                'en': ['on', 'turn on', 'switch on', 'activate', 'enable', 'start', 'open'],
                'hi': ['चालू', 'ऑन', 'शुरू', 'खोलो', 'स्टार्ट'],
                'es': ['encender', 'prender', 'activar', 'abrir'],
                'fr': ['allumer', 'activer', 'ouvrir'],
                'de': ['einschalten', 'anmachen', 'aktivieren'],
                'it': ['accendere', 'attivare', 'aprire'],
                'pt': ['ligar', 'ativar', 'abrir'],
                'ru': ['включить', 'активировать', 'открыть'],
                'ja': ['オン', 'つける', '開始'],
                'ko': ['켜다', '시작', '활성화'],
                'zh': ['打开', '开启', '启动'],
                'ar': ['تشغيل', 'فتح', 'بدء']
            },
            'off': {
                'en': ['off', 'turn off', 'switch off', 'deactivate', 'disable', 'stop', 'close'],
                'hi': ['बंद', 'ऑफ', 'रोको', 'बंद करो', 'बंद करें'],
                'es': ['apagar', 'desactivar', 'cerrar'],
                'fr': ['éteindre', 'désactiver', 'fermer'],
                'de': ['ausschalten', 'ausmachen', 'deaktivieren'],
                'it': ['spegnere', 'disattivare', 'chiudere'],
                'pt': ['desligar', 'desativar', 'fechar'],
                'ru': ['выключить', 'деактивировать', 'закрыть'],
                'ja': ['オフ', '消す', '停止'],
                'ko': ['끄다', '중지', '비활성화'],
                'zh': ['关闭', '关掉', '停止'],
                'ar': ['إيقاف', 'إغلاق', 'تعطيل']
            },
            'lights': {
                'en': ['light', 'lights', 'bulb', 'bulbs', 'lamp', 'lamps'],
                'hi': ['लाइट', 'लाइट्स', 'बल्ब', 'बल्ब्स', 'दीया', 'लैंप'],
                'es': ['luz', 'luces', 'foco', 'focos', 'lámpara'],
                'fr': ['lumière', 'lumières', 'ampoule', 'ampoules'],
                'de': ['licht', 'lichter', 'birne', 'birnen', 'lampe'],
                'it': ['luce', 'luci', 'lampadina', 'lampadine'],
                'pt': ['luz', 'luzes', 'lâmpada', 'lâmpadas'],
                'ru': ['свет', 'лампочка', 'лампа'],
                'ja': ['光', 'ライト', '電球', 'ランプ'],
                'ko': ['빛', '조명', '전구', '램프'],
                'zh': ['灯', '灯光', '灯泡', '灯'],
                'ar': ['ضوء', 'أضواء', 'لمبة', 'مصباح']
            },
            'numbers': {
                'en': {'one': 1, 'two': 2, 'three': 3, 'four': 4, '1': 1, '2': 2, '3': 3, '4': 4},
                'hi': {'एक': 1, 'दो': 2, 'तीन': 3, 'चार': 4, '1': 1, '2': 2, '3': 3, '4': 4},
                'es': {'uno': 1, 'dos': 2, 'tres': 3, 'cuatro': 4},
                'fr': {'un': 1, 'deux': 2, 'trois': 3, 'quatre': 4},
                'de': {'eins': 1, 'zwei': 2, 'drei': 3, 'vier': 4},
                'it': {'uno': 1, 'due': 2, 'tre': 3, 'quattro': 4},
                'pt': {'um': 1, 'dois': 2, 'três': 3, 'quatro': 4},
                'ru': {'один': 1, 'два': 2, 'три': 3, 'четыре': 4},
                'ja': {'一': 1, '二': 2, '三': 3, '四': 4},
                'ko': {'일': 1, '이': 2, '삼': 3, '사': 4},
                'zh': {'一': 1, '二': 2, '三': 3, '四': 4},
                'ar': {'واحد': 1, 'اثنان': 2, 'ثلاثة': 3, 'أربعة': 4}
            },
            'all': {
                'en': ['all', 'every', 'everything', 'each'],
                'hi': ['सब', 'सभी', 'हर', 'प्रत्येक'],
                'es': ['todo', 'todos', 'cada'],
                'fr': ['tout', 'tous', 'chaque'],
                'de': ['alle', 'jede', 'alles'],
                'it': ['tutto', 'tutti', 'ogni'],
                'pt': ['tudo', 'todos', 'cada'],
                'ru': ['все', 'каждый'],
                'ja': ['すべて', '全体', '各'],
                'ko': ['모든', '전체', '각'],
                'zh': ['所有', '全部', '每个'],
                'ar': ['كل', 'جميع', 'كامل']
            }
        }

    def translate_text(self, text, target_lang='en'):
        """Translate text to English using Google Translate API"""
        try:
            # Using Google Translate API
            url = "https://translate.googleapis.com/translate_a/single"
            params = {
                'client': 'gtx',
                'sl': 'auto',  # auto-detect source language
                'tl': target_lang,
                'dt': 't',
                'q': text
            }
            
            response = requests.get(url, params=params, timeout=10)
            if response.status_code == 200:
                data = response.json()
                translated_text = data[0][0][0]
                return translated_text.lower()
            return text.lower()
        except Exception as e:
            print(f"Translation error: {e}")
            return text.lower()

    def detect_language(self, text):
        """Detect the language of the input text"""
        try:
            # Simple keyword-based detection
            text_lower = text.lower()
            for lang_code, lang_name in self.supported_languages.items():
                # Check for language-specific keywords
                for keyword_type in ['on', 'off', 'numbers']:
                    if lang_code in self.command_patterns[keyword_type]:
                        for keyword in self.command_patterns[keyword_type][lang_code]:
                            if keyword in text_lower:
                                return lang_code
            return 'en'  # Default to English
        except Exception as e:
            print(f"Language detection error: {e}")
            return 'en'

    def extract_light_numbers(self, text, language):
        """Extract which lights are being referenced"""
        numbers = []
        text_lower = text.lower()
        
        # Check for "all"
        if language in self.command_patterns['all']:
            for word in self.command_patterns['all'][language]:
                if word in text_lower:
                    return [1, 2, 3, 4]
        
        # Check language-specific numbers
        if language in self.command_patterns['numbers']:
            for word, number in self.command_patterns['numbers'][language].items():
                if word in text_lower:
                    numbers.append(number)
        
        # Also check for numeric patterns (universal)
        for i in range(1, 5):
            if str(i) in text_lower:
                numbers.append(i)
        
        return list(set(numbers))  # Remove duplicates

    def determine_action(self, text, language):
        """Determine if the action is ON or OFF"""
        text_lower = text.lower()
        
        on_count = 0
        off_count = 0
        
        if language in self.command_patterns['on']:
            for word in self.command_patterns['on'][language]:
                if word in text_lower:
                    on_count += 1
        
        if language in self.command_patterns['off']:
            for word in self.command_patterns['off'][language]:
                if word in text_lower:
                    off_count += 1
        
        if on_count > off_count:
            return 'on'
        elif off_count > on_count:
            return 'off'
        else:
            # If ambiguous, check English translation
            translated = self.translate_text(text)
            if any(word in translated for word in self.command_patterns['on']['en']):
                return 'on'
            elif any(word in translated for word in self.command_patterns['off']['en']):
                return 'off'
            return None

    def process_command(self, command_text):
        """Main NLP processing function with enhanced multilingual support"""
        try:
            if not command_text.strip():
                return {'success': False, 'message': 'No command received'}
            
            print(f"🔍 Processing command: {command_text}")
            
            # Detect language
            detected_lang = self.detect_language(command_text)
            print(f"🌐 Detected language: {detected_lang}")
            
            # Translate to English for better processing
            english_translation = self.translate_text(command_text, 'en')
            print(f"🔄 English translation: {english_translation}")
            
            # Extract light numbers from original text
            light_numbers = self.extract_light_numbers(command_text, detected_lang)
            
            # If no numbers found in original, try English translation
            if not light_numbers:
                light_numbers = self.extract_light_numbers(english_translation, 'en')
            
            if not light_numbers:
                return {
                    'success': False, 
                    'message': 'No specific lights mentioned. Try: "light 1 on" or "all lights off"'
                }
            
            # Determine action from original text
            action = self.determine_action(command_text, detected_lang)
            
            # If no action found, try English translation
            if not action:
                action = self.determine_action(english_translation, 'en')
            
            if not action:
                return {
                    'success': False,
                    'message': 'No action specified. Say "on" or "off"'
                }
            
            # Prepare actions
            actions = []
            for light_num in light_numbers:
                actions.append({
                    'light': light_num,
                    'state': action == 'on'
                })
            
            # Generate response message
            if len(light_numbers) == 4:
                light_desc = "all lights"
            else:
                light_desc = "light" + ("s " if len(light_numbers) > 1 else " ") + ", ".join(map(str, light_numbers))
            
            message = f"Turning {action} {light_desc}"
            
            return {
                'success': True,
                'message': message,
                'commands': actions,
                'detected_language': detected_lang,
                'translated_text': english_translation
            }
            
        except Exception as e:
            print(f"❌ Error processing command: {e}")
            return {
                'success': False,
                'message': f'Error processing command: {str(e)}'
            }

    def get_supported_languages(self):
        """Get list of supported languages"""
        return self.supported_languages