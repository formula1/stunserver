if(typeof module != "undefined" && module.exports){
  var Server = require(__root+"/window/public/Window2Server_com");
  var NetworkInstance = require(__dirname+"/NetworkUser.js");
}
/**
  Creates a new Network host. A network host provides a manner to find users with
  a compatible app as yours. The manner in which they find other users is meaningless.
  However, it provides a way to make an offer to another user and to accept an offer.
  @constructor
  @memberof ClientSide
  @augments EventEmitter
  @param {string} url - the url that hosts the users you wish to find
  @param {object} info - the info you wish to give inorder for other users to find you
  @param {object} config - configuration, mostly just specifying ice servers
  @param {object} sconfig - data configuration, in case you are interested in that
*/
function NetworkHost(url,info,config,sconfig){
  EventEmitter.call(this);
  var defaulturl = new URL(document.URL);
  this.on = this.addListener.bind(this);
  this.off = this.removeListener.bind(this);
  if(!config){
    config = {'iceServers': [
			{url: "stun:"+defaulturl.hostname+":3478"},
		]};
    console.log("Stun server should be same as document url");
  }
	if(!sconfig){
		sconfig = {reliable: false};
	}
  this.config = config;
  this.sconfig = sconfig;
  if(url && DocumentHost.url != url){
    this.RTCHost = new Server(defaulturl.hostname,defaulturl.port||80);
  }else{
    this.RTCHost = DocumentHost;
  }
  this.connections = {};
  if(info){
    this.connect(info);
  }
  return this;
}
NetworkHost.prototype = Object.create(EventEmitter.prototype);
NetworkHost.prototype.constructor = NetworkHost;


/**
  if info was not provided, this is a manner to connect.
  @memberof NetworkHost
  @param {object} info - info that the server and other users may will see
*/
NetworkHost.prototype.connect = function(info){
  if(!info) throw new Error("Network Server may not be able to handle no information")
  this.info = info;
  this.id;
  var that = this;

  this.RTCHost.add("self",function(data){
    that.id = data;
  });
  this.RTCHost.add("list",function(data){
    return that.emit("userlist",data);
  });
  this.RTCHost.add("offer",function(data,user){
    console.log("offer");
    that.emit("offer", data);
  });
  this.RTCHost.add("accept",function(data,user){
    if(!that.connections[data.identity])
      throw Error("accepting a gift ungiven");
    that.connections[data.identity].ok(data);
    return that.emit("handshake",that.connections[data.identity]);
  });
  this.RTCHost.add("ice",function(data,user){
    console.log("host ice");
    that.connections[data.identity].remoteIce(data);
  });

}

/**
  Closes all user connections
  @memberof NetworkHost
*/
NetworkHost.prototype.closeAll = function(){
  for(var i in this.connections)
    this.connections[i].close();
}
/**
  Makes an offer directed at a specific user
  @memberof NetworkHost
  @param {string} identity - the namespace that identifies users individually
*/
NetworkHost.prototype.offer = function(identity){
	var promise = new Promise(function(resolve, reject){
		this.connections[identity] = new NetworkInstance(this, identity);
		this.connections[identity].offer(identity,function(err,cur){
		  if(err) return reject(err);
		  resolve(cur);
		});
	}.bind(this));
  return promise;
}
/**
  Accepts an offer
  @memberof NetworkHost
  @param {object} message - the message that was originally given
*/
NetworkHost.prototype.offerAccept = function(message){
	var promise = new Promise(function(resolve, reject){
		var identity = message.identity;
		this.connections[identity] = new NetworkInstance(this,identity);
		this.connections[identity].accept(message,function(err,cur){
		  if(err) return reject(err);
		  resolve(cur);
		});
  }.bind(this));
  return promise;
}

if(typeof module != "undefined" && module.exports){
  module.exports = NetworkHost;
}
