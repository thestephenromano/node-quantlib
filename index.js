var addon = require('bindings')('node-quantlib.node');

console.log('This should be eight:', addon.binomial(
    "2/29/2020",    // evaluation date
    "3/2/2020",     // settlement date
    "3/20/2020",    // expiration date
    "put",          // call/put
    300,            // strike
    296.26,         // underlying
    .4011,          // volatility
    .0097,          // risk free rate
    .019));         // dividend
