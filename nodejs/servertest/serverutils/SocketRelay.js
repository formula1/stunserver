var users = {};
var handles = {};
/*
need access to user
message.id = i;
RTC.emit(i,message)
*/
module.exports = UserEnter;

function UserEnter(ws){
  if(ws.id in users){
    return Command(message,users[ws.id]);
  }
  ws.on("close", function(){
    UserLeave(ws.id);
  })
  ws.add({
    "offer": Offer,
    "accept": Accept,
    "ice": Ice
  })
  users[ws.id] = {
    id:ws.id,
    offers:[],
    connections:[]
  };
  handles[ws.id] = ws;
  ws.trigger("self",ws.id);
  for(var i in handles){
    handles[i].trigger("list",users);
  }
};

function UserLeave(messageid){
  delete users[messageid];
  for(var i in users){
    handles[i].trigger("list",users);
  }
}

function Offer (message,call_ob){
  if(!handles[message.identity]){
    throw new Error("cannot offer to a nonexistant user");
  }
  if(message.identity == call_ob.user.id){
    throw new Error("cannot offer to self: "+message.identity+", "+user.id);
  }
  /*
  This is where you filter the offer
  */

  users[call_ob.user.id].offers.push(message.identity)
  var oid = message.identity;
  message.identity = call_ob.user.id;
  handles[oid].trigger("offer",message);
};

function Accept (message,call_ob){
  console.log("accept");
  if(!users[message.identity]){
    throw new Error("cannot accept to a nonexistant user");
  }
  if(message.identity == call_ob.user.id){
    throw new Error("cannot accept to self: "+message.identity+", "+call_ob.user.id);
  }
  var i = users[message.identity].offers.indexOf(call_ob.user.id);
  if(i == -1){
    throw new Error("cannot accept a user who never offered");
  }
  var oid = message.identity;
  message.identity = call_ob.user.id;
  delete users[oid].offers[i];
  users[oid].connections.push(call_ob.user.id);
  users[call_ob.user.id].connections.push(oid);
  handles[oid].trigger("accept",message);
};

function Ice (message,user){
  console.log("ice");
  if(!users[message.identity]){
    throw new Error("cannot ice to a nonexistant user");
  }
  if(message.identity == call_ob.user.id){
    throw new Error("cannot ice to self: "+message.identity+", "+call_ob.user.id);
  }
  var oid = message.identity;
  message.identity = call_ob.user.id;
  handles[oid].trigger("ice",message);
};
