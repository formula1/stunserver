
var stun = require("../build/Release/stunserver");


var stunserver = new stun.StunServer({
  protocol: "tcp",
  mode: "basic",
  verbosity:5
});

stunserver.start();
