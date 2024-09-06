const button = document.getElementById("data");
const value = document.getElementById("value");
const slider = document.getElementById("range");
const step = document.getElementById("step");

count = "a";

const ws = new WebSocket('ws://IPADDRESS:PORT/ws')

ws.onopen = () => {
    console.log('ws opened on browser')
}


ws.onmessage = (message) => {
    //console.log(`message received ` + message.data)
    let obj = JSON.parse(message.data);
    value.innerHTML = obj.value;
    range.value = obj.value;
}


function statut() {
}
const myTimeout = setTimeout(statut, 1000);

cookies = document.cookies;
console.log(cookies);
const url = "WURL";

function eko(...args) {
    args.forEach(arg => console.log(arg))
}



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

code =getCookie("signal");
console.log('cookie');

buttons = [];
clicked = [];

function doplot() {
    eko("doplot");
    
    const xhr1 = new XMLHttpRequest();
    xhr1.open("GET", "data?step=" + step.value);
    xhr1.send();
    eko("sent");
    xhr1.responseType = "json";
    const d = new Date();
    let time = d.getTime();
    xhr1.onload = () => {
	eko("temps received")
	if (xhr1.readyState == 4 && xhr1.status == 200) {
	    response = xhr1.response;
            //eko('response', response);
            //eko('response x', response.x);
            var trace1 = {
                x: response.x,
                y: response.y,
                type: 'scatter'
            };
            var data = [trace1];
	    Plotly.newPlot('plot', data);
	    eko("plotted");
	}
	eko("processed");
    }
    //setTimeout(doplot, 1000 * 60); // 1 mn
}

button.addEventListener('click', function() { doplot(); });



