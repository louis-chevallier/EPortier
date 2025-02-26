function eko(x) {
    console.log(x)
}
var temps= [33, 44];
var trace1 = {
    y: temps,
    type: 'scatter'
}
var data = [trace1];
//Plotly.newPlot('plot_temperature', data);


const button_bruleur = document.querySelector('.toggleBruleur');
const button_circulateur = document.querySelector('.toggleCirculateur');
const range_consigne = document.querySelector('.consigne');
const range_temperature_bias = document.querySelector('.temperature_bias');

function read_temperature(update = true) {
    console.log("read temperature");
    const xhr = new XMLHttpRequest();
    xhr.open("GET", "/temperature");
    xhr.send();
    xhr.responseType = "json";
    xhr.onload = () => {
        //eko("received")
        if (xhr.readyState == 4 && xhr.status == 200) {
            response = xhr.response;
            /*
              temps.push(response.temperature);i aussi mais peut-être plus vers 18h30 je fais au mie
              if (temps.length > 24*60) { // mn in a day
              temps.shift();
              }
              let labels = [];
              // on oublie ca, le buffer est géré dans l'arduino

              eko()
              temps = response.valeurs;
            */
            tempDHT = response.DHT.temperature;
            hygroDHT = response.DHT.hygrometry;
            gaz = response.MQ2.gaz;

            setd = function(l, v) {
                document.getElementById(l).innerHTML = l + "=" + v;
            }

            /*
            //setd("temperature", "" + response.temperature + "°C");
            setd("temperatureDHT", "" + tempDHT + "°C");
            setd("hygrometrieDHT", hygroDHT);
            setd("gaz", gaz); 
            setd("consigne", response.consigne);
            setd("identity", response.identity);
            setd("relay_bruleur", response.relay_bruleur);
            setd("relay_circulateur", response.relay_circulateur);
            */

            const ttt = JSON.stringify(response, undefined, 2);
            //console.log(ttt);
            document.getElementById("texte").value = ttt;
            
            if (update) {
                button_bruleur.checked = response.relay_bruleur;
                button_circulateur.checked = response.relay_circulateur;
                consigne.value = response.consigne;
            }
            
            let now = new Date();

            //console.log(temps); 
            let interval = xhr.response.interval;
            /*
              for (i in temps) {
              let dd = new Date(now.getTime() - i * interval);
              let ss = dd.toLocaleDateString('fr', { weekday:"long", hour:"numeric", minute:"numeric"});
              labels.unshift(dd);
              }

              var trace1 = {
              x : labels, 
              y : temps,
              type: 'scatter'
              };
              var data = [trace1];
              Plotly.newPlot('plot_temperature', data);
            */
        }
    }
    //console.log("received");
    if (update) {
        setTimeout(read_temperature, 1000 * 2); // 1 mn
    }
}


function set_consigne(v) {
    const xhr = new XMLHttpRequest();
    console.log("v=", v);
    const iv = v ? 1 : 0;
    const uu = "/set_consigne?value=" + v;
    console.log(uu);
    xhr.open("GET", uu);
    xhr.send();
    xhr.responseType = "json";
    xhr.onload = () => {
        //eko("received")
        if (xhr.readyState == 4 && xhr.status == 200) {
            response = xhr.response;
            read_temperature(false);
        }
    }
}

function set_temperature_bias(v) {
    const xhr = new XMLHttpRequest();
    //console.log("v=", v);
    const iv = v ? 1 : 0;
    const uu = "/set_temperature_bias?value=" + v;
    //console.log(uu);
    xhr.open("GET", uu);
    xhr.send();
    xhr.responseType = "json";
    xhr.onload = () => {
        //eko("received")
        if (xhr.readyState == 4 && xhr.status == 200) {
            response = xhr.response;
            read_temperature(false);
        }
    }
}

function set_relay(v, r) {
    const xhr = new XMLHttpRequest();
    console.log("v=", v, "r=", r);
    const iv = v ? 1 : 0;
    const uu = "/set_relay?value=" + iv + "&relay=" + r;
    console.log(uu);
    xhr.open("GET", uu);
    xhr.send();
    xhr.responseType = "json";
    xhr.onload = () => {
        //eko("received")
        if (xhr.readyState == 4 && xhr.status == 200) {
            response = xhr.response;
            read_temperature();
        }
    }
}


button_bruleur.addEventListener('change', ()=> {
    console.log("change bruleur, now=", button_bruleur.checked);    
    set_relay(button_bruleur.checked, 0);
    //read_temperature();
})
button_circulateur.addEventListener('change', ()=> {
    set_relay(button_circulateur.checked, 1);
    console.log("change circulateur");
    //read_temperature();    
    
})

consigne.addEventListener('input', (e)=> {
    set_consigne(e.target.value);
    e.target.nextElementSibling.value = e.target.value; 
    console.log("change consigne");
    //read_temperature();    
    
})

temperature_bias.addEventListener('input', (e)=> {
    set_temperature_bias(e.target.value);
    e.target.nextElementSibling.value = e.target.value;     
    console.log("change temerature bias");
    //read_temperature();    
    
})


const d = new Date();
let time = d.getTime();

setTimeout(read_temperature, 1000*2);

