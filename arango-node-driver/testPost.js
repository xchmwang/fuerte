var node = require('bindings')('node-arangodb');
var vpack = require(__dirname + "/node_modules/node-velocypack/build/Debug/node-velocypack.node")

// create server
var serverUrl = "http://127.0.0.1:8529"

var server = new node.Server(serverUrl);

//create connections
var conn = server.makeConnection();

// create database hund
var database = new node.Database(server, "hund");

// create  collection
database.create(conn);
var collection = new node.Collection(database, "dackel")

// creates some object that is an equivaltent to
// the format enum in collection.h unfortunatly it
// is modifiable. should be probably some kind of
// frozen or sealed object

conUrl = new node.ConnectionUrl()
conUrl.setServerUrl(serverUrl)
conUrl.setDbName("_db")
conn.setUrl(conUrl)

conn.setPostReq();
request_data = vpack.encode({"request" : "body"})
conn.setPostField(request_data)

conn.setBuffer();


console.log("\n\ntest:")
var loops = 0;
sink = conn.SetAsynchronous(false);
do {
  ++loops;
  conn.Run();
} while (conn.IsRunning());
console.log("Sync connection result : " + conn.Result());
console.log("Sync connection loops : " + loops);
console.log("Sync response code : " + conn.ResponseCode());
