var __core = __root+"/wrappers";
var async = require("async");
var fs = require("fs");
var bowerstatic = require(__dirname+"/bower_static.js");

module.exports = function(req,res,next){
  res.setHeader('content-type', 'application/javascript');
  bowerstatic.raw("/bc/eventEmitter",function(err,ee){
    if(err) return next(err);
  bowerstatic.raw("/bc/es6-promise",function(err,promise){
    if(err) return next(err);
  async.eachSeries([
    ee,
    promise,
    __core+"/abstract/StreamPromise.js",
    __core+"/abstract/MessageRouter.js",
    __core+"/abstract/MessageWriter.js",
    __core+"/abstract/MessageDuplex.js",
    __core+"/window/public/Window2Server_com.js",
    __core+"/network/public/NetworkHost.js",
    __core+"/network/public/NetworkUser.js",
  ],function(file, next){
    var temp = fs.createReadStream(file, {encoding:"utf-8"});
    temp.on('data',res.write.bind(res));
    temp.on('end',next);
    temp.on('error',next);
  },function(err){
    res.end();
    if(err) return next(err);
  });
  })
  })
}
