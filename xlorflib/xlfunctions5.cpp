/**
@file  xlfunctions5.cpp
@brief Implementation of Excel callable functions
*/

#include <orflib/pricers/ptpricers.hpp>

#include <xlorflib/xlutils.hpp>
#include <xlw/xlw.h>

#include <cmath>

using namespace xlw;
using namespace orf;

BEGIN_EXTERN_C

LPXLFOPER EXCEL_EXPORT xlOrfPtRisk(LPXLFOPER xlPtWghts,
                                   LPXLFOPER xlAssetRets,
                                   LPXLFOPER xlAssetVols,
                                   LPXLFOPER xlCorrelMat,
                                   LPXLFOPER xlHeaders)
{
  EXCEL_BEGIN;

  if (XlfExcel::Instance().IsCalledByFuncWiz())
    return XlfOper(true);

  Vector ptwghts = xlOperToVector(XlfOper(xlPtWghts));
  Vector assetRets = xlOperToVector(XlfOper(xlAssetRets));
  Vector assetVols = xlOperToVector(XlfOper(xlAssetVols));
  Matrix correlMat = xlOperToMatrix(XlfOper(xlCorrelMat));
  // handling the xlHeaders argument
  bool headers;
  if (XlfOper(xlHeaders).IsMissing() || XlfOper(xlHeaders).IsNil())
    headers = false;
  else
    headers = XlfOper(xlHeaders).AsBool();

  std::tuple<double, double> res = ptRisk(ptwghts, assetRets, assetVols, correlMat);

  RW offset = headers ? 1 : 0;
  XlfOper xlRet(offset + 1, 2); // construct a range of size 1 x ngreeks
  if (headers) {
    xlRet(0, 0) = "MeanReturn";
    xlRet(0, 1) = "Volatility";
  };
  xlRet(offset, 0) = std::get<0>(res);
  xlRet(offset, 1) = std::get<1>(res);
  return xlRet;

  EXCEL_END;
}


LPXLFOPER EXCEL_EXPORT xlOrfMvpWghts(LPXLFOPER xlAssetRets,
                                     LPXLFOPER xlAssetVols,
                                     LPXLFOPER xlCorrelMat)
{
EXCEL_BEGIN;

  if (XlfExcel::Instance().IsCalledByFuncWiz())
    return XlfOper(true);

  Vector assetRets = xlOperToVector(XlfOper(xlAssetRets));
  Vector assetVols = xlOperToVector(XlfOper(xlAssetVols));
  Matrix correlMat = xlOperToMatrix(XlfOper(xlCorrelMat));

  Vector wghts = mvpWeights(assetRets, assetVols, correlMat);
  return xlVectorToOper(wghts);

EXCEL_END;
}


LPXLFOPER EXCEL_EXPORT xlOrfMktWghts(LPXLFOPER xlAssetRets,
                                     LPXLFOPER xlAssetVols,
                                     LPXLFOPER xlCorrelMat,
                                     LPXLFOPER xlRFreeRate)
{
  EXCEL_BEGIN;

  if (XlfExcel::Instance().IsCalledByFuncWiz())
    return XlfOper(true);

  Vector assetRets = xlOperToVector(XlfOper(xlAssetRets));
  Vector assetVols = xlOperToVector(XlfOper(xlAssetVols));
  Matrix correlMat = xlOperToMatrix(XlfOper(xlCorrelMat));
  double rfreerate = XlfOper(xlRFreeRate).AsDouble();

  Vector wghts = mktWeights(assetRets, assetVols, correlMat, rfreerate);
  return xlVectorToOper(wghts);

  EXCEL_END;
}


LPXLFOPER EXCEL_EXPORT xlOrfMktRisk(LPXLFOPER xlAssetRets,
                                    LPXLFOPER xlAssetVols,
                                    LPXLFOPER xlCorrelMat,
                                    LPXLFOPER xlRFreeRate,
                                    LPXLFOPER xlHeaders)
{
  EXCEL_BEGIN;

  if (XlfExcel::Instance().IsCalledByFuncWiz())
    return XlfOper(true);

  Vector assetRets = xlOperToVector(XlfOper(xlAssetRets));
  Vector assetVols = xlOperToVector(XlfOper(xlAssetVols));
  Matrix correlMat = xlOperToMatrix(XlfOper(xlCorrelMat));
  double rfreerate = XlfOper(xlRFreeRate).AsDouble();
  bool headers;
  if (XlfOper(xlHeaders).IsMissing() || XlfOper(xlHeaders).IsNil())
    headers = false;
  else
    headers = XlfOper(xlHeaders).AsBool();

  std::tuple<double, double, double> res = mktRisk(assetRets, assetVols, correlMat, rfreerate);

  RW offset = headers ? 1 : 0;
  XlfOper xlRet(offset + 1, 3); // construct a range of size 1 x ngreeks
  if (headers) {
    xlRet(0, 0) = "MeanReturn";
    xlRet(0, 1) = "Volatility";
    xlRet(0, 2) = "Lambda";
  };
  xlRet(offset, 0) = std::get<0>(res);
  xlRet(offset, 1) = std::get<1>(res);
  xlRet(offset, 2) = std::get<2>(res);
  return xlRet;

  EXCEL_END;
}

END_EXTERN_C
