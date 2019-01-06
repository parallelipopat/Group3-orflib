/**
@file  xlfunctions6.cpp
@brief Implementation of Excel callable functions
*/
#include <orflib/defines.hpp>
#include <orflib/market/market.hpp>
#include <orflib/pricers/simplepricers.hpp>
#include <orflib/methods/pde/pde1dsolver.hpp>
#include <orflib/products/barriercallput.hpp>

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
  // make the barrier type argument case insensitive.
  transform(barrier_type.begin(), barrier_type.end(), barrier_type.begin(), ::tolower);
  // convert the string to type char*
  char *barrierType = &barrier_type[0u];

  double price = barrierOptionBS(payoffType, barrierType, spot, strike, barrier,
                                 timeToExp, intRate, divYield, vol);

  return XlfOper(price);

  EXCEL_END;
}

LPXLFOPER EXCEL_EXPORT xlOrfBarrBSPDE(LPXLFOPER xlPayoffType,
                                      LPXLFOPER xlStrike,
                                      LPXLFOPER xlTimeToExp,
                                      LPXLFOPER xlSpot,
                                      LPXLFOPER xlBarrier,
                                      LPXLFOPER xlBarrierType,
                                      LPXLFOPER xlFrequency,
                                      LPXLFOPER xlDiscountCrv,
                                      LPXLFOPER xlDivYield,
                                      LPXLFOPER xlVolatility,
                                      LPXLFOPER xlPdeParams,
                                      LPXLFOPER xlHeaders)
{
  EXCEL_BEGIN;

  if (XlfExcel::Instance().IsCalledByFuncWiz())
    return XlfOper(true);

  int payoffType = XlfOper(xlPayoffType).AsInt();
  double spot = XlfOper(xlSpot).AsDouble();
  double strike = XlfOper(xlStrike).AsDouble();
  double barrier = XlfOper(xlBarrier).AsDouble();
  double timeToExp = XlfOper(xlTimeToExp).AsDouble();
  int frequency = XlfOper(xlFrequency).AsInt();

  BarrierCallPut::Freq freq;
  switch (frequency) {
  case 0:
    freq = BarrierCallPut::Freq::MONTHLY;
    break;
  case 1:
    freq = BarrierCallPut::Freq::WEEKLY;
    break;
  case 2:
    freq = BarrierCallPut::Freq::DAILY;
    break;
  default:
    ORF_ASSERT(0, "error: unknown barrier option frequency type");
  }

  std::string barrier_type = xlStripTick(XlfOper(xlBarrierType).AsString());
  // make the barrier type argument case insensitive.
  transform(barrier_type.begin(), barrier_type.end(), barrier_type.begin(), ::tolower);
  // convert the string to type char*
  char *barrierType = &barrier_type[0u];

  std::string name = xlStripTick(XlfOper(xlDiscountCrv).AsString());
  SPtrYieldCurve spyc = market().yieldCurves().get(name);
  ORF_ASSERT(spyc, "error: yield curve " + name + " not found");

  double divYield = XlfOper(xlDivYield).AsDouble();
  SPtrVolatilityTermStructure spvol;
  if (XlfOper(xlVolatility).IsNumber()) {
    double vol = XlfOper(xlVolatility).AsDouble();
    spvol.reset(new VolatilityTermStructure(&timeToExp, &timeToExp + 1, &vol, &vol + 1));
  }
  else {  // assume string
    std::string volname = xlStripTick(XlfOper(xlVolatility).AsString());
    spvol = market().volatilities().get(volname);
    ORF_ASSERT(spvol, "error: volatility " + volname + " not found");
  }

  // read the PDE parameters
  PdeParams pdeparams = xlOperToPdeParams(xlPdeParams);
  // handling the xlHeaders argument
  bool headers;
  if (XlfOper(xlHeaders).IsMissing() || XlfOper(xlHeaders).IsNil())
    headers = false;
  else
    headers = XlfOper(xlHeaders).AsBool();

  // create the product
  SPtrProduct spprod(new BarrierCallPut(payoffType, strike, barrier, barrierType, freq, timeToExp));

  // create the PDE solver
  Pde1DResults results;
  bool storeAllResults = headers;
  Pde1DSolver solver(spprod, spyc, spot, divYield, spvol, results, storeAllResults);
  solver.solve(pdeparams);

  RW nrows = headers ? 1 + (RW)results.times.size() : 1;
  COL ncols = headers ? 2 + (COL)results.values.front().n_rows : 1;
  XlfOper xlRet(nrows, ncols); // construct a range of size nrows x ncols
  if (headers) {
    xlRet(0, 0) = "Price";
    xlRet(1, 0) = results.prices[0];
    for (RW i = 2; i < nrows; ++i)  xlRet(i, 0) = XlfOper::Error(xlerrNA);

    xlRet(0, 1) = "Time/Spot";
    Vector spots;
    results.getSpotAxis(0, spots);

    for (size_t i = 0; i < spots.size(); ++i)
      xlRet(0, 2 + (COL)i) = spots[i];
    for (size_t i = 0; i < results.times.size(); ++i) {
      xlRet(1 + (RW)i, 1) = results.times[i];
      for (size_t j = 0; j < results.values.front().n_rows; ++j)
        xlRet(1 + (RW)i, 2 + (COL)j) = results.values[i](j, 0);
    }
  }
  else {
    xlRet(0, 0) = results.prices[0];
  }

  return xlRet;

  EXCEL_END;
}

END_EXTERN_C
