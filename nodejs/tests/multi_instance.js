var exec = require("child_process").exec;

var stun = require(__dirname+"/../build/Release/stunserver");

var num = 5;
var baseport = 3000;
if(process.env.stun_number){
  num = parseInt(process.env.stun_number);
}
if(process.env.baseport){
  baseport = parseInt(process.env.baseport);
}


var stuns = [];

for(var i = 0;i<num*2;i+=2){
  console.log(3000+i);
  stuns.push(new stun.StunServer({
    protocol: "tcp",
    mode: "basic",
    primary_port: (baseport+i),
    alternate_port: (baseport+1+i),
  }));
}

console.log("created "+stuns.length+" stun servers");

startservers();
testservers(stopservers);
function startservers(){
  for(var i = 0;i<stuns.length;i++){
    stuns[i].start();
  }

  console.log("started "+stuns.length+" stun servers");
}

function testservers(cb){
  var c = 0;
  function feedback(err,stdout,stderr){
    if(stderr) throw stderr;
    console.log(stdout);
    if(err){
      throw err;
    }
    c++;
    if(c == stuns.length){
      console.log("tested "+stuns.length+" stun servers");
      cb();
    }
  }
  for(var i=0;i<stuns.length;i++){
    exec(__dirname+"/../../client/stunclient localhost "+(baseport+i*2)+" -localaddr=127.0.0.1",
    feedback);
  }
}


function stopservers(){
  for(var i = 0;i<stuns.length;i++){
    stuns[i].stop();
  }
  console.log("stopped "+stuns.length+" stun servers");
}
