#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>



ESP8266WebServer server(80);

const char* ssid = "CHEVALLIER_BORDEAU"; //Enter Wi-Fi SSID
const char* password =  "9697abcdea"; //Enter Wi-Fi Password
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
  Serial.print("end");
}

 
 String page(R""""(
<!DOCTYPE html>
<html>
  <head>
    <meta http-equiv="Content-type" content="text/html; charset=utf-8" />
    <title>Ouverture de la porte</title>
    <style>
    .bb {
        font-size: 80px;
    }
    </style>
  </head>
  <body>
  <div>
    <button class="bb", id="ouvrir">Ouvrir</button>
  </div>
    <script>
      const accueil = "Tapez le code";
      const button = document.getElementById("ouvrir");
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
      // const url = "http://176.161.19.7:8080/main"
      const url = "http://192.168.1.95/main";
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
                document.cookie = "eportiercode=" + cde;
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
                                //console.log(result);
                                if (result.ok) {                                
                                  button.innerHTML = "Système contacté,<br>déverrouillage pendant 2 secondes ...";
                                } else {
                                  button.innerHTML = "Système contacté .. bad luck!";
                                }
                                setTimeout(reset, 2000);
                        }).catch((e) => {
                                button.innerHTML = "Pas moyen de contacter le système! ;)..";
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
      MH = 200;
      //console.log("create buttons");
      for (i = 1; i < 10; i++) {
        console.log(i);
        addbutton(i, ((i-1) % 3) * W + ML, (~~((i-1) / 3) * H) + MH); 
      }                        
    </script>
  </body>
</html>
)"""");

void handle_index() {
  Serial.print("index");

  //page.replace(""", "\"");
 
  server.send(505, "text/html", page.c_str());
  Serial.print("end");
}

void setup() {
  
  Serial.begin(115200); //Begin Serial at 115200 Baud
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

void loop() {
  server.handleClient(); //Handling of incoming client requests
  count += 1;

}