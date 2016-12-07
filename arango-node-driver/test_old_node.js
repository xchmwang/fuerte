var arangodb = require('bindings')('node-arangodb');
var vpack = require(__dirname + "/node_modules/node-velocypack/build/Debug/node-velocypack.node")

// create server and connection
var serverUrl = "http://127.0.0.1:8529"
serverUrl = "vstream://127.0.0.1:8529"
var server = new arangodb.Server(serverUrl);
var conn = server.makeConnection();

// create post
conUrl = new arangodb.ConnectionUrl()
conUrl.setServerUrl(serverUrl)
conUrl.setDbName("testdb")
conUrl.setTailUrl("/_api/document/testcol")

conn.reset() // must be called first
conn.setHeaderOpts();
conn.setUrl(conUrl)
request_data = vpack.encode({"data" : "banana", "_key": "123456"})
conn.setPostField(request_data)
conn.setPostReq();
conn.setBuffer(); // must be called last

// fire request
conn.SetAsynchronous(true);
do{
  conn.Run();
} while(conn.IsRunning());

// get result
var vpResult = conn.Result();
var jsResult = vpack.decode(vpResult)
console.log("result :", jsResult);


conUrl2 = new arangodb.ConnectionUrl()
conUrl2.setServerUrl(serverUrl)
conUrl2.setDbName("testdb")
conUrl2.setTailUrl("/_api/document/testcol/123456")

conn.reset() // must be called first
conn.setHeaderOpts();
conn.setUrl(conUrl2)
//request_data = vpack.encode({"data" : "banana"})
//conn.setPostField(request_data)
conn.setGetReq();
conn.setBuffer(); // must be called last

// fire request
conn.SetAsynchronous(true);
do{
  conn.Run();
} while(conn.IsRunning());

// get result
var vpResult = conn.Result();
var jsResult = vpack.decode(vpResult)
console.log("result :", jsResult);
