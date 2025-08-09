# **Vitrine Inteligente**

Projeto acadêmico - **controle automático de iluminação de vitrine + monitoramento de temperatura**

**Arquivo principal:** `vitrine_inteligente.ino`

**Dependências:**
- Arduino core for ESP32  
- DHT sensor library (Adafruit)  
- U8g2 *(ou Adafruit SSD1306 + Adafruit GFX se preferir)*  
- EEPROM  

## **Instalação**
1. Instale as bibliotecas no Arduino IDE.  
2. Conecte o ESP32 via USB.  
3. Ajuste os pinos no topo do arquivo se necessário.  
4. Faça upload para o ESP32.  
5. Teste PIR e relé em bancada antes de instalar na vitrine.  

## **Segurança**
- Se for comutar tensão de rede *(mains)*, consulte um eletricista e use relé e caixa adequados.  
- Mantenha GND comum entre módulos.  

---

# **Esquema Elétrico**

**Componentes utilizados:**
- 1× Arduino Uno R3  
- 1× Display LCD 16×2 com módulo I2C  
- 1× Módulo de botões (ou botões individuais)  
- 1× Buzzer passivo (5V)  
- Jumpers macho-macho  
- Protoboard *(opcional)*  
- Fonte USB *(ou tomada + adaptador 5V)*  

**Ligações:**

**LCD 16×2 I2C**
- VCC → 5V do Arduino  
- GND → GND do Arduino  
- SDA → A4 do Arduino  
- SCL → A5 do Arduino  

**Botões** *(exemplo com 3 botões: "Hambúrguer", "Batata", "Bebida")*  
- Um lado de cada botão → pino digital (2, 3, 4 do Arduino)  
- Outro lado de cada botão → GND  
- *Pull-up interno será ativado no código (não precisa resistor externo)*  

**Buzzer**
- Positivo → pino digital 8 do Arduino  
- Negativo → GND  

**Diagrama Simplificado:**


+5V ------------------- LCD VCC
GND ------------------- LCD GND
A4 -------------------- LCD SDA
A5 -------------------- LCD SCL

Pin 2 ---[Botão 1]--- GND
Pin 3 ---[Botão 2]--- GND
Pin 4 ---[Botão 3]--- GND

Pin 8 ---[Buzzer]---- GND


# **Explicação do Código**

O código foi escrito para **controlar um sistema simples de pedidos na lanchonete**.  

**O que ele faz:**

### **1. Inicialização**
- Configura o display LCD 16×2 para exibir mensagens de status.  
- Ativa o *pull-up* interno dos pinos de botões para evitar resistores externos.  
- Prepara o buzzer para emitir sinais sonoros.  

### **2. Loop Principal**
- Lê constantemente o estado de cada botão.  
- Se algum botão for pressionado, registra o pedido no display e aciona um **beep** no buzzer.  
- Exibe no LCD o tipo do pedido feito.  
- Aguarda um pequeno `delay` para evitar leituras duplicadas devido ao *bounce* do botão.  

### **3. Funções Extras**
- `beep()` emite um som curto no buzzer para confirmar o pedido.  
- O sistema é modular: é fácil adicionar mais botões para novos produtos.  
