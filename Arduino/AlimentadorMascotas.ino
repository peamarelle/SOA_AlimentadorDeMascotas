// Bibliotecas de C:
#include <Servo.h>
#include <stdio.h>
#include <SoftwareSerial.h>
// Actuadores y sensores:
#define PIN_SENSOR_DISPENSER                  A1
#define PIN_SENSOR_PLATO                    A0
#define PIN_SENSOR_PIR                              13
#define PIN_LED                     8
#define PIN_SERVOMOTOR                              11

// Definición de estados del sistema embebido:
enum estados {ESTADO_DISPENSER_VACIO, 
                ESTADO_DISPENSER_LLENO, 
                ESTADO_ESPERANDO, 
                ESTADO_CARGANDO,
                ESTADO_INICIAL} estadoActual;

// Definición de eventos disponibles del sistema embebido:
enum eventos {EVENTO_LLENE_DISPENSER, 
                EVENTO_VACIE_DISPENSER, 
                EVENTO_CARGUE_PLATO, 
                EVENTO_MASCOTA_COMIO_COMIDA, 
                EVENTO_MASCOTA_PRESENTE,
                EVENTO_MASCOTA_AUSENTE, 
                EVENTO_ANDROID_APAGAR_LED,
                EVENTO_ANDROID_ENVIAR_SENSORES,
                EVENTO_VIRTUAL} nuevoEvento;

// Definición de constantes para el funcionamiento:
#define CANTIDAD_SENSORES                 3
#define ANGULO_MIN_PLATO                            20
#define ANGULO_MAX_PLATO                            50
#define ANGULO_MIN_DISPENSER                        20
#define COMPUERTA_ABIERTA                           45
#define COMPUERTA_CERRADA                           0
#define FLEX_MIN                                    0
#define FLEX_MAX                                    1023
#define POT_MIN                                     0
#define POT_MAX                                     1023
#define UMBRAL_TIEMPO                               1000
#define SENSOR_DISPENSER                            0
#define SENSOR_PLATO                                1
#define SENSOR_PIR                                  2
#define VALOR_INICIAL_SENSORES                      -1
#define BAUDIOS_MONITOR_SERIAL                      9600
#define BLUETOOTH_RX                                3
#define BLUETOOTH_TX                                2
#define BAUDIOS_BLUETOOTH                           9600

// Definición de variables globales:
Servo servoMotor;
int estado, comidaEnDispenser;
bool tiempoCumplido;
long ultimoTiempoActual;

//Definción Serial Bluethoot
SoftwareSerial BTSerial(BLUETOOTH_RX,BLUETOOTH_TX);
String BTcontent;
String BTSendString;

// Definición de estructura para sensores:
struct estSensor
{
    int  pin;
    long valorActual;
    long valorPrevio;
};
estSensor sensores[CANTIDAD_SENSORES];

// Declaración de las funciones a utilizar: 
int leerSensorFlex(int puerto,int minV, int maxV);
bool verificarEstadoSensorDispenser();
bool verificarEstadoSensorPlato();
bool verificarEstadoSensorPIR();
bool verificarEstadoSensores();
void generarEvento();
void encenderLED();
void apagarLED();
void abrirCompuerta();
void cerrarCompuerta();
void maquinaEstadosAlimentadorMascotas();
boolean verificarBTSerial();
/*-----------------------------------------------------------------------------------------------------
Función: iniciarSistema()
Descripción: Esta función se ejecuta en el setup() de Arduino. Asigna los pines de la placa de desarrollo 
    para cada sensor y actuador. Inicializa el estado de la máquina de estados, el array de sensores y 
    las variables del temporizador por software.
-----------------------------------------------------------------------------------------------------*/
void iniciarSistema()
{

    pinMode(PIN_LED, OUTPUT); // Pin digital del LED.
    servoMotor.attach(PIN_SERVOMOTOR); // Pin digital PWM del servomotor.
    servoMotor.write(COMPUERTA_CERRADA);  
    
    pinMode(PIN_SENSOR_PLATO, INPUT); // Pin analógico del sensor flex.
    pinMode(PIN_SENSOR_DISPENSER, INPUT); // Pin analógico del sensor flex.
    pinMode(PIN_SENSOR_PIR, INPUT); // Pin digital del sensor PIR.

    estadoActual = ESTADO_INICIAL; 

    sensores[SENSOR_DISPENSER].pin = PIN_SENSOR_DISPENSER; 
    sensores[SENSOR_PLATO].pin = PIN_SENSOR_PLATO;
    sensores[SENSOR_PIR].pin = PIN_SENSOR_PIR;
    
    sensores[SENSOR_PLATO].valorPrevio = VALOR_INICIAL_SENSORES; // Asigna un valor inicial al valor previo de los sensores para realizar la primera verificación.
    sensores[SENSOR_DISPENSER].valorPrevio = VALOR_INICIAL_SENSORES;
    sensores[SENSOR_PIR].valorPrevio = VALOR_INICIAL_SENSORES;
    
    tiempoCumplido = false;
    ultimoTiempoActual = millis();
    
    Serial.begin(BAUDIOS_MONITOR_SERIAL); // Asigna el valor de baudios para el monitor serial.
    BTSerial.begin(BAUDIOS_BLUETOOTH);
}

/*-----------------------------------------------------------------------------------------------------
Función: leerSensorFlex(int puerto)
Descripción: Transforma el valor leído por el analogRead del sensor flex en un entero entre 0 y 255.
-----------------------------------------------------------------------------------------------------*/
int leerSensorFlex(int puerto,int minV, int maxV)
{
    int valor = map(analogRead(puerto), minV, maxV, 0, 255);
    return valor; 
}

/*-----------------------------------------------------------------------------------------------------
Función: verificarEstadoSensorDispenser()
Descripción: Se encargará de generar dos eventos según la lectura del sensor flex
    del dispenser, asignagndo el nuevo evento a una variable global.
-----------------------------------------------------------------------------------------------------*/
bool verificarEstadoSensorDispenser( )
{
    sensores[SENSOR_DISPENSER].valorActual = leerSensorFlex(PIN_SENSOR_DISPENSER,FLEX_MIN,FLEX_MAX);
    comidaEnDispenser = sensores[SENSOR_DISPENSER].valorActual;
    
    int valorActual = sensores[SENSOR_DISPENSER].valorActual;
    int valorPrevio = sensores[SENSOR_DISPENSER].valorPrevio;
    
    sensores[SENSOR_DISPENSER].valorPrevio = valorActual;

    
    if(valorActual < ANGULO_MIN_DISPENSER)
    {
       nuevoEvento = EVENTO_VACIE_DISPENSER; // Se genera cuando el valor actual del sensor es menor al mínimo declarado.
       return true;
    }
    else if(valorActual > valorPrevio)
    {
        nuevoEvento = EVENTO_LLENE_DISPENSER; // Se genera cuando el valor actual del sensor es mayor al valor previo.
        return true;
    }    

    return false;
}

/*-----------------------------------------------------------------------------------------------------
Función: verificarEstadoSensorPlato()
Descripción: Se encargará de generar dos eventos según la lectura del sensor flex
    del plato, asignando el nuevo evento a una variable global.
    Si la cantidad actual de comida es igual a la cantidad previa, la función devolverá
    falso, por lo que no genera un evento.
-----------------------------------------------------------------------------------------------------*/
bool verificarEstadoSensorPlato( )
{
    sensores[SENSOR_PLATO].valorActual = leerSensorFlex(PIN_SENSOR_PLATO,POT_MIN,POT_MAX);
    
    int valorActual = sensores[SENSOR_PLATO].valorActual;
    int valorPrevio = sensores[SENSOR_PLATO].valorPrevio;

    sensores[SENSOR_PLATO].valorPrevio = valorActual;
    
    if (valorActual == valorPrevio)
    {
        return false;
    }
    else if(valorActual < ANGULO_MIN_PLATO)
    {
        nuevoEvento = EVENTO_MASCOTA_COMIO_COMIDA; // Se genera cuando el valor actual del sensor es menor al mínimo declarado.
        return true;
    }
    else if( (valorActual >= ANGULO_MIN_PLATO) || 
        ((comidaEnDispenser < ANGULO_MAX_PLATO) && (valorActual >= comidaEnDispenser) ) )
    {
        nuevoEvento = EVENTO_CARGUE_PLATO; // Se genera cuando el valor actual es mayor al mínimo del plato o a la sumatoria del restante de comida
                                            // en el dispenser y la comida que había en el plato.
        return true;
    }

    return false;
}

/*-----------------------------------------------------------------------------------------------------
Función: verificarEstadoSensorPIR()
Descripción: Se encargará de generar dos eventos según la lectura del sensor PIR,
    asignando el nuevo evento a una variable global.
-----------------------------------------------------------------------------------------------------*/
bool verificarEstadoSensorPIR( )
{
    sensores[SENSOR_PIR].valorActual = digitalRead(PIN_SENSOR_PIR);
    
    int valorActual = sensores[SENSOR_PIR].valorActual;
   
    if(estadoActual == ESTADO_ESPERANDO)
    {
     if( valorActual == HIGH )
      {
        nuevoEvento = EVENTO_MASCOTA_PRESENTE; // Se genera cuando el sensor se encuentra en HIGH.
        return true;
      }
      else  
      {
        nuevoEvento = EVENTO_MASCOTA_AUSENTE; // Se genera cuando el sensor se encuentra en LOW.
        return true;
      }     
    
    }

    return false;
}

/*-----------------------------------------------------------------------------------------------------
Función: verificarEstadoSensores()
Descripción: Se encargará de generar los correspondientes eventos,
    ejecutando las funciones que verifican el estado de cada uno de los sensores.
-----------------------------------------------------------------------------------------------------*/
bool verificarEstadoSensores( )
{
 
    if(verificarEstadoSensorDispenser() == true ||verificarEstadoSensorPlato() == true || verificarEstadoSensorPIR() == true )
    {
      Serial.print("Sensor dispenser: ");
      Serial.println(sensores[SENSOR_DISPENSER].valorActual);
      Serial.print("Sensor plato: ");
      Serial.println(sensores[SENSOR_PLATO].valorActual);
      
      return true;
    }
}

/*-----------------------------------------------------------------------------------------------------
Función: generarEvento()
Descripción: Cuenta con una implementación de un temporizador por software.
    Mientras el temporizador se encuentre activado se generarán enventos virtuales.
    Una vez que el termporizador finalice, se guardará un nuevo tiempo actual y se llamará a la función
    verificarEstadoSensores(). Ésta, de haberse registrado un nuevo evento, nos devolverá true.    
    Además, canaliza los nuevos eventos en una sola función.
-----------------------------------------------------------------------------------------------------*/
void generarEvento( )
{
  long tiempoActual = millis();
  int  diferencia = (tiempoActual - ultimoTiempoActual);
  tiempoCumplido = (diferencia > UMBRAL_TIEMPO)? (true):(false);

  if(tiempoCumplido) // Si se cumple el tiempo del temporizador por software, no hubo un nuevo evento. 
  {
    tiempoCumplido = false;
    ultimoTiempoActual = tiempoActual;
        /*Serial.println("Sensor Dispenser: ");
        Serial.println(leerSensorFlex(PIN_SENSOR_DISPENSER,FLEX_MIN,FLEX_MAX));
        Serial.println("Sensor Plato:");
        Serial.println(leerSensorFlex(PIN_SENSOR_PLATO,POT_MIN,POT_MAX));
        Serial.println("Sensor PIR");
        Serial.println(sensores[SENSOR_PIR].valorActual);
      */
        if(verificarBTSerial())
        {
            Serial.print("True");
            return;
        }
        else if(verificarEstadoSensores())
        {
          return;
        }
    }
      
  nuevoEvento = EVENTO_VIRTUAL; // Se genera un evento virtual.
}

boolean verificarBTSerial(){
  Serial.println("Entro BT");
  if (BTSerial.available() > 0) {
    Serial.println("Entro Available");  
    BTcontent = BTSerial.readStringUntil('\n');
    Serial.println(BTcontent);
    if (BTcontent.equals("LED_ON")){
      nuevoEvento = EVENTO_ANDROID_APAGAR_LED;
      return true;
    }
   if (BTcontent.equals("LED_OFF")){
     nuevoEvento = EVENTO_ANDROID_APAGAR_LED;
    return true;
  }
   if (BTcontent.equals("GET_SENSOR_VALUE")){
    nuevoEvento = EVENTO_ANDROID_ENVIAR_SENSORES;
    BTSendString = "SENSOR_VALUE,"+String(sensores[SENSOR_DISPENSER].valorActual)+","+String(sensores[SENSOR_PLATO].valorActual)+"|";
    Serial.println(BTSendString);
   return true;
   }
  BTcontent="";
  }
  return false;
}


/*-----------------------------------------------------------------------------------------------------
Función: encenderLED()
Descripción: Utiliza la función digitalWrite para encender el LED.
-----------------------------------------------------------------------------------------------------*/
void encenderLED()
{
    digitalWrite(PIN_LED, HIGH);
}

/*-----------------------------------------------------------------------------------------------------
Función: apagarLED()
Descripción: Utiliza la función digitalWrite para apagar el LED.
-----------------------------------------------------------------------------------------------------*/
void apagarLED()
{
     digitalWrite(PIN_LED, LOW);
}

/*-----------------------------------------------------------------------------------------------------
Función: abrirCompuerta()
Descripción: Utiliza la función write propia del servomotor para abrir la compuerta inferior móvil 
    del dispenser, que deja caer la comida.
-----------------------------------------------------------------------------------------------------*/
void abrirCompuerta()
{
    servoMotor.write(COMPUERTA_ABIERTA);
}

/*-----------------------------------------------------------------------------------------------------
Función: cerrarCompuerta()
Descripción: Utiliza la función write propia del servomotor para cerrar la compuerta inferior móvil 
    del dispenser.
-----------------------------------------------------------------------------------------------------*/
void cerrarCompuerta()
{
    servoMotor.write(COMPUERTA_CERRADA);
}

/*-----------------------------------------------------------------------------------------------------
Función: maquinaEstadosAlimentadorMascotas()
Descripción: Esta función se ejecuta en el loop() de Arduino. Implementa el patrón de máquina de estados 
    a través de instrucciones condicionales: dos niveles de switch, uno para los estados y uno para los 
    eventos en cada estado. Muestra a través del monitor serial en cada nuevo evento el nombre del estado 
    anterior al evento y del evento. 
-----------------------------------------------------------------------------------------------------*/
void maquinaEstadosAlimentadorMascotas()
{
    generarEvento();
    
    switch(estadoActual) // Según el estado actual, evalúa el nuevo evento.
    {
        case ESTADO_INICIAL:
        {
            switch(nuevoEvento) // Según el nuevo evento, reacciona de determinada manera.
            {
                case EVENTO_VIRTUAL: // Estado inicial, evento virtual: realiza la transición al estado dispenser lleno.
                {
                    Serial.println("--------------------------------------------------");
                    Serial.println("ESTADO_INICIAL");
                    Serial.println("EVENTO_VIRTUAL");
                    Serial.println("--------------------------------------------------");
                    
                    estadoActual = ESTADO_DISPENSER_LLENO;
                }
                break;
                
                default: // Estado inicial, cualquier otro evento: no realiza ninguna acción.
                    Serial.println("--------------------------------------------------");
                    Serial.println("ESTADO_INICIAL");
                    Serial.println("EVENTO_DESCONOCIDO");
                    Serial.println("--------------------------------------------------");
                break;
            }
        break;
        }

        case ESTADO_DISPENSER_LLENO:
        {
            switch(nuevoEvento)
            {
                case EVENTO_VIRTUAL: // Estado dispenser lleno, evento virtual: realiza la transición al estado actual.
                {
                    estadoActual = ESTADO_DISPENSER_LLENO;
                    
                }
                break;

                case EVENTO_MASCOTA_COMIO_COMIDA: // Estado dispenser lleno, evento mascota comió comida: realiza transición al estado esperando.
                {
                    Serial.println("--------------------------------------------------");
                    Serial.println("ESTADO_DISPENSER_LLENO");
                    Serial.println("EVENTO_VACIE_PLATO");
                    Serial.println("--------------------------------------------------");

                    estadoActual = ESTADO_ESPERANDO;
                }
                break;

                 case EVENTO_VACIE_DISPENSER: // Estado dispenser lleno, evento mascota comió comida: realiza transición al estado esperando.
                {
                    Serial.println("--------------------------------------------------");
                    Serial.println("ESTADO_DISPENSER_LLENO");
                    Serial.println("EVENTO_VACIE_DISPENSER");
                    Serial.println("--------------------------------------------------");
                    encenderLED();
                    estadoActual = ESTADO_DISPENSER_VACIO;
                }
                break;

                case EVENTO_ANDROID_ENVIAR_SENSORES: // 
                {
                    Serial.println("--------------------------------------------------");
                    Serial.println("ESTADO_DISPENSER_LLENO");
                    Serial.println("EVENTO_ANDROID_ENVIAR_SENSORES");
                    Serial.println("--------------------------------------------------");
                    BTSerial.print(BTSendString);
                    
                    estadoActual = ESTADO_DISPENSER_LLENO;
                }
                break;
                
                default:  // Estado dispenser lleno, cualquier otro evento: no realiza ninguna acción.
                    Serial.println("--------------------------------------------------");
                    Serial.println("ESTADO_DISPENSER_LLENO");
                    Serial.println(nuevoEvento);
                    Serial.println("EVENTO_DESCONOCIDO");
                    Serial.println("--------------------------------------------------");
                break;
            }
            break;
        }

        case ESTADO_DISPENSER_VACIO:
        {
            switch(nuevoEvento)
            {
                case EVENTO_VIRTUAL: // Estado dispenser vacío, evento virtual: realiza la transición al estado actual.
                {
                    estadoActual = ESTADO_DISPENSER_VACIO;
                }
                break;

                case EVENTO_LLENE_DISPENSER: // Estado dispenser vacío, evento llené dispenser: apaga el LED y realiza transición al estado dispenser lleno.
                {
                    Serial.println("--------------------------------------------------");
                    Serial.println("ESTADO_DISPENSER_VACIO");
                    Serial.println("EVENTO_LLENE_DISPENSER");
                    Serial.println("--------------------------------------------------");

                    apagarLED();

                    estadoActual = ESTADO_DISPENSER_LLENO;
                }
                break;
                case EVENTO_ANDROID_APAGAR_LED: // Estado dispenser vacío, desde la aplicación en android apago el led
                {
                    Serial.println("--------------------------------------------------");
                    Serial.println("ESTADO_DISPENSER_VACIO");
                    Serial.println("EVENTO_ANDROID_APAGAR_LED");
                    Serial.println("--------------------------------------------------");
                    apagarLED();
                    estadoActual = ESTADO_DISPENSER_VACIO;
                }
                break;
                case EVENTO_ANDROID_ENVIAR_SENSORES: // 
                {
                    Serial.println("--------------------------------------------------");
                    Serial.println("ESTADO_DISPENSER_LLENO");
                    Serial.println("EVENTO_ANDROID_ENVIAR_SENSORES");
                    Serial.println("--------------------------------------------------");
                    BTSerial.print(BTSendString);
                    
                    estadoActual = ESTADO_DISPENSER_VACIO;
                }
                break;

                default: // Estado dispenser lleno, cualquier otro evento: no realiza ninguna acción.
                    Serial.println("--------------------------------------------------");
                    Serial.println("ESTADO_DISPENSER_VACIO");
                    Serial.println(sensores[SENSOR_DISPENSER].valorActual);
                    Serial.println("EVENTO_DESCONOCIDO");
                    Serial.println("--------------------------------------------------");
                break;

              
                
            }
            break;
        }

        case ESTADO_ESPERANDO:
        {
            switch(nuevoEvento)
            {
                case EVENTO_VIRTUAL: // Estado esperando, evento virtual: realiza la transición al estado actual.
                {
                    estadoActual = ESTADO_ESPERANDO;
                }
                break;

                case EVENTO_MASCOTA_PRESENTE: // Estado esperando, evento mascota presente: realiza la transición al estado actual.
                {
                    Serial.println("--------------------------------------------------");
                    Serial.println("ESTADO_ESPERANDO");
                    Serial.println("EVENTO_MASCOTA_PRESENTE");
                    Serial.println("--------------------------------------------------");

                    estadoActual = ESTADO_ESPERANDO;
                }
                break;

                case EVENTO_MASCOTA_AUSENTE: // Estado esperando, evento mascota ausente: abre la compuerta inferior móvil del dispenser y realiza la transición al estado cargando.
                {
                    Serial.println("--------------------------------------------------");
                    Serial.println("ESTADO_ESPERANDO");
                    Serial.println("EVENTO_MASCOTA_AUSENTE");
                    Serial.println("--------------------------------------------------");

                    abrirCompuerta();

                    estadoActual = ESTADO_CARGANDO;
                }
                break;

                default: // Estado esperando, cualquier otro evento: no realiza ninguna acción.
                    Serial.println("--------------------------------------------------");
                    Serial.println("ESTADO_ESPERANDO");
                    Serial.println(nuevoEvento);
                    Serial.println("EVENTO_DESCONOCIDO");
                    Serial.println("--------------------------------------------------");
                break;
            }
            break;
        }

        case ESTADO_CARGANDO:
        {

            switch(nuevoEvento)
            {
                case EVENTO_VIRTUAL: // Estado cargando, evento virtual: realiza la transición al estado actual.
                {
                    estadoActual = ESTADO_CARGANDO;
                }
                break;

                case EVENTO_CARGUE_PLATO: // Estado cargando, evento cargué plato: cierra la compuerta inferior móvil del dispenser y realiza la transición al estado dispenser lleno.
                {
                    Serial.println("--------------------------------------------------");
                    Serial.println("ESTADO_CARGANDO");
                    Serial.println("EVENTO_CARGUE_PLATO");
                    Serial.println("--------------------------------------------------");

                    cerrarCompuerta();

                    estadoActual = ESTADO_DISPENSER_LLENO;
                }
                break;

                case EVENTO_VACIE_DISPENSER: // Estado cargando, evento vacié dispenser: enciende el LED y realiza transición al estado dispenser vacío.
                {
                    Serial.println("--------------------------------------------------");
                    Serial.println("ESTADO_DISPENSER_LLENO");
                    Serial.println("EVENTO_VACIE_DISPENSER");
                    Serial.println("--------------------------------------------------");

                    cerrarCompuerta();
                    
                    encenderLED();

                    estadoActual = ESTADO_DISPENSER_VACIO;
                }
                break;

                default: // Estado cargando, cualquier otro evento: no realiza ninguna acción.
                    Serial.println("--------------------------------------------------");
                    Serial.println("ESTADO_CARGANDO");
                    Serial.println("EVENTO_DESCONOCIDO");
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
