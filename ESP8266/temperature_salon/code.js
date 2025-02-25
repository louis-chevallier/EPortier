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
  
  function read_temperature() {
    console.log("read temperature");
    const xhr = new XMLHttpRequest();
    xhr.open("GET", "/temperature");
    xhr.send();
    xhr.responseType = "json";
    xhr.onload = () => {
      eko("received")
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

        //setd("temperature", "" + response.temperature + "°C");
        setd("temperatureDHT", "" + tempDHT + "°C");
        setd("hygrometrieDHT", hygroDHT);
        setd("gaz", gaz); 
        setd("consigne", response.consigne);
        setd("relay", response.relay);
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
    setTimeout(read_temperature, 1000 * 60); // 1 mn
  }
  setTimeout(read_temperature, 1000);

  const d = new Date();
  let time = d.getTime();

