if(typeof module != "undefined" && module.exports){
  var MessageWriter = require(__root+"/core/abstract/MessageWriter.js");
}

/**
  Provides a websocket connection to a host. Is the client implementation of
  {@linkcode https://github.com/einaros/ws}

  @memberof ClientSide
  @constructor
  @augments MessageWriter
  @param {string} host - the domain that we will be speaking to
  @param {string} [port=80] - the port to connect to
  @param {string} [path=""] - the path that will be appended to all namespaces
*/
function Server(host,port,path){
  var that = this;
  path = (path)?path:false;
  port = (port)?port:80;
  MessageDuplex.call(this,function(message){
    if(path)
      message.name = path + message.name;
    console.log(message);
    that.socket.send(JSON.stringify(message));
  });
  // method calls that are sent and waiting an answer
  try {
    this.host = "ws://"+host+":"+port;
    this.socket = new WebSocket(this.host);
    this.socket.onopen = function(){
      that.ready();
    }
    this.socket.onmessage = function(message){
      try{
        message = JSON.parse(message.data);
      }catch(e){
        that.socket.close();
      }
      console.log(message);
      that.handleMessage(message,this.socket);
    }
    this.socket.onclose = function(){
      console.log('Socket Status: ' + that.socket.readyState + ' (Closed)');
      that.stop();
    };
  } catch (exception) {
    console.log('Error' + exception);
  }
}

Server.prototype = Object.create(MessageDuplex.prototype);
Server.prototype.constructor = Server;

/**
  Provides the server that the current application was originally created by
  @var {Server} DocumentHost
  @memberof ClientSide
*/
/**
  Provides a direct communication to the forked process that the serverside runs on
  @var {Server} ApplicationFork
  @memberof ClientSide
*/
if(typeof module != "undefined" && module.exports){
  module.exports = Server;
}else{
  window.DocumentHost = null;
  (function(url){
    url = new URL(url);
    var port = (typeof wp != "undefined")?wp:(document.cookie.pwp)?document.cookie.pwp:parseInt(url.port)+200;
    window.DocumentHost = new Server(url.hostname,port);
  })(document.URL)
}
