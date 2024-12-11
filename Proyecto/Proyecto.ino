/*Inicio
  ↓
Seleccionar Producto
  ↓
Ajustar Cantidad
  ↓
Confirmar Pedido
  ↓
Ingresar Dinero
  ↓
Entregar Producto
  ↓
Resetear Sistema
  ↓
Inicio*/

#include <string.h>
#include <stdio.h>
#include <ctype.h>
bool producto = 1;  // 0 para Agua o 1 para gaseosa
int costo_agua = 500;
int costo_gaseosa = 700;
volatile int contadorINT0 = 0;  // Contador para INT0 $ 100
volatile int contadorINT1 = 0;  // Contador para INT1 $ 200
volatile int contadorINT2 = 0;  // Contador para INT2 $ 500
volatile int total_pagar = 0;
volatile int total_agua = 0;
volatile int total_gaseosa = 0;
volatile int cantidad_agua = 0;
volatile int cantidad_gaseosa = 0;
volatile static bool ingresando_cantidad_agua = false;
volatile static bool ingresando_cantidad_gaseosa = false;
volatile static bool mostro_total_agua = false;
volatile static bool mostro_total_gaseosa = false;
volatile static bool segundo_producto = false;  // Flag para saber si estamos ingresando el segundo producto
const byte ROWS = 4;
const byte COLS = 4;
bool needsUpdate = false;  // Flag para actualizar el LCD

char keys[ROWS][COLS] = {
  { '7', '8', '9', '/' },
  { '4', '5', '6', '*' },
  { '1', '2', '3', '-' },
  { 'C', '0', '=', '+' }
};

byte colPins[COLS] = { 10, 11, 12, 13 };
byte rowPins[ROWS] = { 50, 51, 52, 53 };
String entrada = "";

#define RS 35  // Pines del LCD
#define RW 36
#define E 37
#define D0 22
#define D1 23
#define D2 24
#define D3 25
#define D4 26
#define D5 27
#define D6 28
#define D7 29

void setup() {
  Serial.begin(9600);
  //Configurar pines de interrupciones como entrada con resistencia pull - up
  pinMode(19, INPUT_PULLUP);  // INT2
  pinMode(20, INPUT_PULLUP);  // INT1
  pinMode(21, INPUT_PULLUP);  // INT0
  pinMode(6, OUTPUT);         // INT0
  pinMode(7, OUTPUT);         // INT0

  // Configurar las interrupciones externas
  attachInterrupt(digitalPinToInterrupt(19), manejarINT2, FALLING);  // INT2
  attachInterrupt(digitalPinToInterrupt(20), manejarINT1, FALLING);  // INT1
  attachInterrupt(digitalPinToInterrupt(21), manejarINT0, FALLING);  // INT0

  for (byte i = 0; i < ROWS; i++) pinMode(rowPins[i], INPUT_PULLUP);  // Configuración de pines del teclado
  for (byte i = 0; i < COLS; i++) {
    pinMode(colPins[i], OUTPUT);
    digitalWrite(colPins[i], HIGH);
  }

  pinMode(RS, OUTPUT);  // Configuración de pines del LCD
  pinMode(E, OUTPUT);
  pinMode(RW, OUTPUT);
  for (int i = D0; i <= D7; i++) pinMode(i, OUTPUT);

  inicializarLCD();  // Inicializar LCD
  writeString("Bebida a escoger");
  moverCursor(1, 0);  // Línea 1 (segunda línea), columna 0
  writeString("1.Agua 2.Gaseosa");
}

void moverCursor(int linea, int columna) {
  byte posicion = (linea == 0) ? 0x80 : 0xC0;  // 0x80 para línea 0, 0xC0 para línea 1
  posicion += columna;                         // Agregar el desplazamiento de columna
  enviar(posicion, false);
  delay(50);
}

// Funciones del LCD
void inicializarLCD() {
  delay(50);
  enviar(0x38, false);  // Modo de 8 bits, 2 líneas
  enviar(0x0C, false);  // Display encendido, cursor apagado
  enviar(0x06, false);  // Incrementar cursor
  enviar(0x01, false);  // Limpiar pantalla
  delay(5);
}

void writeString(const char *str) {
  while (*str) enviar(*str++, true);
}

void enviar(byte data, bool rs) {
  digitalWrite(RS, rs ? HIGH : LOW);
  for (int i = 0; i < 8; i++) digitalWrite(D0 + i, (data >> i) & 1);
  pulsoEnable();
}

void pulsoEnable() {
  digitalWrite(E, HIGH);
  delayMicroseconds(1);
  digitalWrite(E, LOW);
  delayMicroseconds(100);
}

void limpiarLCD() {
  enviar(0x01, false);
  delay(50);
}

char leerTeclado() {
  for (byte c = 0; c < COLS; c++) {
    digitalWrite(colPins[c], LOW);  // Activar la columna actual
    delay(10);
    for (byte r = 0; r < ROWS; r++) {
      if (digitalRead(rowPins[r]) == LOW) {  // Si se detecta una tecla presionada
        delay(200);                          // Anti-rebote
        digitalWrite(colPins[c], HIGH);      // Desactivar la columna
        return keys[r][c];                   // Retornar la tecla correspondiente
      }
    }
    digitalWrite(colPins[c], HIGH);  // Desactivar la columna
  }
  return '\0';  // Ninguna tecla presionada
}


void loop() {
  char tecla = leerTeclado();  // Leer la tecla presionada
  if (tecla != '\0') {         // Si se presionó una tecla válida
    logica(tecla);
    needsUpdate = true;  // Indicar que se necesita actualizar el LCD
  }
  delay(100);
}

char *int_to_char(int num) {
  int i = log10(num) + 1;
  char *s = (char *)calloc(i + 1, sizeof(char));

  for (i--; num != 0; i--) {
    s[i] = (num % 10) + '0';
    num /= 10;
  }

  return s;
}

void logica(char tecla) {
  Serial.println("ingresando_cantidad_agua ");
  Serial.println(ingresando_cantidad_agua);  
  Serial.println("ingresando_cantidad_gaseosa");
  Serial.println(ingresando_cantidad_gaseosa);  
  Serial.println("mostro_total_agua");
  Serial.println(mostro_total_agua);
  Serial.println("mostro_total_gaseosa ");
  Serial.println(mostro_total_gaseosa);  
  Serial.println("segundo_producto");  // Flag para saber si estamos ingresando el segundo producto
  Serial.println(segundo_producto);  
  Serial.println("****");  


  char text[20];  //variables auxiliares para conversion int a string
  char text2[20];

  if (ingresando_cantidad_agua && isdigit(tecla) && !ingresando_cantidad_gaseosa) {
    limpiarLCD();
    writeString("Agua: $500");
    moverCursor(1, 0);
    sprintf(text, "Cantidad: %d", cantidad_agua);
    cantidad_agua = cantidad_agua * 10 + (tecla - '0');  // Construir la cantidad
    writeString(text);
  } else if (ingresando_cantidad_gaseosa && isdigit(tecla) && !ingresando_cantidad_agua) {
    limpiarLCD();
    writeString("Gaseosa: $700");
    moverCursor(1, 0);                                // Línea 1 (segunda línea), columna 0
    sprintf(text, "Cantidad: %d", cantidad_gaseosa);  // Convertir el número y concatenar
    cantidad_gaseosa = cantidad_gaseosa * 10 + (tecla - '0');  // Construir la cantidad
    writeString(text);
  }


  else if (tecla == '1' && !segundo_producto && !ingresando_cantidad_gaseosa) {
    limpiarLCD();
    writeString("Agua: $500");
    moverCursor(1, 0);                             // Línea 1 (segunda línea), columna 0
    sprintf(text, "Cantidad: %d", cantidad_agua);  // Convertir el número y concatenar
    writeString(text);
    ingresando_cantidad_agua = true;  // Activar el modo de ingreso de cantidad
    segundo_producto = true;
  } else if (tecla == '2' && !segundo_producto && !ingresando_cantidad_agua) {
    limpiarLCD();
    writeString("Gaseosa: $700");
    moverCursor(1, 0);                                // Línea 1 (segunda línea), columna 0
    sprintf(text, "Cantidad: %d", cantidad_gaseosa);  // Convertir el número y concatenar
    writeString(text);
    ingresando_cantidad_gaseosa = true;  // Activar el modo de ingreso de gaseosa
    segundo_producto = true;
  }

  else if (tecla == '=' && segundo_producto && !ingresando_cantidad_agua && (cantidad_gaseosa > 0 || cantidad_agua > 0)) {
    limpiarLCD();
    writeString("Agua: $500");
    moverCursor(1, 0);                                   // Línea 1 (segunda línea), columna 0
    sprintf(text, "Cantidad: %d", cantidad_agua);        // Convertir el número y concatenar
    cantidad_agua = cantidad_agua * 10 + (tecla - '0');  // Construir la cantidad
    ingresando_cantidad_agua = true;                     // Activar el modo de ingreso de cantidad
    ingresando_cantidad_gaseosa = false;
    writeString(text);
  } else if (tecla == '=' && segundo_producto && !ingresando_cantidad_gaseosa && (cantidad_gaseosa > 0 || cantidad_agua > 0)) {
    limpiarLCD();
    writeString("Gaseosa: $700");
    moverCursor(1, 0);                                         // Línea 1 (segunda línea), columna 0
    sprintf(text, "Cantidad: %d", cantidad_gaseosa);           // Convertir el número y concatenar
    cantidad_gaseosa = cantidad_gaseosa * 10 + (tecla - '0');  // Construir la cantidad
    ingresando_cantidad_gaseosa = true;                        // Activar el modo de ingreso de gaseosa
    ingresando_cantidad_agua = false;
    writeString(text);
  }

  // else if (tecla == '1' && ingresando_cantidad_agua && !ingresando_cantidad_Gaseosa && !segundo_producto) {
  //   limpiarLCD();
  //   writeString("Agua: $500");
  //   moverCursor(1, 0);                             // Línea 1 (segunda línea), columna 0
  //   sprintf(text, "Cantidad: %d", cantidad_agua);  // Convertir el número y concatenar
  //   writeString(text);

  //   ingresando_cantidad_agua = true;  // Activar el modo de ingreso de cantidad
  // }



  // else if (tecla == '2' && !ingresando_cantidad_agua && !ingresando_cantidad_Gaseosa && !segundo_producto) {
  //   limpiarLCD();
  //   writeString("Gaseosa: $700");
  //   moverCursor(1, 0);                                // Línea 1 (segunda línea), columna 0
  //   sprintf(text, "Cantidad: %d", cantidad_gaseosa);  // Convertir el número y concatenar
  //   writeString(text);
  //   ingresando_cantidad_Gaseosa = true;  // Activar el modo de ingreso de cantidad

  // } else if (ingresando_cantidad_Gaseosa && isdigit(tecla) && !ingresando_cantidad_agua) {
  //   cantidad_gaseosa = cantidad_gaseosa * 10 + (tecla - '0');  // Construir la cantidad
  //   limpiarLCD();
  //   writeString("Gaseosa: $700");
  //   moverCursor(1, 0);
  //   sprintf(text, "Cantidad: %d", cantidad_gaseosa);
  //   writeString(text);
  //   ingresando_cantidad_Gaseosa = true;
  // } else if (ingresando_cantidad_agua && isdigit(tecla) && !ingresando_cantidad_Gaseosa) {
  //   cantidad_agua = cantidad_agua * 10 + (tecla - '0');  // Construir la cantidad
  //   limpiarLCD();
  //   writeString("Agua: $500");
  //   moverCursor(1, 0);                             // Línea 1 (segunda línea), columna 0
  //   sprintf(text, "Cantidad: %d", cantidad_agua);  // Convertir el número y concatenar
  //   writeString(text);
  //   ingresando_cantidad_agua = true;
  //   ingresando_cantidad_Gaseosa = true;
  // } else if (tecla == '=' && (cantidad_gaseosa > 0 || cantidad_agua > 0) && !segundo_producto) {
  //   segundo_producto = true;
  //   if (cantidad_gaseosa == 0) {
  //     limpiarLCD();
  //     writeString("Gaseosa: $700");
  //     moverCursor(1, 0);                                // Línea 1 (segunda línea), columna 0
  //     sprintf(text, "Cantidad: %d", cantidad_gaseosa);  // Convertir el número y concatenar
  //     writeString(text);
  //     ingresando_cantidad_Gaseosa = true;
  //   } else {
  //     limpiarLCD();
  //     writeString("Agua: $500");
  //     moverCursor(1, 0);
  //     sprintf(text, "Cantidad: %d", cantidad_agua);
  //     writeString(text);
  //     ingresando_cantidad_agua = true;
  //   }
  // } else if (tecla == '=' && cantidad_gaseosa > 0 && cantidad_agua > 0 && segundo_producto && !mostro_total_agua) {
  //   segundo_producto = true;
  //   limpiarLCD();
  //   total_agua = cantidad_agua * 500;
  //   sprintf(text, "Agua $500 x %d", cantidad_agua);
  //   writeString(text);
  //   moverCursor(1, 0);
  //   sprintf(text2, "Total = %d", total_agua);
  //   writeString(text2);
  //   mostro_total_agua = true;
  // } else if (tecla == '=' && cantidad_gaseosa > 0 && cantidad_agua > 0 && segundo_producto && !mostro_total_gaseosa) {
  //   segundo_producto = true;
  //   limpiarLCD();
  //   total_gaseosa = cantidad_gaseosa * 700;
  //   sprintf(text, "Gaseosa $700 x %d", cantidad_gaseosa);
  //   writeString(text);
  //   moverCursor(1, 0);
  //   sprintf(text2, "Total = %d", total_gaseosa);
  //   writeString(text2);
  //   mostro_total_gaseosa = true;
  // }
}

void manejarINT0() {  //Rutinas de servicio de interrupción(ISR)
  contadorINT0++;     // Incrementar contador de INT0
}
void manejarINT1() {
  contadorINT1++;  // Incrementar contador de INT1
}
void manejarINT2() {
  contadorINT2++;  // Incrementar contador de INT2
}
