const express = require('express');
const quantlib = require('bindings')('node-quantlib.node');

const app = express();
const port = 8081;

app.get('/optionprice/binomial', function (req, res) {

    var evaluationDate = req.query.eval;
    var settlementDate = req.query.settle;
    var expirationDate = req.query.expire;
    var optionType = req.query.t;
    var strike = req.query.s;
    var underlying = req.query.u;
    var volatility = req.query.vol;
    var riskfreeRate = req.query.rfr;
    var dividendYield = req.query.div;

    var price = quantlib.binomial(
        evaluationDate,
        settlementDate,
        expirationDate,
        optionType,
        parseFloat(strike),
        parseFloat(underlying),
        parseFloat(volatility),
        parseFloat(riskfreeRate),
        parseFloat(dividendYield)
    );

    res.setHeader('Content-Type', 'application/json');
    res.send(price)


})

app.listen(port, () => console.log(`Node-QuantLib app listening on port ${port}!`))
