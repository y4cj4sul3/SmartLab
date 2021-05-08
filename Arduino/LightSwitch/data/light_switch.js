// Light Switch
var lightSwitchBtn = document.getElementById("lightSwitchBtn");
var lightState = true;

// Servo
var xmlHTTPmotor = new XMLHttpRequest();
xmlHTTPmotor.onreadystatechange = function () {
	if (xmlHTTPmotor.readyState == 4 && xmlHTTPmotor.status == 200) {
		var response = xmlHTTPmotor.responseText;
		console.log(response);
	}
};
function toggleLightSwitch(lightId) {
	var queryCmd = "/switch?dir=" + lightId;
	console.log(queryCmd);
	xmlHTTPmotor.open("GET", queryCmd, true);
	xmlHTTPmotor.send(null);
}
