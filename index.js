var addon = require('bindings')('node-quantlib.node');

console.log('This should be eight:', addon.binomial("2/28/2020"));
