
var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);

connection.onopen = function () {
  connection.send('Connection ' + new Date());
};

connection.onerror = function (error) {
  console.log('WebSocket Error ', error);
};

connection.onmessage = function (e) {
  console.log('Server: ', e.data);
  var values = e.data.split("_");
  for (f=1; f<=values.length; f++) {
  	document.getElementById('sl_'+f).value = values[f-1];
	}
};

connection.onclose = function () {
  console.log('WebSocket connection closed');
};

function sendSliderData(element_id) {
	connection.send('#' + element_id + '_' + document.getElementById('sl_'+element_id).value.toString());
}

function sendButtonData(data) {
	connection.send(data);
}