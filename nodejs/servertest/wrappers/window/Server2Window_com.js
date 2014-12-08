var MessageDuplex = require(__root+"/wrappers/abstract/MessageDuplex");

function ClientSocket(socket){
  var that = this;
  this.id = Date.now()+"|"+Math.random();
  this.socket = socket;
  MessageDuplex.call(this,function(message){
    delete message.user;
    that.socket.send(JSON.stringify(message));
  });
  that.ready();
  // method calls that are sent and waiting an answer
  this.socket.on("message",function(message){
    try{
      message = JSON.parse(message);
    }catch(e){
      that.socket.close();
    }
    that.handleMessage(message,that);
  });
  this.socket.on("close",function(){
    console.log('Socket Status: ' + that.socket.readyState + ' (Closed)');
    that.stop();

  });
}

ClientSocket.prototype = Object.create(MessageDuplex.prototype);
ClientSocket.prototype.constructor = ClientSocket;

module.exports = ClientSocket;
