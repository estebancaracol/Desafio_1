// C++ code
//
#include <Adafruit_LiquidCrystal.h>

Adafruit_LiquidCrystal lcd(0);

const int botonInicio = 2;
const int botonPausa = 4;
const int pinEntrada = A0;
float amplitud = 0;
float frecuencia = 0;
char tipoOnda[12];
bool adquiriendo = false;
bool enPausa = false;
int* valoresAnalogicos = nullptr;
int numMuestras = 100;
bool esSenoidal = false, esCuadrada = false, esTriangular = false;
int indice = 0;
int val = 0;
unsigned long tiempoAnterior = 0;
int ultimoValor = 0;

void setup(){
  
  pinMode(botonInicio, INPUT_PULLUP);
  pinMode(botonPausa, INPUT_PULLUP);
  pinMode(pinEntrada, INPUT);
  
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("Esperando...");
  valoresAnalogicos = new int[numMuestras];
}

void loop(){
  
  manejarBotones();
  
  if (adquiriendo && !enPausa){
    adquiriendoMuestras();
  }
}

void manejarBotones(){
  
  if (digitalRead(botonInicio) == LOW && !adquiriendo){
    adquiriendo = true;
    lcd.clear();
    lcd.print("Adquiriendo...");
    delay(1000);
  }
    
  if (digitalRead(botonPausa) == LOW && adquiriendo){
    enPausa = !enPausa;
    lcd.clear();
    lcd.print(enPausa ? "En Pausa..." : "Reanudando...");
    delay(1000);
  }
}
 
void adquiriendoMuestras(){
  
  val = analogRead(pinEntrada);
  valoresAnalogicos[indice] = val;
  indice++;
  Serial.println(val);

  if (indice >= numMuestras){
    procesarSenal();
    mostrarResultados();
    indice = 0;
  }

  delay(100);
}

void procesarSenal(){
  
  amplitud = 0;
  frecuencia = 0;
  strcpy(tipoOnda, "Desconocido");
  
  for (int i = 0; i < numMuestras; i++){
    float voltaje = valoresAnalogicos[i] * (5.0 / 1023.0);
    
    if (voltaje > amplitud){
      amplitud = voltaje;
    }

    if (ultimoValor < 512 && valoresAnalogicos[i] >= 512){
      unsigned long tiempoActual = millis();
      unsigned long intervalo = tiempoActual - tiempoAnterior;
      if (intervalo > 0){
        frecuencia = 1000.0 / intervalo;
      }
      tiempoAnterior = tiempoActual;
    }

    ultimoValor = valoresAnalogicos[i];
  }
  
  determinarTipoOnda();
}

void determinarTipoOnda(){
  
  int maxDelta = 0;
  int cuentaConstante = 0;
  int transiciones = 0;
  bool tieneCambiosLineales = true;
  
  esSenoidal = false;
  esCuadrada = false;
  esTriangular = false;
  
  for (int i = 1; i < numMuestras; i++){
    int delta = abs(valoresAnalogicos[i] - valoresAnalogicos[i - 1]);
    
    if (delta > maxDelta) maxDelta = delta;
    if (delta == 0) cuentaConstante++;
    if (i >= 2 && delta != abs(valoresAnalogicos[i - 1] - valoresAnalogicos[i - 2])){
      tieneCambiosLineales = false;
    }
    
    if ((valoresAnalogicos[i] >= 512 && valoresAnalogicos[i - 1] < 512) || (valoresAnalogicos[i] < 512 && valoresAnalogicos[i - 1] >= 512)){
      transiciones++;
    }
  }
  
  if (cuentaConstante > numMuestras / 2) {
    esCuadrada = true;
    strcpy(tipoOnda, "Cuadrada");
  } else if (tieneCambiosLineales && transiciones >= 2) {
    esTriangular = true;
    strcpy(tipoOnda, "Triangular");
  } else if (transiciones >= 2 && maxDelta > 50 && !tieneCambiosLineales){
    esSenoidal = true;
    strcpy(tipoOnda, "Senoidal");
  } else {
    strcpy(tipoOnda, "Desconocido");
  }
  
  if (esSenoidal) {
    Serial.println("Onda Senoidal");
  } else if (esCuadrada) {
    Serial.println("Onda Cuadrada");
  } else if (esTriangular) {
    Serial.println("Onda Triangular");
  } else {
    Serial.println("Onda Desconocida");
  }

  delay(1000);
}

void mostrarResultados(){
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Freq: ");
  lcd.print(frecuencia);
  lcd.print(" Hz");
  
  lcd.setCursor(0, 1);
  lcd.print("Amp: ");
  lcd.print(amplitud);
  lcd.print("V");
  
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tipo de onda:");
  lcd.setCursor(0, 1);
  lcd.print(tipoOnda);
  
  delay(2000);
}

void cleanup(){
  delete[] valoresAnalogicos;
}
