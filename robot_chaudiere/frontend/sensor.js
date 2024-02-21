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
            tempDHT = response.DHT.temperature;
            hygroDHT = response.DHT.hygrometry;
            gaz = response.MQ2.gaz;
	    millis = response.millis;
            tempext = response.tempext;
            tempchaudiere = response.tempchaudiere.value;
            setd = function(l, v) {
		document.getElementById(l).innerHTML = l + "=" + v;
            }
            console.log("tempext", tempext)
            //setd("temperature", "" + response.temperature + "°C");
            setd("temperatureDHT", "" + tempDHT + "°C");
            setd("hygrometrieDHT", hygroDHT);
            setd("gazMQ2", gaz);
            setd("tempext", tempext);
            setd("tempchaudiere", tempchaudiere);
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
	    eko(li)
	    
	    let interval = response.interval;
	    let temps = [];	
	    let begin = d.getTime() - li * interval*1000;
	    let hygro = [];
	    let gaz = []
            let tempext = []
            let tempchaudiere = []
	    for (i in buf) {
		let dd = new Date(begin + i * interval*1000);
		let ss = dd.toLocaleDateString('fr', { weekday:"long", hour:"numeric", minute:"numeric"});
		labels.unshift(dd);
		temps.unshift(buf[i].DHT.temperature);
		hygro.unshift(buf[i].DHT.hygrometry);
		gaz.unshift(buf[i].MQ2.gaz);
		tempext.unshift(buf[i].tempext);
		tempchaudiere.unshift(buf[i].tempchaudiere);
	    }
	    eko("refresh plot");
	    const trace_temp = {  name: 'temperature', x : labels,  y : temps,  type: 'scatter', 'line': {'shape': 'spline'}};
	    const trace_hygro = {  name : 'hygrometry', x : labels,  y : hygro,  type: 'scatter' , 'line': {'shape': 'spline', 'smoothing': 1.3}};
	    const trace_gaz = {  name : 'gaz', x : labels,  y : gaz,  type: 'scatter', 'line': {'shape': 'spline', 'smoothing': 5.3} };
	    const trace_tempext = {  name : 'tempext', x : labels,  y : tempext,  type: 'scatter', 'line': {'shape': 'spline', 'smoothing': 5.3} };            
	    const trace_tempchaudiere = {  name : 'tempchaudiere', x : labels,  y : tempchaudiere,  type: 'scatter', 'line': {'shape': 'spline', 'smoothing': 5.3} };            
	    var data = []
	    eko(gid("temp").value);
	    eko(gid("hygro").value);
	    eko(gid("gaz").value);

	    if (gid("temp").value == "ON") { data.push(trace_temp); }
	    if (gid("hygro").value == "ON") { data.push(trace_hygro); }
	    if (gid("gaz").value == "ON") { eko("gaz"); data.push(trace_gaz); }
	    if (gid("tempextb").value == "ON") { eko("tempext"); data.push(trace_tempext); }
	    if (gid("tempchaudiereb").value == "ON") { eko("tempchaudiere"); data.push(trace_tempchaudiere); }
	    Plotly.newPlot('plot_temperature', data);
	    eko("plotted");
	}
	eko("processed");
    }
    setTimeout(doplot, 1000 * 60); // 1 mn
}
setTimeout(doplot, 1000 * 2); // 1 mn
eko("ok");	
