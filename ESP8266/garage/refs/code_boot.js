


function load_code() {
    var script = document.createElement('script');
    script.onload = function () {
        console.log('loading js');
    };
    script.src = 'code.js';
    document.head.appendChild(script); //or something of the likes
}

setTimeout(load_code, 2000);
