window.onload = function(){
	buttonOn = document.getElementById('submitOn');
	buttonOn.onClick = relayOn;
	
	buttonOff = document.getElementById('submitOff');
	buttonOff.onClick = relayOff;
}

function relayOn(){
	
	hidden = document.getElementById("total");
	hidden.value = "1";
	
	form = document.getElementById("relay");
	form.method = "GET";
	form.action = "relay.php";
	form.submit();
}

function relayOff(){
	
	hidden = document.getElementById("total");
	hidden.value = "0";
	
	form = document.getElementById("relay");
	form.method = "GET";
	form.action = "relay.php";
	form.submit();
}
