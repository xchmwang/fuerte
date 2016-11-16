var arangodb = require('bindings')('node-arangodb');
var vpack = require(__dirname + "/node_modules/node-velocypack/build/Debug/node-velocypack.node")

// create server
var serverUrl = "http://127.0.0.1:8529"
 serverUrl = "vstream://127.0.0.1:8529"
var server = new arangodb.Server(serverUrl);

//create connections
var conn = server.makeConnection();

//create post
conUrl = new arangodb.ConnectionUrl()
conUrl.setServerUrl(serverUrl)
conUrl.setDbName("testdb")
conUrl.setTailUrl("/_api/document/testcol")

conn.reset() // must be called first
conn.setHeaderOpts();
conn.setUrl(conUrl)
request_data = vpack.encode({"data" : "banana"})
conn.setPostField(request_data)
conn.setPostReq();
conn.setBuffer(); // must be called last


//fire request
sink = conn.SetAsynchronous(true);
new Promise(function (resolve, reject) {
 function run() {
   try {
     conn.Run();
     if (conn.IsRunning()) {
       setImmediate(run);
     } else {
       resolve();
     }
   } catch (e) {
     reject(e);
   }
 }
 run();
})
.then(function () {
  let vpResult = conn.Result();
  jsResult = vpack.decode(vpResult)
  console.log("Sync connection result :", jsResult);
  console.log("Sync response code : " + conn.ResponseCode());
})
.catch(function (e) {
  console.error(e.stack);
});
