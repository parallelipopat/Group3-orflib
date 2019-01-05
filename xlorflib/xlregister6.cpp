/**
@file  xlregister6.cpp
@brief Registration of Excel callable functions
*/

#include <xlw/xlw.h>
using namespace xlw;

namespace {

  // Register the function ORF.EUROBS
  XLRegistration::Arg OrfBarrBSArgs[] = {
    { "PayoffType", "1: call; -1: put", "XLF_OPER" },
    { "BarrierType", "ui, uo,di, do", "XLF_OPER" },
    { "Spot", "spot", "XLF_OPER" },
    { "Strike", "strike", "XLF_OPER" },
    { "Barrier", "barrier level", "XLF_OPER" },
    { "TimeToExp", "time to expiration", "XLF_OPER" },
    { "IntRate", "risk-free interest rate (cont. cmpd)", "XLF_OPER" },
    { "DivYield", "dividend yield (cont. cmpd)", "XLF_OPER" },
    { "Vol", "volatility", "XLF_OPER" }
  };
  XLRegistration::XLFunctionRegistrationHelper regOrfBarrBS(
    "xlOrfBarrBS", "ORF.BARRBS",
    "Price of a Barrier option in the Black-Scholes model.",
    "ORFLIB", OrfBarrBSArgs, 9);

}  // anonymous namespace
