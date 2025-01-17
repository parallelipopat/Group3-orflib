/**
@file  simplepricers.hpp
@brief Declaration of simple pricing functions
*/

#include <orflib/defines.hpp>
#include <orflib/exception.hpp>
#include <orflib/math/matrix.hpp>
#include <orflib/market/market.hpp>

BEGIN_NAMESPACE(orf)

/** The forward price of an asset */
double fwdPrice(double spot, double timeToExp, double intRate, double divYield);

/** The quanto forward price of an asset */
double quantoFwdPrice(double spot, double timeToExp, double intRate, double divYield,
                      double assetVol, double fxVol, double correl);

/** Price of a European digital option in the Black-Scholes model*/
double digitalOptionBS(int payoffType, double spot, double strike, double timeToExp,
                       double intRate, double divYield, double volatility);

/** Price of a European option in the Black-Scholes model*/
Vector europeanOptionBS(int payoffType, double spot, double strike,
                        double timeToExp, double intRate,
                        double divYield, double volatility);


/** Price of a single point knock-out forward contract*/
double knockoutFwd(double spot, double strike, double kolevel, double timeToExp,
                   double timeToKO, double intRate, double divYield, double volatility);

/** Price of a European caplet/floorlet in the Black-Scholes model*/
double capFloorletBS(int payoffType, SPtrYieldCurve spyc, double strikeRate, 
                     double timeToReset, double tenor, double fwdRateVol);

/** Present value of a credit default swap */
orf::Vector cdsPV(SPtrYieldCurve sprfyc, double credSprd, double cdsRate,
                  double recov, double timeToMat, size_t payFreq);

/** Price of a Barrier option in the Black-Scholes model */
double barrierOptionBS(int payoffType, char barrierType[2], double spot, double strike,
                       double barrier, double timeToExp, double intRate, double divYield,
                       double volatility);

/** for Call Down In, Put Up In*/
double barrierCdiPui(int payoffType, double spot, double strike, double barrier,
                     double timeToExp, double intRate, double divYield, double volatility);

  /** for Call Down Out, Put Up Out*/
double barrierCdoPuo(int payoffType, double spot, double strike, double barrier,
                     double timeToExp, double intRate, double divYield, double volatility);

/** for Call Up In, Put Down In*/
double barrierCuiPdi(int payoffType, double spot, double strike, double barrier,
                     double timeToExp, double intRate, double divYield, double volatility);

END_NAMESPACE(orf)
