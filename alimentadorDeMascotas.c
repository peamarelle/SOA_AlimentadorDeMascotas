}
// Bibliotecas de C:
#include <Servo.h>
#include <stdio.h>

// Actuadores y sensores:
#define PIN_SENSOR_DISPENSER A1
#define PIN_SENSOR_PLATO A0
#define PIN_SENSOR_PIR 13
#define PIN_LED 8
#define PIN_SERVOMOTOR 11

// Definición de estados del sistema embebido:
enum estados
{
    ESTADO_DISPENSER_VACIO,
    ESTADO_DISPENSER_LLENO,
    ESTADO_ESPERANDO,
    ESTADO_CARGANDO_PLATO,
    ESTADO_INICIAL
} estadoActual;

// Eventos disponibles:
enum eventos
{
    EVENTO_LLENE_DISPENSER,
    EVENTO_VACIE_DISPENSER,
    EVENTO_CARGUE_PLATO,
    EVENTO_MASCOTA_COMIO_COMIDA,
    EVENTO_MASCOTA_PRESENTE,
    EVENTO_MASCOTA_AUSENTE,
    EVENTO_VIRTUAL
} nuevoEvento;

// Definición de constantes para el funcionamiento:
#define CANTIDAD_SENSORES 3
#define ANGULO_MIN_PLATO 20
#define ANGULO_MAX_PLATO 50
#define ANGULO_MIN_DISPENSER 30
#define COMPUERTA_ABIERTA 135
#define COMPUERTA_CERRADA 0
#define FLEX_MIN 767
#define FLEX_MAX 964
#define UMBRAL_TIEMPO 500
#define SENSOR_DISPENSER 0
#define SENSOR_PLATO 1
#define SENSOR_PIR 2

// Definición de variables globales:
Servo servoMotor;
int estado, comidaEnDispenser, anguloDispenser = 0, anguloPlato = 0;
bool tiempoCumplido;
long ultimoTiempoActual;

// Estructura para sensores:
struct estSensor
{
    int pin;
    long valorActual;
    long valorPrevio;
};
estSensor sensores[CANTIDAD_SENSORES];

void iniciarSistema()
{

    pinMode(PIN_LED, OUTPUT); // Pin digital del LED
    servoMotor.attach(PIN_SERVOMOTOR);
    servoMotor.write(COMPUERTA_CERRADA);

    pinMode(PIN_SENSOR_PLATO, INPUT);     // Pin analógico del sensor flex
    pinMode(PIN_SENSOR_DISPENSER, INPUT); // Pin analógico del sensor flex
    pinMode(PIN_SENSOR_PIR, INPUT);       // Pin digital del sensor PIR

    estadoActual = ESTADO_INICIAL; // Se inicializa en el estado inicial

    sensores[SENSOR_DISPENSER].pin = PIN_SENSOR_DISPENSER;
    sensores[SENSOR_PLATO].pin = PIN_SENSOR_PLATO;
    sensores[SENSOR_PIR].pin = PIN_SENSOR_PIR;

    sensores[SENSOR_PLATO].valorPrevio = -1;
    sensores[SENSOR_DISPENSER].valorPrevio = -1;
    sensores[SENSOR_PIR].valorPrevio = -1;

    tiempoCumplido = false;
    ultimoTiempoActual = millis();

    Serial.begin(9600);
}

int leerSensorFlex(int puerto)
{
    int valor = map(analogRead(puerto), FLEX_MIN, FLEX_MAX, 0, 255);
    return valor;
}

bool verificarEstadoSensorDispenser()
{
    sensores[SENSOR_DISPENSER].valorActual = leerSensorFlex(PIN_SENSOR_DISPENSER);
    comidaEnDispenser = sensores[SENSOR_DISPENSER].valorActual;

    int valorActual = sensores[SENSOR_DISPENSER].valorActual;
    int valorPrevio = sensores[SENSOR_DISPENSER].valorPrevio;

    Serial.println("DISPENSER:");
    Serial.println(sensores[SENSOR_DISPENSER].valorPrevio);
    Serial.println(sensores[SENSOR_DISPENSER].valorActual);

    if (valorActual != valorPrevio)
    {
        sensores[SENSOR_DISPENSER].valorPrevio = valorActual;

        if (valorActual < ANGULO_MIN_DISPENSER)
        {
            nuevoEvento = EVENTO_VACIE_DISPENSER;
        }
        else
        {
            nuevoEvento = EVENTO_LLENE_DISPENSER;
        }

        return true;
    }

    return false;
}

bool verificarEstadoSensorPlato()
{
    sensores[SENSOR_PLATO].valorActual = leerSensorFlex(PIN_SENSOR_PLATO);

    int valorActual = sensores[SENSOR_PLATO].valorActual;
    int valorPrevio = sensores[SENSOR_PLATO].valorPrevio;

    Serial.println("PLATO:");
    Serial.println(sensores[SENSOR_PLATO].valorPrevio);
    Serial.println(sensores[SENSOR_PLATO].valorActual);

    if ((valorActual != valorPrevio) || (valorPrevio < ANGULO_MIN_PLATO))
    {
        sensores[SENSOR_PLATO].valorPrevio = valorActual;

        if (valorActual < ANGULO_MIN_PLATO)
        {
            nuevoEvento = EVENTO_MASCOTA_COMIO_COMIDA;
        }
        else if ((valorActual >= ANGULO_MIN_PLATO) ||
                 ((ANGULO_MAX_PLATO > comidaEnDispenser) && (comidaEnDispenser >= valorActual)))
        {
            nuevoEvento = EVENTO_CARGUE_PLATO;
        }

        return true;
    }

    return false;
}

bool verificarEstadoSensorPIR()
{
    sensores[SENSOR_PIR].valorActual = digitalRead(PIN_SENSOR_PIR);

    int valorActual = sensores[SENSOR_PIR].valorActual;
    int valorPrevio = sensores[SENSOR_PIR].valorPrevio;

    Serial.println("PIR:");
    Serial.println(sensores[SENSOR_PIR].valorPrevio);
    Serial.println(sensores[SENSOR_PIR].valorActual);

    if ((valorActual != valorPrevio) || (valorPrevio == LOW))
    {
        sensores[SENSOR_PIR].valorPrevio = valorActual;

        if ((valorActual == HIGH))
        {
            nuevoEvento = EVENTO_MASCOTA_PRESENTE;
        }
        else
        {
            nuevoEvento = EVENTO_MASCOTA_AUSENTE;
        }

        return true;
    }

    return false;
}

void generarEvento()
{
    long tiempoActual = millis();
    int diferencia = (tiempoActual - ultimoTiempoActual);
    tiempoCumplido = (diferencia > UMBRAL_TIEMPO) ? (true) : (false);

    if (tiempoCumplido)
    {
        tiempoCumplido = false;
        ultimoTiempoActual = tiempoActual;

        if (verificarEstadoSensorDispenser() == true)
        {
            return;
        }
        else if (((estadoActual == ESTADO_CARGANDO_PLATO) || (estadoActual == ESTADO_DISPENSER_LLENO)) &&
                 (verificarEstadoSensorPlato() == true))
        {
            return;
        }
        else if ((estadoActual == ESTADO_ESPERANDO) && (verificarEstadoSensorPIR() == true))
        {
            return;
        }
    }

    nuevoEvento = EVENTO_VIRTUAL; // Se genera un evento "virtual"
}

void encenderLED()
{
    digitalWrite(PIN_LED, HIGH);
}

void apagarLED()
{
    digitalWrite(PIN_LED, LOW);
}

void abrirCompuerta()
{
    servoMotor.write(COMPUERTA_ABIERTA);
}

void cerrarCompuerta()
{
    servoMotor.write(COMPUERTA_CERRADA);
}

void maquinaEstadosAlimentadorMascotas()
{
    generarEvento();

    switch (estadoActual)
    {
        // Resolver esta función, utilizando un switch para los estados. Y para cada estado, un switch para cada evento en el que suceda algo.
        // Ver el diagrama de estados. Me basé en esto: https://www.tinkercad.com/things/0V42IYweGGr-maquina-de-estado-potenciometro-climatizdor-

    case ESTADO_INICIAL:
    {
        switch (nuevoEvento)
        {
        case EVENTO_VIRTUAL:
        {
            Serial.println("--------------------------------------------------");
            Serial.println("ESTADO_INICIAL");
            Serial.println("EVENTO_VIRTUAL");
            Serial.println("--------------------------------------------------");

            estadoActual = ESTADO_DISPENSER_LLENO;
        }
        break;

        default:
            Serial.println("--------------------------------------------------");
            Serial.println("ESTADO_INICIAL");
            Serial.print("EVENTO_DESCONOCIDO: ");
            Serial.println(nuevoEvento);
            Serial.println("--------------------------------------------------");
            break;
        }
        break;
    }

    case ESTADO_DISPENSER_LLENO:
    {
        switch (nuevoEvento)
        {
        case EVENTO_VIRTUAL:
        {
            estadoActual = ESTADO_DISPENSER_LLENO;
        }
        break;

        case EVENTO_VACIE_DISPENSER:
        {
            Serial.println("--------------------------------------------------");
            Serial.println("ESTADO_DISPENSER_LLENO");
            Serial.println("EVENTO_VACIE_DISPENSER");
            Serial.println("--------------------------------------------------");

            encenderLED();

            estadoActual = ESTADO_DISPENSER_VACIO;
        }
        break;

        case EVENTO_MASCOTA_COMIO_COMIDA:
        {
            Serial.println("--------------------------------------------------");
            Serial.println("ESTADO_DISPENSER_LLENO");
            Serial.println("EVENTO_VACIE_PLATO");
            Serial.println("--------------------------------------------------");

            estadoActual = ESTADO_ESPERANDO;
        }
        break;

        default:
            Serial.println("--------------------------------------------------");
            Serial.println("ESTADO_DISPENSER_LLENO");
            Serial.println("EVENTO_DESCONOCIDO: evento.nombre");
            Serial.println("--------------------------------------------------");
            break;
        }
        break;
    }

    case ESTADO_DISPENSER_VACIO:
    {
        switch (nuevoEvento)
        {
        case EVENTO_VIRTUAL:
        {
            estadoActual = ESTADO_DISPENSER_VACIO;
        }
        break;

        case EVENTO_LLENE_DISPENSER:
        {
            Serial.println("--------------------------------------------------");
            Serial.println("ESTADO_DISPENSER_VACIO");
            Serial.println("EVENTO_LLENE_DISPENSER");
            Serial.println("--------------------------------------------------");

            apagarLED();

            estadoActual = ESTADO_DISPENSER_LLENO;
        }
        break;

        default:
            Serial.println("--------------------------------------------------");
            Serial.println("ESTADO_DISPENSER_VACIO");
            Serial.println("EVENTO_DESCONOCIDO: evento.nombre");
            Serial.println("--------------------------------------------------");
            break;
        }
        break;
    }

    case ESTADO_ESPERANDO:
    {
        switch (nuevoEvento)
        {
        case EVENTO_VIRTUAL:
        {
            estadoActual = ESTADO_ESPERANDO;
        }
        break;

        case EVENTO_MASCOTA_PRESENTE:
        {
            Serial.println("--------------------------------------------------");
            Serial.println("ESTADO_ESPERANDO");
            Serial.println("EVENTO_MASCOTA_PRESENTE");
            Serial.println("--------------------------------------------------");

            estadoActual = ESTADO_ESPERANDO;
        }
        break;

        case EVENTO_MASCOTA_AUSENTE:
        {
            Serial.println("--------------------------------------------------");
            Serial.println("ESTADO_ESPERANDO");
            Serial.println("EVENTO_MASCOTA_AUSENTE");
            Serial.println("--------------------------------------------------");

            abrirCompuerta();

            estadoActual = ESTADO_CARGANDO_PLATO;
        }
        break;

        default:
            Serial.println("--------------------------------------------------");
            Serial.println("ESTADO_ESPERANDO");
            Serial.println("EVENTO_DESCONOCIDO: evento.nombre");
            Serial.println("--------------------------------------------------");
            break;
        }
        break;
    }

    case ESTADO_CARGANDO_PLATO:
    {
        switch (nuevoEvento)
        {
        case EVENTO_VIRTUAL:
        {
            estadoActual = ESTADO_CARGANDO_PLATO;
        }
        break;

        case EVENTO_CARGUE_PLATO:
        {
            Serial.println("--------------------------------------------------");
            Serial.println("ESTADO_CARGANDO");
            Serial.println("EVENTO_CARGUE_PLATO");
            Serial.println("--------------------------------------------------");

            cerrarCompuerta();

            estadoActual = ESTADO_DISPENSER_LLENO;
        }
        break;

        default:
            Serial.println("--------------------------------------------------");
            Serial.println("ESTADO_CARGANDO");
            Serial.println("EVENTO_DESCONOCIDO: evento.nombre");
            Serial.println("--------------------------------------------------");
            break;
        }
        break;
    }
    }
}

// Funciones de Arduino:
void setup()
{
    iniciarSistema();
}

void loop()
{
    maquinaEstadosAlimentadorMascotas();
}