function eko(x) {
    console.log(x)
}

var temps= [33, 44];
var trace1 = {
    y: temps,
    type: 'scatter'
}
var data = [trace1];

eko("start xxx");

function read_temperature() {
    console.log("read temperature");
    const xhr = new XMLHttpRequest();
    //xhr.open("GET", "http://192.168.1.33/temperature");
    xhr.open("GET", "./data");
    xhr.send();
    xhr.responseType = "json";
    xhr.onload = () => {
        eko("received")
        if (xhr.readyState == 4 && xhr.status == 200) {
            response = xhr.response;
            //eko()
            console.log("response", response)
            tempds18 = response.DS18B20_salon.value;
            tempDHT = response.DHT.temperature;
            hygroDHT = response.DHT.hygrometry;
            gaz = response.MQ2.gaz;
	    millis = response.millis;
            tempext = response.tempext;
            tempchaudiere = response.tempchaudiere.value;
            setd = function(l, v, lab) {
                if (lab == undefined) {
                    lab = l;
                }
		document.getElementById(l).innerHTML = lab + "=" + v;
            }
            console.log("tempext", tempext)
            //setd("temperature", "" + response.temperature + "°C");
            setd("temperatureDS18", "" + tempds18 + "°C", "Température intérieure");
            setd("temperatureDHT", "" + tempDHT + "°C", "Température intérieure");
            setd("hygrometrieDHT", hygroDHT, "Hygrométrie");
            setd("gazMQ2", gaz, "Niveau Gaz ambiant");
            setd("tempext", tempext, "Température extérieure");
            setd("tempchaudiere", tempchaudiere, "Température chaudière");
            setd("millis", millis);
	    
	}
	setTimeout(read_temperature, 1000 * 60); // 1 mn	
    }
}

function gid(i) {
    return document.getElementById(i);
}

function toggle_select(select)
{
    o = select;
    eko(o.id);
    eko(o.value);
    doplot();
}

function toggle(button)
{
    o = button;
    eko(o.id);
    eko(o.value);
    o.value = (o.value =="OFF") ? "ON" : "OFF";
    setTimeout(doplot, 1000 * 0.5); // 0.5 sec
    eko();
}

function send(button)
{
    o = button;
    eko(o.id);
    eko(o.value);
    eko();
}

//console.log("received");
setTimeout(read_temperature, 1000 * 1); // 1 mn

function doplot() {
    const xhr1 = new XMLHttpRequest();

    hh = gid("hist").value;
    eko(["hh", hh]);
    
    xhr1.open("GET", "read?s=" + hh);
    xhr1.send();
    xhr1.responseType = "json";
    
    //setTimeout(read_temperature, 1000);
    
    eko("get temps..");
    const d = new Date();
    let time = d.getTime();
    xhr1.onload = () => {
	eko("temps received")
	if (xhr1.readyState == 4 && xhr1.status == 200) {
	    response = xhr1.response;
	    buf = response.buffer;
	    let labels = [];
	    let li = buf.length;
	    
	    let interval = response.interval;
	    let temps = [];	
	    let temps2 = [];	
	    let begin = d.getTime() - li * interval*1000;
	    let hygro = [];
	    let gaz = []
            let tempext = []
            let tempchaudiere = []


            
            l = [ ['temperature', temps, (ee) => ee.DHT.temperature, 'temp'],
                  ["temperature2", temps2, (ee) => "DS18B20_salon" in ee ? ee.DS18B20_salon.value: -1 , 'tempds18'],
                  ["hygrometry", hygro, (ee) => ee.DHT.hygrometry, 'hygro'],
                  ["gaz", gaz, (ee) => ee.MQ2.gaz, 'gaz'],
                  ["tempext", tempext, (ee) => ee.tempext, 'tempextb'],
                  ["tempchaudiere", tempchaudiere, (ee) => ee.tempchaudiere.value, 'tempchaudiereb']
                ]

            function ff(e, i) {
	        let labels2 = [];
                let yv = [];
                for (i in buf) {
		    let dd = new Date(begin + i * interval*1000);
		    let ss = dd.toLocaleDateString('fr', { weekday:"long", hour:"numeric", minute:"numeric"});
                    let v = e[2](buf[i]);
                    /*
                    if (e[0] == "temperature2") {
                        console.log('buf', buf[i]);
                        console.log('v' , v);
                    }
                    */
                    if (v > 0) {
                        labels2.unshift(dd);
                        yv.unshift(v);
                    }
                }
	        const trace = {  name: e[0], x : labels2,  y : yv,  type: 'scatter', 'line': {'shape': 'spline'}};
                return trace;
            }
            const traces2 = l.filter( (e) => gid(e[3]).value == "ON");
            const traces = traces2.map(ff);
            data = traces;
            
	    Plotly.newPlot('plot_temperature', data);
	    eko("plotted");
	}
	eko("processed");
    }
    //setTimeout(doplot, 1000 * 60); // 1 mn
}
setTimeout(doplot, 1000 * 1); // 1 mn
eko("ok");	
