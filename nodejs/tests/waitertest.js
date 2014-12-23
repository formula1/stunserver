var cluster = require("cluster");


if (cluster.isMaster) {
  cluster.fork();

  cluster.on("exit",function(worker, code, signal) {
    throw new Error("the server should not be exiting");
  });
  setTimeout(function(){
    console.log("stun server stays alive");
    process.exit();
  },5000);
}else{
  var stun = require(__dirname+"/../build/Release/stunserver");

  var stunserver = new stun.StunServer({
    protocol: "tcp",
    mode: "basic",
    verbosity:5
  });

  stunserver.start();
}
