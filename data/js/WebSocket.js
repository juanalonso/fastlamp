
var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);

connection.onopen = function () {
  connection.send('Connection ' + new Date());
};

connection.onerror = function (error) {
  console.log('WebSocket Error ', error);
};

connection.onmessage = function (e) {
  console.log('Server: ', e.data);
};

connection.onclose = function () {
  console.log('WebSocket connection closed');
};

function sendSliderData(element_id) {
	connection.send('#' + element_id + '_' + document.getElementById('sl_'+element_id).value.toString());
}