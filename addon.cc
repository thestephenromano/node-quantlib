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

    // parse data param
    std::string expireDate = info[0].As<Napi::String>().Utf8Value();
    std::vector<std::string> result;
    boost::split(result, expireDate, boost::is_any_of("/"));

    int month = std::stoi(result[0]);
    int day = std::stoi(result[1]);
    int year = std::stoi(result[2]);

    // set up Calendar, dates
    Calendar calendar = TARGET();
    Date evaluationDate(day, (Month)month, year);
    Date settlementDate = Date::nextWeekday(evaluationDate, Monday);    // Naive and does not account for holidays
    Settings::instance().evaluationDate() = evaluationDate;

    std::cout << evaluationDate << "\n";
    std::cout << settlementDate << "\n";

    return Napi::Number::New(env, 3.14);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "add"), Napi::Function::New(env, Add));
  exports.Set(Napi::String::New(env, "binomial"), Napi::Function::New(env, Binomial));
  return exports;
}

NODE_API_MODULE(addon, Init)
