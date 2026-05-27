
count = "a";
const accueil = "Tapez le code";
const button = document.getElementById("ouvrir");
//const swap = document.getElementById("swap");
const ekodiv = document.getElementById("eko");
const buttons_div = document.getElementById("buttons");

//const url = "http://78.207.134.29:8083/main";
//const url = "http://78.207.134.29:8083/main";
//const url = "http://78.207.134.29:8083/main";
const url = "http://176.188.228.22:8080";
//const url = "http://192.168.1.95/main";

const mode = "deploy";
const testing = mode == "testing";

const ws = new WebSocket('ws://' + "176.188.228.22" + ':' + "8080" + '/ws')

console.log("ws", ws);
console.log("testing", testing);

function eko(...txt) {
    if (testing) {
        ekodiv.innerHTML += txt.toString() + '<br>';
        ws.send(txt.toString())
    }
    console.log(txt);
}

if (true) {
    ws.onopen = () => {
        console.log('ws opened on browser')
        ws.send('hello world')
    }
    
    ws.onmessage = (message) => {
        console.log(`message received ` + message.data)
        if (testing) {
            ekodiv.innerHTML += message.data + '<br>';
        }
    }
}

function statut() {
    murl = "statut_porte";
    //console.log("statut fetching");
    count = count + "a";
    //document.getElementById("statut").innerHTML = "fetching";
    //console.log("url 1", murl);
    fetch(murl).then(function(response) {
        //console.log('reponse statut 1 ', response);        
        d = response.json();
        //console.log('reponse statut ', d);
        //console.log(d["porte"]);
        //console.log("result 1", d);
        return d;
    }).then(function(data) {
        //console.log("then 1 ", data);
        //button.innerHTML = count;
        //console.log(data);
        //console.log('data statut ', data);
        var dsds = "La porte est " + data["porte_fermee"] + ", " + data["porte_ouverte"] + "<br>";
        //dsds +=  "buf_len : " + data["buf_len"] + "<p>";
        //dsds +=  "swapped : " + data["swapped"] + "<p>";
        document.getElementById("statut").innerHTML = dsds;
        setTimeout(statut, 1000);
    }).catch(function(ee) {
        console.log("statut statut failed");
        //document.getElementById("statut").innerHTML = ee;
        
    });

    //ws.send("ws message");
    
}
const myTimeout = setTimeout(statut, 1000);

cookies = document.cookies;
console.log('cookies ', cookies);
reset = function(){
    button.style.fontSize="100px";
    button.innerHTML = accueil;
    button.disabled = false;
};

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
console.log('code from cookie ', code);

function ouvre(cde) {
    eko("cde= ", cde);
    document.cookie = "eportiercode=" + cde + ";SameSite=Strict";
    cookies = document.cookies;
    //console.log("cookies ", cookies);
    cookies = document.cookie;
    //console.log("cookies2 ", cookies);
    button.style.fontSize="30px";
    button.innerHTML = "le verrou va s'ouvrir ...";
    setTimeout(() => {
        const murl = "main" + cde;        
        button.disabled = true;
        eko("murl ", murl)
        fetch(murl).then(function(result)  {
            d = result.json();
            eko("result ouvre ", d);
            //console.log("result ", d);
            return d;
        }).then(function(data) {
            //console.log("then ", data);
            if (data.status == "ok") {                                
                button.innerHTML = "Système contacté,<br>déverrouillage pendant 2 secondes ...";
            } else {
                button.innerHTML = "Système contacté .. bad luck!";
            }
            setTimeout(reset, 2000);
        }).catch((e) => {
            eko("err", e);
            let tt = "Pas moyen de contacter le système!.. fetching " + murl;
            button.innerHTML = tt;
            eko(tt);
            setTimeout(reset, 2000);
        })}, 
               1000);
}

function debug() {
    ouvre("96713");
    const delay = Math.floor(Math.random()*5000+1);       
    setTimeout(debug, delay);
}

//setTimeout(debug, 1000);

function swap_func(cde) {
    murl = "swap";
    fetch(murl).then(function(response) {
        d = response.json();
        return d;
    }).then(function(data) {
        document.getElementById("statut").innerHTML = "La porte est " + data["porte"];
        setTimeout(statut, 1000);
    }).catch(function(ee) {
        console.log("swap Booo");
        //document.getElementById("statut").innerHTML = ee;
        
    });    
}

button.addEventListener('click', function() { ouvre(code); });
//swap.addEventListener('click', function() { swap_func(); });
reset();
buttons = [];
clicked = [];
function addbutton(txt, x, y) {
    // Create a button element
    const nbutton = document.createElement('button');
    nbutton.innerText = txt;
    //console.log(nbutton);
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
    buttons_div.appendChild(nbutton);
};
W=30*4;
H=75*2;
ML=  100;
MH = 230;
//console.log("create buttons");
for (i = 1; i < 10; i++) {
    //console.log(i);
    addbutton(i, ((i-1) % 3) * W + ML, (~~((i-1) / 3) * H) + MH); 
}


ekodiv.style.position = "absolute";
ekodiv.style.top = H*5 + 'px';
ekodiv.style.fontSize = "20px";

