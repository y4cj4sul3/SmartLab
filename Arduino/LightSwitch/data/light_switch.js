class LightSwitchModel {
	constructor(id) {
		this.id = id;
		// create from template
		const btnTemplate = document.querySelector("#lightSwitchTemplate").content;
		let node = btnTemplate.cloneNode(true);
		this.node = node;
		let div = node.querySelector(".lightSwitch");
		this.div = div;
		div.querySelector(".btnLabel").innerHTML = "Light Switch " + (id + 1);
		this.btn = div.querySelector(".lightSwitchBtn");
		this.btn.id = id;
		this.btn.onclick = () => {
			this.toggleLightSwitch();
		};
		this.btn.innerHTML = "ON";

		// add to lightSwitches div
		document.getElementById("lightSwitches").appendChild(node);

		// light switch state (true -> ON, false -> OFF)
		this.switchState = false;
	}
	static xmlHTTP = new XMLHttpRequest();
	static sendCmd(id, state) {
		var queryCmd = "/switch?id=" + id + "&state=" + state;
		console.log(queryCmd);
		this.xmlHTTP.open("GET", queryCmd, true);
		this.xmlHTTP.send(null);
	}

	toggleLightSwitch() {
		// document.getElementById("")
		this.switchState = !this.switchState;

		if (this.switchState) {
			this.btn.innerHTML = "OFF";
		} else {
			this.btn.innerHTML = "ON";
		}

		LightSwitchModel.sendCmd(this.id, this.switchState);
	}
}
LightSwitchModel.xmlHTTP.onreadystatechange = function () {
	if (LightSwitchModel.xmlHTTP.readyState == 4 && LightSwitchModel.xmlHTTP.status == 200) {
		var response = LightSwitchModel.xmlHTTP.responseText;
		console.log(response);
	}
};

lightSwitches = [];
for (var i = 0; i < 3; ++i) {
	let lightSwitch = new LightSwitchModel(i);
	lightSwitches.push(lightSwitch);
}
