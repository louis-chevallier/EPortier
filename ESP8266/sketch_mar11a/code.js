count = "a";
const accueil = "Tapez le code";
const button = document.getElementById("ouvrir");

const ws = new WebSocket('ws://192.168.1.177:80/ws')

ws.onopen = () => {
    console.log('ws opened on browser')
}


ws.onmessage = (message) => {
    console.log(`message received ` + message.data)
}


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
