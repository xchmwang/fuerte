var node = require('bindings')('node-arangodb');
var vpack = require(__dirname + "/node_modules/node-velocypack/build/Debug/node-velocypack.node")

// create server
var serverUrl = "http://127.0.0.1:8529"
serverUrl = "vstream://127.0.0.1:8529"

var server = new node.Server(serverUrl);
var conn = server.makeConnection();

//create post

//set url path
conUrl = new node.ConnectionUrl()
conUrl.setServerUrl(serverUrl)
conUrl.setDbName("esel")
conUrl.setTailUrl("/hund/sdafdfsf")
request_data = vpack.encode({"name" : "test"})

//exact order as in create document (VppConnection)
conn.setPostReq();
conn.setPostField(request_data)
conn.setUrl(conUrl)
conn.setHeaderOpts();
conn.setBuffer(); //must be last



//fire request
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
