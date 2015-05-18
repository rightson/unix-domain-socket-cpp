var net = require('net');

var socket_path = './.socket';
var client = net.createConnection(socket_path);

client.on("connect", function() {
    console.log('connecting to ' + socket_path);
});

client.on("data", function(data) {
    console.log('got data [%s]', data)
});

client.write(process.argv[2] || 'hi');
