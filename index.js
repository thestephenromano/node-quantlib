var addon = require('bindings')('node-quantlib.node');

console.log('Binomial: ', addon.binomial(
    "2/29/2020",    // evaluation date
    "3/2/2020",     // settlement date
    "3/20/2020",    // expiration date
    "put",          // call/put
    275,            // strike
    296.26,         // underlying
    .4011,          // volatility
    .0097,          // risk free rate
    .019));         // dividend

console.log('Monte Carlo: ', addon.montecarlo(
    "2/29/2020",    // evaluation date
    "3/2/2020",     // settlement date
    "3/20/2020",    // expiration date
    "put",          // call/put
    275,            // strike
    296.26,         // underlying
    .4011,          // volatility
    .0097,          // risk free rate
    .019));         // dividend