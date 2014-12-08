global.__root = __dirname;
var httpport = process.env.httpport || 3000;
var websocketport = process.env.websocketport || 3200;

require('es6-promise').polyfill();

var WebSocketServer = require('ws').Server;
var express = require('express')
var app = express();

app.get(/^\/?$/,function(req,res,next){
  res.sendFile(__dirname+"/public/index.html");
})
app.use(express.static(__dirname+"/public/"));
app.get(/^\/bc\/.*/,require(__root+"/serverutils/bower_static.js"));
app.get("/api.js",require(__root+"/serverutils/client_api.js"));

var httpserver = app.listen(httpport, function () {
  var add = httpserver.address();
  console.log('HTTP Server running at http://%s:%s', add.address, add.port)
});

var wss = new WebSocketServer({
  port: websocketport
});
console.log("web socket is at: " + wss.options.host + ":" + wss.options.port);

var ClientSocket = require(__root+"/wrappers/window/Server2Window_com.js");

wss.on('connection', function (ws) {
  require(__root+"/serverutils/SocketRelay.js")(new ClientSocket(ws));
});
