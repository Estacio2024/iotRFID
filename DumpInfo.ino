#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 10   // Pino SDA do módulo RFID
#define RST_PIN 9   // Pino RST do módulo RFID

MFRC522 rfid(SS_PIN, RST_PIN);        // Instância para o leitor RFID
LiquidCrystal_I2C lcd(0x27, 16, 2);   // Endereço do LCD I2C (ajuste se necessário)

byte uidFelype[] = {0xD3, 0xFF, 0x2E, 0xDB};    // UID do cartão "Felype"
byte uidAdmin[] = {0x43, 0x6A, 0x56, 0x14};     // UID do cartão "Admin"

const int maxCartoes = 10;  // Máximo de cartões cadastrados
byte uidsCadastrados[maxCartoes][4];  // Array para guardar UIDs cadastrados
int totalCadastrados = 0;             // Contador de cartões cadastrados

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  lcd.init();
  lcd.backlight();
  lcd.print("Acesse com cartao");
  Serial.println("Sistema de acesso inicializado.");
}

void loop() {
  // Verifica se há um cartão próximo ao leitor
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    Serial.print("UID do cartão detectado: ");
    printUID();

    // Verifica se o UID corresponde ao cartão "Felype"
    if (compareUID(rfid.uid.uidByte, uidFelype, rfid.uid.size)) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("BEM-VINDO FELYPE");
      Serial.println("BEM-VINDO FELYPE"); // Envia a mensagem via Serial para a aplicação Python
    }
    // Verifica se o UID corresponde ao cartão "Admin"
    else if (compareUID(rfid.uid.uidByte, uidAdmin, rfid.uid.size)) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Modo Admin");
      Serial.println("Modo Admin: Habilitar cadastro");
      delay(2000);
      addNewCard();  // Chama a função para cadastrar um novo cartão
    }
    // Verifica se o cartão lido é um cartão previamente cadastrado
    else if (verificaCartaoCadastrado(rfid.uid.uidByte)) {
      lcd.clear();
      lcd.setCursor(0, 0);
      Serial.println("Cartão cadastrado");
      lcd.print("ALUNO ");
      lcd.print(totalCadastrados);
      lcd.setCursor(0, 1);
      lcd.print("DA ESTACIO");
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Cartao desconhecido");
      Serial.println("Cartao desconhecido");
    }

    rfid.PICC_HaltA();  // Termina a comunicação com o cartão
    delay(1000);         // Atraso para evitar múltiplas leituras
  }
}

// Função para imprimir o UID do cartão no monitor serial
void printUID() {
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();
}

// Função para comparar UIDs
bool compareUID(byte *uid1, byte *uid2, byte tamanho) {
  for (byte i = 0; i < tamanho; i++) {
    if (uid1[i] != uid2[i]) return false;
  }
  return true;
}

// Função para adicionar um novo cartão
void addNewCard() {
  if (totalCadastrados < maxCartoes) {
    Serial.println("Aproxime o novo cartao para cadastro...");

    while (!rfid.PICC_IsNewCardPresent());  // Espera até que um novo cartão seja apresentado

    if (rfid.PICC_ReadCardSerial()) {
      // Salva o UID do novo cartão no array uidsCadastrados
      for (byte i = 0; i < rfid.uid.size; i++) {
        uidsCadastrados[totalCadastrados][i] = rfid.uid.uidByte[i];
      }
      totalCadastrados++;

      Serial.print("Novo UID cadastrado como Aluno ");
      Serial.print(totalCadastrados);
      Serial.println(" da Estácio");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Cartao cadastrado");
      lcd.setCursor(0, 1);
      lcd.print("ALUNO ");
      lcd.print(totalCadastrados);
      lcd.print(" DA ESTACIO");
      delay(2000);
    }
  } else {
    Serial.println("Limite de cadastro atingido.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Limite de");
    lcd.setCursor(0, 1);
    lcd.print("cadastro");
    delay(2000);
  }
}

// Função para verificar se o cartão é cadast rado
bool verificaCartaoCadastrado(byte *uid) {
  for (int i = 0; i < totalCadastrados; i++) {
    if (compareUID(uid, uidsCadastrados[i], rfid.uid.size)) {
      return true;
    }
  }
  return false;
}
