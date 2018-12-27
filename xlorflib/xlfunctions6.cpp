/**
@file  xlfunctions6.cpp
@brief Implementation of Excel callable functions
*/
#include <orflib/defines.hpp>
#include <orflib/market/market.hpp>
#include <orflib/pricers/simplepricers.hpp>

#include <xlorflib/xlutils.hpp>
#include <xlw/xlw.h>

using namespace xlw;
using namespace orf;

BEGIN_EXTERN_C

LPXLFOPER EXCEL_EXPORT xlOrfBarrBS(LPXLFOPER xlPayoffType,
                                   LPXLFOPER xlBarrierType,
                                   LPXLFOPER xlSpot,
                                   LPXLFOPER xlStrike,
                                   LPXLFOPER xlBarrier,
                                   LPXLFOPER xlTimeToExp,
                                   LPXLFOPER xlIntRate,
                                   LPXLFOPER xlDivYield,
                                   LPXLFOPER xlVolatility)
{
  EXCEL_BEGIN;
  if (XlfExcel::Instance().IsCalledByFuncWiz())
    return XlfOper(true);

  int payoffType = XlfOper(xlPayoffType).AsInt();
  double spot = XlfOper(xlSpot).AsDouble();
  double strike = XlfOper(xlStrike).AsDouble();
  double barrier = XlfOper(xlBarrier).AsDouble();
  double timeToExp = XlfOper(xlTimeToExp).AsDouble();
  double intRate = XlfOper(xlIntRate).AsDouble();
  double divYield = XlfOper(xlDivYield).AsDouble();
  double vol = XlfOper(xlVolatility).AsDouble();

  std::string barrier_type = xlStripTick(XlfOper(xlBarrierType).AsString());
  transform(barrier_type.begin(), barrier_type.end(), barrier_type.begin(), ::tolower);
//  char *barrierType = &barrier_type[0u];

  double price = barrierOptionBS(payoffType, barrier_type, spot, strike, barrier,
                                 timeToExp, intRate, divYield, vol);

  return XlfOper(price);

  EXCEL_END;
}

END_EXTERN_C
