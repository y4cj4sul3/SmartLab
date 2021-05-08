// DOM components
var nkDiv = document.getElementById("nkDiv");
var nkStatus = document.getElementById("nkStatus");
var nkSSID = document.getElementById("nkSSID");
var nkPasswd = document.getElementById("nkPasswd");
var nkScanbtn = document.getElementById("nkScanbtn");
var nkConnectBtn = document.getElementById("nkConnectBtn");
var nkResetDiv = document.getElementById("nkResetDiv");

function showNetworkStatus(text, color) {
	nkStatus.innerHTML = text;
	nkStatus.style.color = color;
}

function showNetworkView(show = true) {
	if (show) {
		// show div and hide reset div
		nkDiv.style.display = "block";
		nkResetDiv.style.display = "none";
	} else {
		nkDiv.style.display = "none";
		nkResetDiv.style.display = "block";
	}
}

function enableConnectBtn(active) {
	if (active) {
		nkConnectBtn.disabled = false;
		nkConnectBtn.innerHTML = "Connect";
	} else {
		nkConnectBtn.disabled = true;
		nkConnectBtn.innerHTML = "Connecting...";
	}
}

function connectToOtherNetwork() {
	// show message
	showNetworkStatus("Scanning Networks...", "gray");
	// disconnect from current network and scan for networks
	scanNetwork();
	// show connection UI
	showNetworkView();
}

// Scan for networks
var xmlHTTPsn = new XMLHttpRequest();
xmlHTTPsn.onreadystatechange = function () {
	if (xmlHTTPsn.readyState == 4 && xmlHTTPsn.status == 200) {
		var response = xmlHTTPsn.responseText;
		console.log(response);
		// clear previous scan result
		nkSSID.options.length = 0;
		// parse response
		var option;
		var preNL = response.indexOf("\n") + 1;
		var nextNL = response.indexOf("\n", preNL);
		while (nextNL != -1) {
			option = document.createElement("option");
			option.text = response.substring(preNL, nextNL);
			nkSSID.add(option);
			// get next SSID
			preNL = nextNL + 1;
			nextNL = response.indexOf("\n", preNL);
		}
		// no network
		if (nkSSID.options.length == 0) {
			// show message
			showNetworkStatus("No network found.", "orange");
		} else {
			showNetworkStatus("", "gray");
			// re-enable connect button
			enableConnectBtn(true);
		}
		// re-rnable scan button
		nkScanbtn.disabled = false;
		nkScanbtn.innerHTML = "Scan Networks";
	}
};
function scanNetwork() {
	// disable scan button
	nkScanbtn.disabled = true;
	nkScanbtn.innerHTML = "Scanning...";
	// disabled connect button
	nkConnectBtn.disabled = true;

	// send request
	xmlHTTPsn.open("GET", "/scan_network", true);
	xmlHTTPsn.send(null);
}

// Check for network status
var firstTimeChecking = true;
var xmlHTTPnk = new XMLHttpRequest();
xmlHTTPnk.onreadystatechange = function () {
	if (xmlHTTPnk.readyState == 4 && xmlHTTPnk.status == 200) {
		var response = xmlHTTPnk.responseText;
		console.log(response);

		if (response.charAt(0) == "C") {
			// hide div
			showNetworkView(false);
			// show message
			var content = response.substring(3).split(",");
			showNetworkStatus("&#10004; Connect to network: " + content[0] + " (IP: " + content[1] + ")", "green");
		} else {
			if (firstTimeChecking) {
				// show message
				showNetworkStatus("Scanning Networks...", "gray");
				firstTimeChecking = false;
				// Scan for networks
				scanNetwork();
			} else {
				if (response.charAt(0) == "F") {
					// Connection Failed
					showNetworkStatus("&#10008; Connection Failed. Please try again.", "red");
					// re-enable submit button
					enableConnectBtn(true);
					// Scan for networks
					scanNetwork();
				} else {
					// Connecting
					// keep check for network status
					console.log("keep checking");
					setTimeout(checkNetworkStatus, 1000);
				}
			}
		}
	}
};
function checkNetworkStatus() {
	xmlHTTPnk.open("GET", "/network", true);
	xmlHTTPnk.send(null);
}
checkNetworkStatus();

// Connect to network
var xmlHTTPcn = new XMLHttpRequest();
xmlHTTPcn.onreadystatechange = function () {
	if (xmlHTTPcn.readyState == 4 && xmlHTTPcn.status == 200) {
		var response = xmlHTTPcn.responseText;
		console.log(response);
		if (response === "connecting") {
			// check network status
			checkNetworkStatus();
		} else {
			// Failed cause button receive empty body
			showNetworkStatus("$#10008; Failed to connect to the SmartLab. Please try again", "red");
			// re-enable submit button
			enableConnectBtn(true);
		}
	}
};
nkConnectBtn.onclick = function () {
	// block blank request
	if (nkSSID.value === "" || nkPasswd.value === "") {
		// show failed message
		nkStatus.innerHTML = "&#10008; SSID and password cannot be blank.";
		return;
	}
	var ssid = nkSSID.value;
	var passwd = nkPasswd.value;

	// disable submit button
	enableConnectBtn(false);

	// send request
	var request = "ssid=" + ssid + "&passwd=" + passwd;
	console.log(request);
	// xmlHTTPcn.timeout = 60000;
	xmlHTTPcn.open("POST", "/network", true);
	xmlHTTPcn.send(request);
};
