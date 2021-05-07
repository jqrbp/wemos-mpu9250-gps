var head = 0, tail = 0, ring = new Array();
var myJSON;
var redval = 0;
var irval = 0;
var eventListen = null;
var calibmag = false;

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

function chkMyJSON(_val) {
	if(myJSON == null) return false;
	if(myJSON == undefined) return false;
	if(myJSON[_val] == null) return false;
	if(myJSON[_val] == undefined) return false;
	return true;
}

function loadMyJson(_str) {
	myJSON = null;
	try {
		myJSON = JSON.parse(_str);
	} catch (e) {
		return false;
	}

	return true;
}

function setMyJson(_val) {
	if(chkMyJSON(_val)) {
		getID(_val).value = myJSON[_val];
		return true;
	}
	return false;
}

function setMyJsonRadToDeg(_val) {
	if(chkMyJSON(_val)) {
		getID(_val).value = (myJSON[_val]*180/Math.PI).toFixed(4);
		return true;
	}
	return false;
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
        getID("lr").value = s; 
        getID("lr").scrollTop =
        getID("lr").scrollHeight;

        if (loadMyJson(msgjson[i] + "}")) {
            setMyJson("ip");

            //object quaternion
			setMyJson("qw"); setMyJson("qx"); setMyJson("qy"); setMyJson("qz");
            setMyJsonRadToDeg("r"); setMyJsonRadToDeg("p"); setMyJsonRadToDeg("y");
            setMyJson("ax"); setMyJson("ay"); setMyJson("az");
            setMyJson("latitude"); setMyJson("longitude");

            if(chkMyJSON("h")) {
                getID("heading").value = (myJSON.h*180/Math.PI).toFixed(2);
                document.getElementById("compass-img").style.transform = 'rotate(' + ((myJSON.h*180/Math.PI) - 360 ) + 'deg)';
            }

            //magnetometer values
            setMyJson("mx"); setMyJson("my"); setMyJson("mz"); setMyJsonRadToDeg("mh");

            //magnetometer calibration
            setMyJson("mbx"); setMyJson("mby"); setMyJson("mbz"); 
            setMyJson("msx"); setMyJson("msy"); setMyJson("msz");

            if(chkMyJSON("mcx")) {
                if(chkMyJSON("mci")) {
                    addpoints(myJSON.mx/3, myJSON.my/3, myJSON.mz/3, myJSON.mcx);
                    document.getElementById("stat").value = "calibno:" + myJSON.mci + "/" + myJSON.mcx;
                    if(myJSON.mci >= myJSON.mcx - 2) calibmag = false;
                }
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
    getID("lr").value = "";
    tail = head;
}

function MagCalibrateReqCallback(responseText) {
    console.log(responseText);
}

function ReadMagParam() {
    httpGetAsync(getID("windowUrl").value + "get/magcalib", MagCalibrateReqCallback);
}

function MagCalibrate() {
    calibmag = true;
    httpGetAsync(getID("windowUrl").value + "calib", MagCalibrateReqCallback);
}

function SetNorth() {
    httpGetAsync(getID("windowUrl").value + "setNorth", MagCalibrateReqCallback);
}

function HideInfo() {
    getID("tableInfo").hidden = true;
}

function ShowInfo() {
    getID("tableInfo").hidden = false;
}

document.addEventListener("DOMContentLoaded", function() {
    getID("windowUrl").value = document.URL;
    httpGetAsync(getID("windowUrl").value + "rest/events/subscribe", SSESubscribeCallback);
    HideInfo();
    webgl_start();
}, true);