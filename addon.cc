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

using namespace QuantLib;


Napi::Value Add(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  if (info.Length() < 2) {
    Napi::TypeError::New(env, "Wrong number of arguments")
        .ThrowAsJavaScriptException();
    return env.Null();
  }

  if (!info[0].IsNumber() || !info[1].IsNumber()) {
    Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
    return env.Null();
  }

  double arg0 = info[0].As<Napi::Number>().DoubleValue();
  double arg1 = info[1].As<Napi::Number>().DoubleValue();
  Napi::Number num = Napi::Number::New(env, arg0 + arg1);

  return num;
}

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
            new BinomialVanillaEngine<JarrowRudd>(bsmProcess,timeSteps)));

    return Napi::Number::New(env, americanOption.NPV());
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "add"), Napi::Function::New(env, Add));
  exports.Set(Napi::String::New(env, "binomial"), Napi::Function::New(env, Binomial));
  return exports;
}

NODE_API_MODULE(addon, Init)
