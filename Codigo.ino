#include <SPI.h>
#include <Ethernet.h>
#include <Twitter.h>  //librería se descarga desde arduino-tweet.appspot.com y se guarda en la carpeta libraries del software de Arduino
// Variables
int analogZero,analogZero2; // Variables para pines analógicos que generan códigos al azar en Tweets
char msg[140]; // Mensaje para Twitter
byte lasercontrol = 0; // Control on / off del laser
int alarma= 3; // Pin para controlar on / off del led y el láser
int fotoreceptor = 0; // Valor del fotoreceptor en entrada A0
int fotoreceptorValor; // Valor del fotoreceptor que depende de la luz
String estado="Apagada"; // Alarma inicia Apagada
// Configuración de red en placa Arduino y Twitter 
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 90); // IP local asignada asignada a la placa Arduino
IPAddress gateway(192, 168, 0, 1); // IP del Router 
IPAddress subnet(255, 255, 255, 0);
EthernetServer server(8085); // Puerto del Arduino, se puede cambiar
Twitter twitter("1096747920-0JDh28ejoFzGcOe96cTmkDWOyiRk7ETCwk9Arva"); // Token de la cuenta de Twitter
void setup()
{
  Serial.begin(9600); 
  Ethernet.begin(mac, ip);
  server.begin();  
  pinMode(alarma,OUTPUT); 
}
void loop()
{  
  analogZero=analogRead(1);  // Lecturas de los pines A1 y A3 para generar códigos al azar y publicar en Twitter
  analogZero2=analogRead(3);
  EthernetClient client = server.available();  // Se configura el Web Server
  if (client) {
    boolean currentLineIsBlank = true; 
    String cadena=""; 
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        cadena.concat(c);
         int posicion=cadena.indexOf("ALARMA=");
          if(cadena.substring(posicion)=="ALARMA=ON") // Si la alarma se Enciende desde la App
          {
            digitalWrite(alarma,HIGH);
            estado="Encendida";    lasercontrol=1;  
          }
          if(cadena.substring(posicion)=="ALARMA=OFF") // Si la alarma se Apaga desde la App
          {
            digitalWrite(alarma,LOW);
            estado="Apagada";
            lasercontrol=0;
          }        
        if (c == '\n' && currentLineIsBlank) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println();
            // Página web HTML
            client.println("<html>");
            client.println("<head>");
            client.println("</head>");
            client.println("<body>");
            client.println("<center><img src=\"http://i.imgur.com/FCzkAUu.png\"></center>"); 
            client.println("<br /><h3 align='center'>Alarma = ");
            client.print(estado);
            client.println("</h3>");
            client.println("<div style='text-align:center;'>");
            client.println("<button onClick=location.href='./?ALARMA=ON\' style='margin:auto;background-color: #4CAF50;color: snow;padding: 10px;border: 1px solid #4CAF50;width:80px;'>");
            client.println("Encender");
            client.println("</button>");
            client.println("<button onClick=location.href='./?ALARMA=OFF\' style='margin:auto;background-color: #4CAF50;color: snow;padding: 10px;border: 1px solid #4CAF50;width:65px;'>");
            client.println("Apagar");
            client.println("</button>");
            client.println("<br /><br />");                      
            client.println("</body>");
            client.println("</html>");
            break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop(); 
  }
    // Se captura el valor del fotoreceptor, sin incidencia de la luz del láser es menor a 750
    fotoreceptorValor= analogRead(fotoreceptor);
     // Si el fotoreceptor deja de recibir luz del láser y ALARMA está ON
     if ( (fotoreceptorValor<750) && (lasercontrol==1) ) {                           
        // Se publica mensaje en Twitter, analogZero y analogZero2 publican números al azar para Twitter
        sprintf(msg, "Hay un intruso Revisa APP!: %d%d.", analogZero, analogZero2);     
            // Opcional muestra el mensaje en el Puerto Serie
            Serial.println(msg);
            Serial.println("Conectando a Twitter...");
            if (twitter.post(msg)) {
            int status = twitter.wait(&Serial);
            if (status == 200) {
            Serial.println("200 OK");
            } else {
            Serial.print("Error : code ");
            Serial.println(status);
            }
            } else {
            Serial.println("Fallo en la conexión.");
            }
     delay(60001); // Delay de un minuto para no hacer spam en Twitter       
     }           
}
