'use strict';
module.exports = {
  fuerte: require('bindings')('node-arangodb'),
  vpack: require(__dirname + "/node_modules/node-velocypack/build/Debug/node-velocypack.node")
};
