var head = 0, tail = 0, ring = new Array();
var myJSON;
var redval = 0;
var irval = 0;
var eventListen = null;

function getID(_str) {
    return document.getElementById(_str);
}
function getIr() {
    return irval;
}  

function getRed() {
    return redval;
}

function get_appropriate_ws_url(extra_url)
{
    var pcol;
    var u = document.URL;

    /*
     * We open the websocket encrypted if this page came on an
     * https:// url itself, otherwise unencrypted
     */

    // wss is not supported yet by arduino websocket
    // if (u.substring(0, 5) === "https") {
    //     pcol = "wss://";
    //     u = u.substr(8);
    // } else {
    //     pcol = "ws://";
    //     if (u.substring(0, 4) === "http")
    //         u = u.substr(7);
    // }

    pcol = "ws://";
    u = u.split("/");
    var p = u[0].split(":");

    /* + "/xxx" bit is for IE10 workaround */

    return pcol + p[0] + ":81" + "/" + extra_url;
}

function isJSONHRValid(_str) {
    myJSON = null;
    try {
        myJSON = JSON.parse(_str.trim());
    } catch (e) {
        console.log("failed to parse json: " + e);
        return false;
    }

    // if(myJSON.value == undefined) return false;
    // if(myJSON.latitude == undefined) return false;
    // if(myJSON.longitude == undefined) return false;

    return true;
}

function got_packet(msgdata) {
    var n, s = "";
    
    var msgjson = msgdata.split("}")
    for (i =0; i < msgjson.length-1; i++) {
        ring[head] = msgjson[i] + "\n";
        head = (head + 1) % 50;
        if (tail === head)
            tail = (tail + 1) % 50;

        n = tail;
        do {
            s = s + ring[n];
            n = (n + 1) % 50;
        } while (n !== head);
        getID("r").value = s; 
        getID("r").scrollTop =
        getID("r").scrollHeight;

        if (isJSONHRValid(msgjson[i] + "}")) {
            if(myJSON.latitude != undefined) {
                getID("latitude").value = myJSON.latitude
            }
            if(myJSON.longitude != undefined) {
                getID("longitude").value = myJSON.longitude
            }
            if(myJSON.ip != undefined) {
                getID("ip").value = myJSON.ip
            }
            if(myJSON.qw != undefined) {
                getID("qw").value = myJSON.qw
            }
            if(myJSON.qx != undefined) {
                getID("qx").value = myJSON.qx
            }
            if(myJSON.qy != undefined) {
                getID("qy").value = myJSON.qy
            }
            if(myJSON.qz != undefined) {
                getID("qz").value = myJSON.qz
            }
            if(myJSON.r != undefined) {
                getID("roll").value = myJSON.r
            }
            if(myJSON.p != undefined) {
                getID("pitch").value = myJSON.p
            }
            if(myJSON.y != undefined) {
                getID("yaw").value = myJSON.y
            }
            if(myJSON.ax != undefined) {
                getID("ax").value = myJSON.ax
            }
            if(myJSON.ay != undefined) {
                getID("ay").value = myJSON.ay
            }
            if(myJSON.az != undefined) {
                getID("az").value = myJSON.az
            }
            if(myJSON.h != undefined) {
                getID("heading").value = myJSON.h
            }
        }
    }

}

function SSEClose() {
    if (eventListen != null)
        eventListen.close();
}

function SSEListenEvents() {
    console.log("Listening");
    eventListen = new EventSource(getID("windowUrl").value + "rest/events/" + getID("SSEChannel").value);
    eventListen.addEventListener("event", function(event) {
        got_packet(event.data);
    }, false);
}

function SSESubscribeCallback(responseText) {
    eventListen = new EventSource(responseText);
    eventListen.addEventListener("event", function(event) {
        got_packet(event.data);
    }, false);
}

function SSESubscribe() {
    console.log("Subscribing");
    httpGetAsync(getID("windowUrl").value + "rest/events/subscribe", SSESubscribeCallback);
}

function httpGetAsync(theUrl, callback)
{
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function() { 
        if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
            callback(xmlHttp.responseText);
    }
    xmlHttp.open("GET", theUrl, true); // true for asynchronous 
    xmlHttp.send(null);
}

function clearLog() {
    getID("r").value = "";
    tail = head;
}

document.addEventListener("DOMContentLoaded", function() {
    getID("windowUrl").value = document.URL;
    httpGetAsync(getID("windowUrl").value + "rest/events/subscribe", SSESubscribeCallback);
    webgl_start();
}, true);