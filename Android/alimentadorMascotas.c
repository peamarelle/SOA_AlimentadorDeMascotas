// C++ code
//
#include <Servo.h>
#include <stdio.h>

// Actuadores y Sensores
#define LED 8
#define SENSOR_PLATO A0
#define SENSOR_DISPENSER A1
#define SENSOR_PROX 13
#define SERVOMOTOR 12

// Definición de Estados
#define PLATO_LLENO_ST 0
#define PLATO_VACIO_ST 1
#define MASCOTA_PRESENTE_ST 2
#define CARGANDO_PLATO_ST 3
#define DISPENSER_VACIO_ST 4

// Definición Constantes de Funcionamiento
#define ANGULO_MIN_PLATO 20
#define ANGULO_MAX_PLATO 50
#define ANGULO_MIN_DISPENSER 30
#define COMPUERTA_ABIERTA 135
#define COMPUERTA_CERRADA 0
#define COMIDA_EN_PLATO 50
#define FLEX_MIN 767
#define FLEX_MAX 964

// Definición de constantes de Timer
#define UMBRAL_TIEMPO 5000
unsigned long tiempo_actual;
unsigned long tiempo_anterior;

// Definición de Funciones
int readFlex(int puerto);
void chequear_dispenser_plato_vacio();
void chequear_mascota_presente();
void chequear_carga_completa();
void chequear_dispenser_lleno();
void cargar_plato();

// Definición de Variables Globales
Servo servoMotor;
int estado;
int angulo_dispenser = 0, angulo_plato = 0;

void setup()
{

    pinMode(LED, OUTPUT); // Pin digital LED
    servoMotor.attach(SERVOMOTOR);
    servoMotor.write(COMPUERTA_CERRADA);

    pinMode(SENSOR_PLATO, INPUT);     // Pin analógico SENSOR PLATO
    pinMode(SENSOR_DISPENSER, INPUT); // pin analógico SENSOR DISPENSER
    pinMode(SENSOR_PROX, INPUT);      // pin digital SENSOR PIR

    estado = PLATO_LLENO_ST;    // Estado arranca en Estado inicial
    tiempo_anterior = millis(); // Inicializa el tiempo.

    Serial.begin(9600);
}

void loop()
{
    tiempo_actual = millis();
    if ((tiempo_actual - tiempo_anterior) >= UMBRAL_TIEMPO)
    {
        tiempo_anterior = tiempo_actual; 
        switch (estado)
        {
        case PLATO_LLENO_ST:
        {
            chequear_dispenser_plato_vacio();
            break;
        }
        case PLATO_VACIO_ST:
        {
            chequear_mascota_presente();

            if (estado == MASCOTA_PRESENTE_ST)
                break;
            cargar_plato();
            break;
        }
        case MASCOTA_PRESENTE_ST:
        {
            chequear_mascota_presente();
            break;
        }
        case CARGANDO_PLATO_ST:
        {
            chequear_carga_completa();
            break;
        }
        case DISPENSER_VACIO_ST:
        {
            digitalWrite(LED, HIGH);
            chequear_dispenser_lleno();
            break;
        }
        }
    }
}


/*----------------------------------------------------------------------------------------------------------------------------
-- Función: read_flex                                                                                                        -
-- Descripción: Transforma el valor leído por el analogRead de un Sensor flex, en un valor entre 0 y 255                     -
--                                                                                                                           -
----------------------------------------------------------------------------------------------------------------------------*/
int readFlex(int puerto)
{
    int valor = map(analogRead(puerto), FLEX_MIN, FLEX_MAX, 0, 255);
    return valor;
}

/*---------------------------------------------------------------------------------------------------------------------------
-- Función: chequear_dispenser_plato_vacio                                                                                  -
-- Descripción: Lee el sensor flex del dispenser y del plato.                                                               -
--         Si la lectura del sensor dispenser es menor que el minimo definido el estado cambia a DISPENSER_VACIO y retorna. -
--         Si la lectura del sensor plato es menor que el mínimo definido el estado cambia a PLATO_VACIO y retorna.         -
--                                                                                                                          -
----------------------------------------------------------------------------------------------------------------------------*/


void chequear_dispenser_plato_vacio()
{
    if (readFlex(SENSOR_DISPENSER) < ANGULO_MIN_DISPENSER)
    {
        estado = DISPENSER_VACIO_ST;
        return;
    }

    if (readFlex(SENSOR_PLATO) < ANGULO_MIN_PLATO)
    {
        estado = PLATO_VACIO_ST;
    }
    return;
}

/*----------------------------------------------------------------------------------------------------------------------------
-- Función: chequear_mascota_presente                                                                                        -
-- Descripción: Lee el sensor PIR que indica si la mascota esta presente.                                                    -
--         Si la lectura del sensor PIR es HIGH el estado cambia a MASCOTA_PRESENTE  y retorna.                              -
--         Si la lectura del sensor PIR es LOW  el estado vuelve a PLATO_VACIO_ST y retorna.                                 -
--                                                                                                                           -
----------------------------------------------------------------------------------------------------------------------------*/
void chequear_mascota_presente()
{
    if (digitalRead(SENSOR_PROX) == HIGH)
    {
        estado = MASCOTA_PRESENTE_ST;
        return;
    }
    else 
    {
        estado = PLATO_VACIO_ST;
    }
    return;
}

/*----------------------------------------------------------------------------------------------------------------------------
-- Función: chequear_carga_completa                                                                                          -
-- Descripción: Lee el sensor Flex del plato de comida                                                                       -
--         Si la lectura del Flex es mayor que al máximo definido por parámetro (Significa que el plato se lleno)            -
--         cambio el estado PLATO_LLENO                                                                                      -
--                                                                                                                           -
----------------------------------------------------------------------------------------------------------------------------*/
void chequear_carga_completa()
{
    if (readFlex(SENSOR_PLATO) >= ANGULO_MAX_PLATO )
    {
        servoMotor.write(COMPUERTA_CERRADA); // CIERRO COMPUERTA
        estado = PLATO_LLENO_ST;
    }

    if(ANGULO_MAX_PLATO >= angulo_dispenser && (readFlex(SENSOR_PLATO) >= angulo_plato + angulo_dispenser)){
        servoMotor.write(COMPUERTA_CERRADA); // CIERRO COMPUERTA
        estado = PLATO_LLENO_ST;
    }
    return;
}

/*----------------------------------------------------------------------------------------------------------------------------
-- Función: chequear_dispenser_lleno                                                                                         -
-- Descripción: Lee el sensor Flex del dispenser de comida                                                                   -
--         Si la lectura del Flex es mayor que al máximo definido por parámetro (Significa que el plato se lleno)            -
--         apago el led que indica dispenser vacío y cambio al estado inicial PLATO_LLENO                                    -
--                                                                                                                           -
----------------------------------------------------------------------------------------------------------------------------*/

void chequear_dispenser_lleno()
{
    if (readFlex(SENSOR_DISPENSER) >= ANGULO_MIN_DISPENSER)
    {
        digitalWrite(LED, LOW);
        estado = PLATO_LLENO_ST;
    }
    return;
}

/*----------------------------------------------------------------------------------------------------------------------------
-- Función: cargar_plato                                                                                                     -
-- Descripción: Realiza las operaciones necesarias para cargar el plato de comida:                                           -
--         Guarda en angulo_dispenser la cantidad de comida del dispenser previa a la carga                                  -
--         Guarda en angulo_plato la cantidad de comida del plato previa a la carga                                          -
--         Escribe el servomotor para abrir el dispenser de comida y cambia el estado a CARGANDO_PLATO                       -
----------------------------------------------------------------------------------------------------------------------------*/


void cargar_plato()
{

    angulo_dispenser = readFlex(SENSOR_DISPENSER);
    angulo_plato = readFlex(SENSOR_PLATO);
    servoMotor.write(COMPUERTA_ABIERTA); // ABRE COMPUERTA
    estado = CARGANDO_PLATO_ST;
}
