var node = require('bindings')('node-arangodb');
var vpack = require(__dirname + "/node_modules/node-velocypack/build/Debug/node-velocypack.node")

// create server
var serverUrl = "http://127.0.0.1:8529"
serverUrl = "vstream://127.0.0.1:8529"

var server = new node.Server(serverUrl);

//create connections
var conn = server.makeConnection();


/////////////////////////////////////////////////////////////
// if these functions are not executed the program with
// terminate with a segfault
//
 var database = new node.Database(server, "hund");
 database.create(conn);
 var collection = new node.Collection(database, "dackel")
/////////////////////////////////////////////////////////////

//create post

//set url path
conUrl = new node.ConnectionUrl()
conUrl.setServerUrl(serverUrl)
conUrl.setDbName("collection")
conUrl.setTailUrl("/dfasdfsad/sdafdfsf")

//conn.reset() // results in operaton canelled
conn.setHeaderOpts();
conn.setUrl(conUrl)
request_data = vpack.encode({"name" : "test"})
conn.setPostField(request_data)
conn.setPostReq();
conn.setBuffer();


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
