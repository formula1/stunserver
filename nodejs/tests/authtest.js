var exec = require("child_process").exec;

var stun = require("../build/Release/stunserver");

var stunserver = new stun.StunServer({
  protocol: "tcp",
  mode: "basic",
  verbosity:3
});
stunserver.start();

var authcbs = [
  function(AuthReq,AuthRes,next){
    process.nextTick(function(){
      console.log(JSON.stringify(AuthRes)+1);
      next(AuthRes);
    });
  },
  function(AuthReq,AuthRes,next){
    process.nextTick(function(){
      console.log(JSON.stringify(AuthRes)+2);
      next(AuthRes);
    });
  },
  function(AuthReq,AuthRes,next){
    process.nextTick(function(){
      console.log(JSON.stringify(AuthRes)+3);
      next(AuthRes);
    });
  },
];

var c = 0;
for(var i=0;i<authcbs.length;i++){
  stunserver.doAuth = authcbs[i];
  exec(__dirname+"/../../client/stunclient localhost -localaddr=127.0.0.1",
  MadeCall);
}

function MadeCall(err,stdout,stderr){
  console.log(c);
  if(err){
    throw err;
  }
  if(stderr) throw stderr;
  console.log(stdout);
  c++;
  if(c == authcbs.length){
    console.log("tested "+authcbs.length+" different Auth CallBacks");
    stunserver.stop();
  }
}
