var arangodb = require('bindings')('arangodb-arangodb');
var vpack = require(__dirname + "/arangodb_modules/arangodb-velocypack/build/Debug/arangodb-velocypack.arangodb")

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
conUrl.setTailUrl("/something")

conn.reset() // must be called first
conn.setHeaderOpts();
conn.setUrl(conUrl)
request_data = vpack.encode({"name" : "testdb"})
conn.setPostField(request_data)
conn.setPostReq();
conn.setBuffer(); // must be called last


//fire request
sink = conn.SetAsynchronous(false);
new Promise((resolve, reject) => {
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
.then(() => {
  let vpResult = conn.Result();
  jsResult = vpack.decode(vpResult)
  console.log("Sync connection result : " + conn.result);
  console.log("Sync response code : " + conn.ResponseCode());
})
.catch((e) => {
  console.error(e.stack);
});
