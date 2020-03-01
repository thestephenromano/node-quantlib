#include <napi.h>

#include <ql/qldefines.hpp>
#include <ql/instruments/vanillaoption.hpp>
#include <ql/pricingengines/vanilla/binomialengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/pricingengines/vanilla/baroneadesiwhaleyengine.hpp>
#include <ql/pricingengines/vanilla/bjerksundstenslandengine.hpp>
#include <ql/pricingengines/vanilla/batesengine.hpp>
#include <ql/pricingengines/vanilla/integralengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/pricingengines/vanilla/mceuropeanengine.hpp>
#include <ql/pricingengines/vanilla/mcamericanengine.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/utilities/dataformatters.hpp>

#include <boost/algorithm/string.hpp>
#include <ctime>

using namespace QuantLib;


Napi::Value Binomial(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // set up dates
    //

    // eval date
    std::string evalDateParam = info[0].As<Napi::String>().Utf8Value();
    std::vector<std::string> result;
    boost::split(result, evalDateParam, boost::is_any_of("/"));

    int evalMonth = std::stoi(result[0]);
    int evalDay = std::stoi(result[1]);
    int evalYear = std::stoi(result[2]);

    // settlement date
    std::string settlementDateParam = info[1].As<Napi::String>().Utf8Value();
    boost::split(result, settlementDateParam, boost::is_any_of("/"));

    int settlementMonth = std::stoi(result[0]);
    int settlementDay = std::stoi(result[1]);
    int settlementYear = std::stoi(result[2]);

    // expire date
    std::string expirationDateParam = info[2].As<Napi::String>().Utf8Value();
    boost::split(result, expirationDateParam, boost::is_any_of("/"));

    int expirationMonth = std::stoi(result[0]);
    int expirationDay = std::stoi(result[1]);
    int expirationYear = std::stoi(result[2]);

    Date evaluationDate(evalDay, (Month)evalMonth, evalYear);
    Date settlementDate(settlementDay, (Month)settlementMonth, settlementYear); // earliest can be excised
    Date expirationDate(expirationDay, (Month)expirationMonth, expirationYear);

    std::cout << "Evaluation Date: " << evaluationDate << "\n";
    std::cout << "Settlement Date: " << settlementDate << "\n";
    std::cout << "Expiration Date: " << expirationDate << "\n";

    Calendar calendar = TARGET();
    Settings::instance().evaluationDate() = evaluationDate;

    // our options
    //

    Option::Type type(Option::Put);
    Real strike = info[4].As<Napi::Number>().DoubleValue();
    Real underlying = info[5].As<Napi::Number>().DoubleValue();
    Volatility volatility = info[6].As<Napi::Number>().DoubleValue();
    Rate riskFreeRate = info[7].As<Napi::Number>().DoubleValue();
    Spread dividendYield = info[8].As<Napi::Number>().DoubleValue();
    DayCounter dayCounter = Actual365Fixed();

    std::cout << "Strike: " << strike << "\n";
    std::cout << "Underlying: " << underlying << "\n";
    std::cout << "Volatility: " << volatility << "\n";
    std::cout << "Risk free rate: " << riskFreeRate << "\n";
    std::cout << "Dividend yield: " << dividendYield << "\n";

    ext::shared_ptr<Exercise> americanExercise(new AmericanExercise(settlementDate, expirationDate));
    Handle<Quote> underlyingH(ext::shared_ptr<Quote>(new SimpleQuote(underlying)));

    // bootstrap the yield/dividend/vol curves
    Handle<YieldTermStructure> flatTermStructure(
        ext::shared_ptr<YieldTermStructure>(
            new FlatForward(settlementDate, riskFreeRate, dayCounter)));

    Handle<YieldTermStructure> flatDividendTS(
        ext::shared_ptr<YieldTermStructure>(
            new FlatForward(settlementDate, dividendYield, dayCounter)));

    Handle<BlackVolTermStructure> flatVolTS(
        ext::shared_ptr<BlackVolTermStructure>(
            new BlackConstantVol(settlementDate, calendar, volatility, dayCounter)));

    ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));
    ext::shared_ptr<BlackScholesMertonProcess> bsmProcess(
             new BlackScholesMertonProcess(underlyingH, flatDividendTS, flatTermStructure, flatVolTS));

	// options
	VanillaOption americanOption(payoff, americanExercise);
    Size timeSteps = 801;

    americanOption.setPricingEngine(ext::shared_ptr<PricingEngine>(
        new BinomialVanillaEngine<CoxRossRubinstein>(bsmProcess, timeSteps)));

    std::clock_t start;
    double duration;
    start = std::clock();

    Real npv = americanOption.NPV();
    Real delta = americanOption.delta();
    Real gamma = americanOption.gamma();
    Real theta = americanOption.theta() / 365.0;
    //Real vega = americanOption.vega();
    //Real rho = americanOption.rho();

    std::cout << "NPV: " << npv << "\n";
    std::cout << "delta: " << delta << "\n";
    std::cout << "gamma: " << gamma << "\n";
    std::cout << "theta: " << theta << "\n";
    //std::cout << "vega: " << vega << "\n";
    //std::cout << "rho: " << rho << "\n";

    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

    std::stringstream fmt;
    fmt << "{";
    fmt << "\"npv\": " << npv;
    fmt << ", " << "\"delta\": " << delta;
    fmt << ", " << "\"gamma\": " << gamma;
    fmt << ", " << "\"theta\" :" << theta;
    fmt << ", " <<  "\"_duration\": " << duration;
    fmt << "}";

    return Napi::String::New(env, fmt.str());
}

Napi::Value MonteCarlo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // set up dates
    //

    // eval date
    std::string evalDateParam = info[0].As<Napi::String>().Utf8Value();
    std::vector<std::string> result;
    boost::split(result, evalDateParam, boost::is_any_of("/"));

    int evalMonth = std::stoi(result[0]);
    int evalDay = std::stoi(result[1]);
    int evalYear = std::stoi(result[2]);

    // settlement date
    std::string settlementDateParam = info[1].As<Napi::String>().Utf8Value();
    boost::split(result, settlementDateParam, boost::is_any_of("/"));

    int settlementMonth = std::stoi(result[0]);
    int settlementDay = std::stoi(result[1]);
    int settlementYear = std::stoi(result[2]);

    // expire date
    std::string expirationDateParam = info[2].As<Napi::String>().Utf8Value();
    boost::split(result, expirationDateParam, boost::is_any_of("/"));

    int expirationMonth = std::stoi(result[0]);
    int expirationDay = std::stoi(result[1]);
    int expirationYear = std::stoi(result[2]);

    Date evaluationDate(evalDay, (Month)evalMonth, evalYear);
    Date settlementDate(settlementDay, (Month)settlementMonth, settlementYear); // earliest can be excised
    Date expirationDate(expirationDay, (Month)expirationMonth, expirationYear);

    std::cout << "Evaluation Date: " << evaluationDate << "\n";
    std::cout << "Settlement Date: " << settlementDate << "\n";
    std::cout << "Expiration Date: " << expirationDate << "\n";

    Calendar calendar = TARGET();
    Settings::instance().evaluationDate() = evaluationDate;

    // our options
    //

    Option::Type type(Option::Put);
    Real strike = info[4].As<Napi::Number>().DoubleValue();
    Real underlying = info[5].As<Napi::Number>().DoubleValue();
    Volatility volatility = info[6].As<Napi::Number>().DoubleValue();
    Rate riskFreeRate = info[7].As<Napi::Number>().DoubleValue();
    Spread dividendYield = info[8].As<Napi::Number>().DoubleValue();
    DayCounter dayCounter = Actual365Fixed();

    std::cout << "Strike: " << strike << "\n";
    std::cout << "Underlying: " << underlying << "\n";
    std::cout << "Volatility: " << volatility << "\n";
    std::cout << "Risk free rate: " << riskFreeRate << "\n";
    std::cout << "Dividend yield: " << dividendYield << "\n";

    ext::shared_ptr<Exercise> americanExercise(new AmericanExercise(settlementDate, expirationDate));
    Handle<Quote> underlyingH(ext::shared_ptr<Quote>(new SimpleQuote(underlying)));

    // bootstrap the yield/dividend/vol curves
    Handle<YieldTermStructure> flatTermStructure(
        ext::shared_ptr<YieldTermStructure>(
            new FlatForward(settlementDate, riskFreeRate, dayCounter)));

    Handle<YieldTermStructure> flatDividendTS(
        ext::shared_ptr<YieldTermStructure>(
            new FlatForward(settlementDate, dividendYield, dayCounter)));

    Handle<BlackVolTermStructure> flatVolTS(
        ext::shared_ptr<BlackVolTermStructure>(
            new BlackConstantVol(settlementDate, calendar, volatility, dayCounter)));

    ext::shared_ptr<StrikedTypePayoff> payoff(new PlainVanillaPayoff(type, strike));
    ext::shared_ptr<BlackScholesMertonProcess> bsmProcess(
             new BlackScholesMertonProcess(underlyingH, flatDividendTS, flatTermStructure, flatVolTS));

	// options
	VanillaOption americanOption(payoff, americanExercise);

    // Monte Carlo Method: MC (Longstaff Schwartz)
    Size mcSeed = 42;
    ext::shared_ptr<PricingEngine> mcengine3;
            mcengine3 = MakeMCAmericanEngine<PseudoRandom>(bsmProcess)
                .withSteps(100)
                .withAntitheticVariate()
                .withCalibrationSamples(4096)
                .withAbsoluteTolerance(0.02)
                .withSeed(mcSeed);
    americanOption.setPricingEngine(mcengine3);

    std::clock_t start;
    double duration;
    start = std::clock();

    Real npv = americanOption.NPV();
    //Real delta = americanOption.delta();
    //Real gamma = americanOption.gamma();
    //Real theta = americanOption.theta() / 365.0;
    //Real vega = americanOption.vega();
    //Real rho = americanOption.rho();

    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

    std::cout << "NPV: " << npv << "\n";
    //std::cout << "delta: " << delta << "\n";
    //std::cout << "gamma: " << gamma << "\n";
    //std::cout << "theta: " << theta << "\n";
    //std::cout << "vega: " << vega << "\n";
    //std::cout << "rho: " << rho << "\n";

    std::stringstream fmt;
    fmt << "{";
    fmt << "\"npv\": " << npv;
    fmt << ", " <<  "\"_duration\": " << duration;
    fmt << "}";

    return Napi::String::New(env, fmt.str());
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "binomial"), Napi::Function::New(env, Binomial));
  exports.Set(Napi::String::New(env, "montecarlo"), Napi::Function::New(env, MonteCarlo));
  return exports;
}

NODE_API_MODULE(addon, Init)
