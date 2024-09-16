count = "a";
const accueil = "Tapez le code";
const button = document.getElementById("ouvrir");
const statut = document.getElementById("statut");
const swap_button = document.getElementById("swap");

/*
const ws = new WebSocket('ws://IPADDRESS:PORT/ws')

ws.onopen = () => {
    console.log('ws opened on browser')
}


ws.onmessage = (message) => {
    console.log(`message received ` + message.data)
}
*/


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
const root_url = "ROOT_URL";
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

function debug() {
    console.log('url=', url);
    const xhr = new XMLHttpRequest();
    xhr.open("GET", "./main");
    xhr.send();
    xhr.responseType = "json";
    xhr.onload = () => {
        if (xhr.readyState == 4 && xhr.status == 200) {
            try {
                response = xhr.response;
                console.log('status', response.status);
                console.log('swapped', response.swapped);
                console.log('buf_serial', response.buf_serial);
                var ss = "swapped : " + response.swapped + "\n";
                ss += "buf_serial  : " + response.buf_serial + "\n";
                statut.innerHTML =  ss;
            } catch (error) {
                console.log("error ", error);
            }
            const delay_msec = Math.floor((Math.random() * 4) * 1000);
            console.log(delay_msec);
            setTimeout(debug, delay_msec);
        }
    }
}

function swap_func() {
    console.log('url=', root_url);
    const xhr = new XMLHttpRequest();
    xhr.open("GET", "./swap");
    xhr.send();
    xhr.responseType = "json";
    xhr.onload = () => {
        if (xhr.readyState == 4 && xhr.status == 200) {
            response = xhr.response;
            console.log('status', response.status);
        }
    }
}


button.addEventListener('click', function() { debug();  });
swap_button.addEventListener('click', function() { swap_func();  });

reset();
buttons = [];
clicked = [];
W=30*4;
H=75*2;
ML=  100;
MH = 230;

debug();
