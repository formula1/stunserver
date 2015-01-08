var Transform = require("streams").Transform;
var util = require("util");

var HEADER = {
  SIZE:20,
  TYPELIMIT:0xc000,
  COOKIE:0x2112A442,
  MAX_MESSAGE_LENGTH:800,
};

var attributeMap = {
  0x0001: "STUN_ATTRIBUTE_MAPPEDADDRESS",
  0x0002: "STUN_ATTRIBUTE_RESPONSEADDRESS",
  0x0003: "STUN_ATTRIBUTE_CHANGEREQUEST",
  0x0004: "STUN_ATTRIBUTE_SOURCEADDRESS",
  0x0005: "STUN_ATTRIBUTE_CHANGEDADDRESS", // this is the legacy "other address" from rfc 3489, superceded by STUN_ATTRIBUTE_OTHERADDRESS below
  0x0006: "STUN_ATTRIBUTE_USERNAME",
  0x0007: "STUN_ATTRIBUTE_LEGACY_PASSWORD", // old rfc
  0x0008: "STUN_ATTRIBUTE_MESSAGEINTEGRITY",
  0x0009: "STUN_ATTRIBUTE_ERRORCODE",
  0x000A: "STUN_ATTRIBUTE_UNKNOWNATTRIBUTES",
  0x000B: "STUN_ATTRIBUTE_REFLECTEDFROM", // old rfc
  0x0014: "STUN_ATTRIBUTE_REALM",
  0x0015: "STUN_ATTRIBUTE_NONCE",
  0x0020: "STUN_ATTRIBUTE_XORMAPPEDADDRESS",
  0x0026: "STUN_ATTRIBUTE_PADDING",
  0x0027: "STUN_ATTRIBUTE_RESPONSE_PORT",
  0x8022: "STUN_ATTRIBUTE_SOFTWARE",
  0x8023: "STUN_ATTRIBUTE_ALTERNATESERVER",
  0x8028: "STUN_ATTRIBUTE_FINGERPRINT",
  0x802b: "STUN_ATTRIBUTE_RESPONSE_ORIGIN",
  0x802c: "STUN_ATTRIBUTE_OTHER_ADDRESS",

  // This attribute is sent by the server to legacy clients
  // 0x8020 is is not defined in any RFC, but is the value that Vovida server uses
  0x8020: "STUN_ATTRIBUTE_XORMAPPEDADDRESS_OPTIONAL"


}


function StunTransform(options){
  Transform.call(this,options);
  this._buffer = new Buffer(HEADER.SIZE);
  this._offset = 0;
  this.header = false;
  this.attributes = {};
  this.attributeCount = 0;
  this.curattr = false;
  var _this = this;
  Object.defineProperty(this, 'state', {
    get: function() {
      if(!_this.header) return "header";
      if(_this.offset == _this.header.msgLength+20) return "done";
      return "body";
    },
  });
}
util.inherits(StunTransform,Transform);


StunTransform.prototype._transform = function(chunk, encoding, next){
  var curoff = 0;
  if(this._offset < HEADER.SIZE){ //we are still in the header
    var copylen = Math.min(chunk.length,STUN_HEADER_SIZE-this._offset);
    chunk.copy(this._buffer,this._offset,0,copylen);
    this._offset += copylen;
    if(this._offset == 20){
      this.header = processHeader(this._buffer,this.allowLegacy);
      this._buffer = new Buffer(0);
      this.emit('header', this.header);
    }
    curoff += copylen;
  }
  if(curoff == chunk.length) return next();
  if(chunk.length+this._offset > this.header.msgLength){
    throw new Error("Too much data sent");
  }

  while(curoff < chunk.length){
    this._buffer =  Buffer.concat([this._buffer,chunk.slice(curoff)])
    this.curAttr = processAttribute(this._buffer,this.attributeCount);
    this.attributeCount++;
    this._buffer = new Buffer(0);
  }
  next();
}

StunTransform.prototype.processAttributes = function(){

}


function processHeader(buffer,allowLegacy){
  var msgType = buffer.readUInt16BE(0);
  if((msgType & HEADER.TYPELIMIT) != 0){
    throw new Error("Invalid Message Type: "+msgType);
  }
  msgType = (
    (msgType & 0x000f) |
    ((msgType & 0x00e0)>>1) |
    ((msgType & 0x3E00)>>2)
  );
  switch(msgType & 0x0110){
    case 0x0000: msgType = "request"; break;
    case 0x0010: msgType = "indication"; break;
    case 0x0100: msgType = "success"; break;
    case 0x0110: msgType = "error"; break;
    // couldn't possibly happen, because msgClass is only two bits
    default: throw new Error("Invalid Message Type: "+msgType)
  }
  ret.msgType = msgType;

  ret.msgLength = buffer.readUInt16BE(2);
  if(ret.msgLength%4 != 0){
    throw new Error("Message length is not a multiple of 4: "+ret.msgLength);
  }
  if(ret.msgLength > HEADER.MAX_MSG_LENGTH){
    throw new Error("Message length is greater than max: "+ret.msgLength+" > "+HEADER.MAX_MSG_LENGTH);
  }

  ret.transID = buffer.slice(4,20);
  ret.cookie = ret.transID.readUInt32BE(0);
  ret.isLegacy = ret.cookie != HEADER.COOKIE;
  if(!allowLegacy && ret.isLegacy){
    throw new Error("legacy header when we are not allowing it")
  }
  return ret;
}

function processAttribute(buffer,attrCount){
  var attributeType = chunk.readUInt16BE(0);
  var attributeLength = chunk.readUInt16BE(0);
  var paddingLength= (4 - attributeLength % 4)%4;
  if(attributeLength > MAX_STUN_ATTRIBUTE_SIZE){
    throw new Error(
      "Attribute["+attributeType+
      "] length is greater than max: "+attributeLength+
      " > "+Max_STUN_ATTRIBUTE_SIZE
    );
  }
  if(attrCount == MAX_NUM_ATTRIBUTES){
    throw new Error("Too many Attributes");
  }
  if(!(attributeType in attributeMap)){
    throw new Error("Invalid Attribute type or it is not supported yet")
  }
  if(attributeMap[attributeType] in this.attributes){
    throw new Error("already have attribute: "+attributeType);
  }
  var curattr = {};
  curattr.attributeType = attributeMap[attributeType];
  curattr.size = attributeLength;
  curattr.padding = paddingLength;
  curattr.value = new Buffer(attributeLength);
  buffer.copy(curattr.value,0,curoff+4,attributeLength);
  curoff += 4+attributeLength+paddingLength;
  return curattr;
}
