#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <OneWire.h>

OneWire  _ds(2);  // on pin 10 (a 4.7K resistor is necessary)


ESP8266WebServer server(80);

// chez nous
const char* ssid = "CHEVALLIER_BORDEAU"; //Enter Wi-Fi SSID
const char* password =  "9697abcdea"; //Enter Wi-Fi Password
const char* WURL = "http://176.161.19.7:8080/main";
// deuxieme mcu 
//const char* WURL = "http://176.161.19.7:8081/main";

// chez pepito
//const char* ssid = "Bbox-09179E72"; //Enter Wi-Fi SSID
//const char* password =  "114564D1FA44C45A736FF6AE6D5E3C"; //Enter Wi-Fi Password
//const char* WURL = "http://176.161.19.7:8080/main";


long count = 0;
int ledv = 1>2;
long start = 0;

// 15 = GPIO15, PIN=D8 on board
long PINOUT=15;
void handle_index_main() {
  start = count;
  Serial.print("handle_index_main");
  //Print Hello at opening homepage
  String message("count =");
  message += String(count);
  server.send(200, "text/html", message.c_str());
  //"Hello! This is an index page.");
  int v = ledv ? LOW : HIGH;
  ledv = !ledv;
  digitalWrite(2, LOW);   // Turn the LED on (Note that LOW is the voltage level
                                    // but actually the LED is on; this is because 
                                    // it is acive low on the ESP-01)
  digitalWrite(PINOUT, HIGH); 
  delay(2000);
  digitalWrite(2, HIGH);   // Turn the LED on (Note that LOW is the voltage level
  digitalWrite(PINOUT, LOW); 
  Serial.println("end");
}

 
 String page(R""""(
<!DOCTYPE html>
<html>
  <head>
    <meta http-equiv="Content-type" content="text/html; charset=utf-8" />
    <title>Ouverture du garage</title>
    <style>
    .bb {
        font-size: 80px;
    }
    </style>
  </head>
  <body class="bb">
  <div id="statut"> La porte est ??? </div>
  <div>
    <button class="bb", id="ouvrir">Ouvrir</button>
  </div>
    <script>
      count = "a";
      const accueil = "Tapez le code";
      const button = document.getElementById("ouvrir");

      function statut() {
        murl = "statut_porte";
        console.log("fetching");
        count = count + "a";
        //document.getElementById("statut").innerHTML = "fetching";
        fetch(murl).then(function(response) {
          console.log("reponse");
          d = response.json();
          console.log(d);
          //console.log(d["porte"]);
          return d;
        }).then(function(data) {
          //button.innerHTML = count;
          console.log("data");
          console.log(data);
          document.getElementById("statut").innerHTML = "La porte est " + data["porte"];
          setTimeout(statut, 1000);
        }).catch(function(ee) {
          console.log("Booo");
          //document.getElementById("statut").innerHTML = ee;
          
        });
      }
      const myTimeout = setTimeout(statut, 1000);

      cookies = document.cookies;
      console.log(cookies);
      reset = function(){
                      button.style.fontSize="100px";
                      button.innerHTML = accueil;
                      button.disabled = false;
                  };
      //const url = "http://78.207.134.29:8083/main";
      //const url = "http://78.207.134.29:8083/main";
      //const url = "http://78.207.134.29:8083/main";
      const url = "WURL";
      //const url = "http://192.168.1.95/main";
      
      function getCookie(cname) {
        let name = cname + "=";
        let decodedCookie = decodeURIComponent(document.cookie);
        let ca = decodedCookie.split(';');
        for(let i = 0; i <ca.length; i++) {
          let c = ca[i];
          while (c.charAt(0) == ' ') {
            c = c.substring(1);
          }
          if (c.indexOf(name) == 0) {
            return c.substring(name.length, c.length);
          }
        }
        return "";
      }
      code =getCookie("eportiercode");
      console.log(code);
      function ouvre(cde) {
                console.log(cde);
                document.cookie = "eportiercode=" + cde + ";SameSite=Strict";
                cookies = document.cookies;
                console.log(cookies);
                cookies = document.cookie;
                console.log(cookies);
                button.style.fontSize="30px";
                button.innerHTML = "le verrou va s'ouvrir ...";
                setTimeout(() => {
                        button.disabled = true;
                        console.log(url+code)
                        window.fetch(url+cde, { mode: 'no-cors'}).then((result) => {
                                console.log(result);
                                if (result.ok) {                                
                                  button.innerHTML = "Système contacté,<br>déverrouillage pendant 2 secondes ...";
                                } else {
                                  button.innerHTML = "Système contacté .. bad luck!";
                                }
                                setTimeout(reset, 2000);
                        }).catch((e) => {
                                button.innerHTML = "Pas moyen de contacter le système!.. fetching " + url + cde;
                                setTimeout(reset, 2000);
                        })}, 
                        1000);
      }
      button.addEventListener('click', function() { ouvre(code); });
      reset();
      buttons = [];
      clicked = [];
      function addbutton(txt, x, y) {
        // Create a button element
          const nbutton = document.createElement('button');
          nbutton.innerText = txt;
          console.log(nbutton);
          nbutton.style.position = "absolute";
          nbutton.style.left = x + 'px';
          nbutton.style.top = y + 'px';
          nbutton.style.fontSize = "120px";
          nbutton.style.backgroundColor = 'White';
          nbutton.addEventListener('click', function() {
              console.log( this );
              this.style.backgroundColor = 'Red';
              this.disabled = true;
              clicked.push(this);
              if (clicked.length == 5) {
                code = "";
                for (i=0; i < clicked.length; i++) {
                  b = clicked[i];
                  code += b.innerText;
                  b.style.backgroundColor = 'White';
                  b.disabled = false;
                };
                ouvre(code);
                //console.log(code);
                clicked = [];
              }
          });
        buttons.push(nbutton);
        document.body.appendChild(nbutton);
      };
      W=30*4;
      H=75*2;
      ML=  100;
      MH = 230;
      //console.log("create buttons");
      for (i = 1; i < 10; i++) {
        console.log(i);
        addbutton(i, ((i-1) % 3) * W + ML, (~~((i-1) / 3) * H) + MH); 
      }                        
    </script>
  </body>
</html>
)"""");


bool porte_ouverte() {
  Serial.println("statut porte");
  int a0 = analogRead(A0);
  return a0 < 500;
}


void handle_index() {
  Serial.println("index");
  int a0 = analogRead(A0);
  Serial.println("a0=" + String(a0));

  String porte(porte_ouverte() ? "ouverte" : "fermée");
  
  String npage(page);
  npage.replace("WURL", WURL);
  npage.replace("PORTE", porte);
 
  server.send(505, "text/html", npage.c_str());
  Serial.println("end");
}

void setup() {
  
  pinMode(A0,INPUT);

  Serial.begin(115200); //Begin Serial at 115200 Baud
  Serial.print("starting");
  delay(10);
  
  WiFi.begin(ssid, password);  //Connect to the WiFi network
  
  while (WiFi.status() != WL_CONNECTED) {  //Wait for connection
      delay(500);
      Serial.println("Waiting to connect...");
  }
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //Print the local IP
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
  server.on("/", handle_index); //Handle Index page
  server.on("/main96713", handle_index_main); //Handle Index page

  server.on("/statut_porte", HTTP_GET, []() {
      String message = "POST form was:\n";
      for (uint8_t i = 0; i < server.args(); i++) { message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; }
      Serial.println(message);
      String stat(porte_ouverte() ? "ouverte" : "fermée");
      String json = "{ \"porte\" : \"" + stat + "\" }";
      Serial.println(json);
      server.send(200, "text/json", json);
  });
  
  server.begin(); //Start the server
  Serial.println("setup");
  pinMode(2, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(2, HIGH);  

  // INPUT ANALOG
  pinMode(A0,INPUT);

  
  pinMode(PINOUT, OUTPUT);
  digitalWrite(PINOUT, LOW);  
  Serial.println("Server listening");
}


void onewire(OneWire &ds) {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
}


void loop() {
  onewire(_ds);
  server.handleClient(); //Handling of incoming client requests
  count += 1;
  
}

