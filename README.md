Vitrine Inteligente
===================

Projeto acadêmico - controle automático de iluminação de vitrine + monitoramento de temperatura

Arquivo principal: vitrine_inteligente.ino

Dependências:
 - Arduino core for ESP32
 - DHT sensor library (Adafruit)
 - U8g2 (ou Adafruit SSD1306 + Adafruit GFX se preferir)
 - EEPROM

Instalação:
 1. Instale as bibliotecas no Arduino IDE.
 2. Conecte o ESP32 via USB.
 3. Ajuste os pinos no topo do arquivo se necessário.
 4. Faça upload para o ESP32.
 5. Teste PIR e relé em bancada antes de instalar na vitrine.

Segurança:
 - Se for comutar tensão de rede (mains), consulte um eletricista e use relé e caixa adequados.
 - Mantenha GND comum entre módulos.
