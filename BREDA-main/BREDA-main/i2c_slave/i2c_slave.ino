#include <Wire.h> // Librería de arduino para comunicación I2C
#include <max6675.h> //  Librería de arduino para manejar el módulo MAX6675.
 
// Define Slave I2C Address - Dirección de I2C del esclavo
#define SLAVE_ADDR 9 // Te dirige al periférico (sensor), no aún a la dirección de memoria/registro es por asi decirlo (Dirección del dispositivo esclavo en la red I2C)

int size; // 'size' como variable global para su uso en funciones

//Define el número de bytes que queremos enviar como respuesta
#define ANSWERSIZE answer.length()  // Usa el tamaño de la cadena 'answer' para definir el tamaño del mensaje

// Esta macro no funcionará correctamente sin una definición previa y constante de 'answer'
String answer = "Hola soy el esclavo y este mensaje es mi respuesta"; // Inicialización de 'answer'

//------------------------------------------------------------------

void setup(){
  // Inicializa la comunicación I2C como esclavo
  Wire.begin(SLAVE_ADDR);

  // Registra la función a ejecutar al recibir datos desde el maestro
  Wire.onReceive(receiveEvent); 

  // Registra la función a ejecutar cuando el maestro solicita datos
  Wire.onRequest(requestEvent); 

  // Inicializa la comunicación serial a 9600 baudios
  Serial.begin(9600);
  Serial.println("Prueba de comunicación I2C con un esclavo");
}

//------------------------------------------------------------------

void receiveEvent(int howMany){   // Función ejecutada si desde el master se recibe una orden
  // Inicializa el tamaño del mensaje recibido
  size = 0;
  char question[100]; // Definir un buffer para almacenar las preguntas

  // Lee los bytes disponibles enviados por el maestro
  while (Wire.available()){
    question[size] = Wire.read(); // Esto [] ya no indica el tamaño, indica la posición que quieres revisar
    if (size < 99) size++; // Evitar desbordamiento del buffer
  }
  question[size] = '\0'; // Marca el final de la cadena para que C++ entienda que termina ahí la cadena y pueda operar con ella facilmente

  // Envía de vuelta la pregunta recibida
  Wire.write(question, size); // Escribe la question y la longitud que tiene (size)

  // Imprime la información recibida en el monitor serial
  Serial.println("Hola soy el esclavo y estoy recibiendo información del master"); // Imprime por serial ese string
}

//------------------------------------------------------------------

void requestEvent() { // Función ejecutada cuando me solicita datos el Master
  
  int ANSWERSIZE = answer.length(); // Definimos el tamaño de la respuesta justo antes de usarla
  
  byte response[ANSWERSIZE]; // Definimos un buffer (byte) que almacena la informacion de la respuesta
  
  //bucle 'for' en posiciones de byte desde 0 hasta llegar al tamaño de respuesta de uno en uno
  // Llena el buffer con los caracteres de la respuesta
  for (int i = 0; i < ANSWERSIZE; i++){
    response[i] = (byte)answer.charAt(i);
  }

  // Enviamos la respuesta al Master
  Wire.write(response, sizeof(response));
  
  Serial.println("Hola, soy el esclavo y el master me ha solicitado de información");  // Imprime la solicitud en el monitor serial
}
 
 //------------------------------------------------------------------

void loop()
{
  // Refresca la lectura cada medio segundo
  delay(500);
}