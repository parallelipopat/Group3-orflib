/**
@file  simplepricers.cpp
@brief Implementation of simple pricing functions
*/

#include <orflib/pricers/simplepricers.hpp>
#include <orflib/math/stats/normaldistribution.hpp>

#include <cmath>

BEGIN_NAMESPACE(orf)

/** The forward price of an asset */
double fwdPrice(double spot, double timeToExp, double intRate, double divYield)
{
  ORF_ASSERT(spot >= 0.0, "spot must be non-negative");
  ORF_ASSERT(timeToExp >= 0.0, "time to expiration must be non-negative");
  ORF_ASSERT(intRate >= 0.0, "interest rate must be non-negative");
  ORF_ASSERT(divYield >= 0.0, "dividend yield must be non-negative");

  return spot * exp((intRate - divYield) * timeToExp);
}

/** The quanto forward price of an asset */
double quantoFwdPrice(double spot, double timeToExp, double intRate, double divYield,
                      double assetVol, double fxVol, double correl)
{
  double fwd = fwdPrice(spot, timeToExp, intRate, divYield);
  ORF_ASSERT(assetVol >= 0.0, "asset volatility must be non-negative");
  ORF_ASSERT(fxVol >= 0.0, "FX volatility must be non-negative");
  ORF_ASSERT(correl <= 1.0 && correl >= -1.0, "asset-FX correlation must be in [-1, 1]");

  double cvx = exp(correl * assetVol * fxVol * timeToExp);
  return cvx * fwd;
}

/** Price of a European digital option in the Black-Scholes model*/
double digitalOptionBS(int payoffType, double spot, double strike, double timeToExp,
                       double intRate, double divYield, double volatility)
{
  ORF_ASSERT(payoffType == 1 || payoffType == -1, "payoffType must be 1 or -1");
  ORF_ASSERT(strike >= 0.0, "strike must be non-negative");
  ORF_ASSERT(divYield >= 0.0, "dividend yield must be non-negative");
  ORF_ASSERT(volatility >= 0.0, "volatility must be non-negative");

  double phi = payoffType;
  double fwd = fwdPrice(spot, timeToExp, intRate, divYield);
  double sigT = volatility * sqrt(timeToExp);
  double d2 = log(fwd / strike) / sigT - 0.5 * sigT;
  NormalDistribution normal;

  double price = exp(-intRate * timeToExp) * normal.cdf(phi * d2);

  return price;
}

/** Price and Greeks of a European option in the Black-Scholes model*/
Vector europeanOptionBS(int payoffType, double spot, double strike,
                        double timeToExp, double intRate,
                        double divYield, double volatility)
{
  ORF_ASSERT(payoffType == 1 || payoffType == -1, "payoffType must be 1 or -1");
  ORF_ASSERT(strike >= 0.0, "strike must be non-negative");
  ORF_ASSERT(volatility >= 0.0, "volatility must be non-negative");

  double phi = payoffType;
  double fwd = fwdPrice(spot, timeToExp, intRate, divYield);
  double sigT = volatility * sqrt(timeToExp);
  double d1 = log(fwd / strike) / sigT + 0.5 * sigT;
  double d2 = d1 - sigT;

  NormalDistribution normal;
  double epsilon = 1.0e-012;  // a very small hard-coded number

  // precompute common quantities
  double df = exp(-intRate * timeToExp);
  double qf = exp(-divYield * timeToExp);
  double nd1 = normal.cdf(phi * d1);
  double nd2 = normal.cdf(phi * d2);
  double nprd1 = normal.pdf(d1);      // the normal density     
  double sqrtT = sqrt(timeToExp);

  // price and Greeks
  double price = phi * df * (fwd * nd1 - strike * nd2);
  double delta = phi * qf * nd1;
  double gamma = qf * nprd1 / (spot * volatility * sqrtT);
  gamma = sqrtT < epsilon ? 0.0 : gamma;
  double theta = -qf * nprd1 * spot * volatility / (2.0 * sqrtT);
  theta += phi * divYield * qf * spot * nd1;
  theta -= phi * intRate * df * strike * nd2;
  theta = sqrtT < epsilon ? 0.0 : theta;
  double vega = qf * sqrtT * spot * nprd1;

  Vector vec(5);
  vec[0] = price;
  vec[1] = delta;
  vec[2] = gamma;
  vec[3] = theta;
  vec[4] = vega;

  return vec;
}

/** Price of a single point knock-out forward contract*/
double knockoutFwd(double spot, double strike, double kolevel,
                   double timeToExp, double timeToKO,
                   double intRate, double divYield, double volatility)
{
  ORF_ASSERT(strike >= 0.0, "strike must be non-negative");
  ORF_ASSERT(kolevel >= 0.0, "knock-out level must be non-negative");
  ORF_ASSERT(timeToKO <= timeToExp, "time to knock out must be less or equal to expiration");
  ORF_ASSERT(volatility >= 0.0, "volatility must be non-negative");

  double dfko = exp(-divYield * (timeToExp - timeToKO));
  double price = europeanOptionBS(1, spot, kolevel, timeToKO, 
    intRate, divYield, volatility)[0];
  double digimult = (kolevel - exp(-(intRate - divYield) * (timeToExp - timeToKO)) * strike); 
  price +=  digimult * digitalOptionBS(1, spot, kolevel, timeToKO, 
    intRate, divYield, volatility);

  price *= dfko;
  return price;
}

/** Price of a European caplet/floorlet in the Black-Scholes model*/
double capFloorletBS(int payoffType, SPtrYieldCurve spyc, double strikeRate,
  double timeToReset, double tenor, double fwdRateVol)
{
  ORF_ASSERT(payoffType == 1 || payoffType == -1, "payoffType must be 1 or -1");
  ORF_ASSERT(strikeRate >= 0.0, "strike fwd rate must be non-negative");
  ORF_ASSERT(timeToReset >= 0.0, "time to reset must be non-negative");
  ORF_ASSERT(tenor >= 0.0, "fwd rate tenor must be non-negative");
  ORF_ASSERT(fwdRateVol >= 0.0, "fwd rate volatility must be non-negative");

  double epsilon = 1.0e-012;  // a very small hard-coded number
  double phi = payoffType;
  double timeToPay = timeToReset + tenor;                 // T2, payment time
  double frate = spyc->fwdRate(timeToReset, timeToPay);   // F(0, T1, T2)
  size_t annfreq = (size_t) (1.0 / tenor + epsilon);
  frate = fromContCmpd(frate, annfreq);
  double df = spyc->discount(timeToPay);                  // P(0, T2)
  double pervol = fwdRateVol * std::sqrt(timeToReset);          // sigma*sqrt(T2-T1)

  double d1 = log(frate / strikeRate) / pervol + 0.5 * pervol;
  double d2 = d1 - pervol;
  NormalDistribution normal;

  // precompute common quantities
  double nd1 = normal.cdf(phi * d1);
  double nd2 = normal.cdf(phi * d2);
  double nprd1 = normal.pdf(d1);      // the normal density     

  double price = phi * df * (frate * nd1 - strikeRate * nd2) * tenor;

  return price;
}

/** Present value of a credit default swap */
orf::Vector cdsPV(SPtrYieldCurve sprfyc, double credSprd, double cdsRate,
                  double recov, double timeToMat, size_t payFreq)
{
  ORF_ASSERT(credSprd > 0.0, "credit spread must be non-negative");
  ORF_ASSERT(cdsRate >= 0.0, "CDS rate must be non-negative");
  ORF_ASSERT(recov >= 0.0 && recov <= 1.0, "recovery must be between 0.0 and 1.0");
  ORF_ASSERT(timeToMat >= 0.0, "time to maturity must be non-negative");
  ORF_ASSERT(payFreq >= 1, "lay frequency must be positive");

  double DeltaT = 1.0 / payFreq;                            // regular observation/accrual period
  size_t npay = (size_t) std::ceil(timeToMat * payFreq);    // number of periods within timeToMat
  double epsilon = 1.0e-012;  // a very small hard-coded number

  if (npay == 0)
    return Vector(2, arma::fill::zeros);

  Vector paytimes(npay);
  paytimes.fill(timeToMat);
  for (size_t i = 0; i < npay; ++i) {
    paytimes[npay - i - 1] -= i * DeltaT;
  }

  Vector survprob(npay);
  survprob.fill(1.0);
  for (size_t i = 0; i < npay; ++i) {
    survprob[i] = std::exp(-credSprd * paytimes[i]) - recov;
    survprob[i] = survprob[i] > 0.0 ? survprob[i] : 0.0;  // survival prob cannot be negative!
    survprob[i] /= (1.0 - recov + epsilon);               // add epsilon to handle the case recov=1
  }

  double pvpl = 0.0;
  double pvdl = 0.0;

  for (size_t i = 0; i < npay; ++i) {
    double df = sprfyc->discount(paytimes[i]);
    double DeltaT = i == 0 ? paytimes[i] : paytimes[i] - paytimes[i - 1];
    pvpl += cdsRate *  DeltaT * survprob[i] * df;
    double defprob = i == 0 ? (1 - survprob[i]) : survprob[i - 1] - survprob[i];
    pvdl += (1.0 - recov) * defprob * df;
  }
  Vector ret(2);
  ret[0] = pvdl;
  ret[1] = pvpl;

  return ret;
}

/** Price of a Barrier option in the Black-Scholes model */
double barrierOptionBS(int payoffType, char barrierType[2], double spot, double strike,
                       double barrier, double timeToExp, double intRate, double divYield,
                       double volatility)
{
  ORF_ASSERT(payoffType == 1 || payoffType == -1, "payoffType must be 1 or -1");
  ORF_ASSERT(strike >= 0.0, "strike must be non-negative");
  ORF_ASSERT(barrier >= 0.0, "barrier must be non-negative");
  ORF_ASSERT(volatility >= 0.0, "volatility must be non-negative");
  
  ORF_ASSERT(strcmp(barrierType, "uo") == 0 || strcmp(barrierType, "ui") == 0 ||
             strcmp(barrierType, "do") == 0 || strcmp(barrierType, "di") == 0,
             "invalid barrier type: must be one of uo, ui, do, or di.");

  double price = 0.0;
  bool barrier_check = barrier <= strike;

  double option_price = europeanOptionBS(payoffType, spot, strike, timeToExp, intRate, divYield, volatility)[0];

  /** We use the fact that a put-up is the same as call-down (and put-down = call-up
      with the introduction of a negative sign in some terms, which we toggle using phi.
  */
  if (payoffType == -1) {
    barrierType[0] = barrierType[0] == 'u' ? 'd' : 'u';
    barrier_check = !barrier_check;
  }

  if (barrier_check) {
    if (strcmp(barrierType, "di") == 0) {
      price = barrierCdiPui(payoffType, spot, strike, barrier, timeToExp, intRate, divYield, volatility);
    }
    else if (strcmp(barrierType, "do") == 0) {
      price = option_price - barrierCdiPui(payoffType, spot, strike, barrier, timeToExp, intRate, divYield, volatility);
    }
    else if (strcmp(barrierType, "ui") == 0) {
      price = option_price;
    }
    else {
      price = 0.0;
    }
  }
  else {
    if (strcmp(barrierType, "di") == 0) {
      price = option_price - barrierCdoPuo(payoffType, spot, strike, barrier, timeToExp, intRate, divYield, volatility);
    }
    else if (strcmp(barrierType, "do") == 0) {
      price = barrierCdoPuo(payoffType, spot, strike, barrier, timeToExp, intRate, divYield, volatility);
    }
    else if (strcmp(barrierType, "ui") == 0) {
      price = barrierCuiPdi(payoffType, spot, strike, barrier, timeToExp, intRate, divYield, volatility);
    }
    else {
      price = option_price - barrierCuiPdi(payoffType, spot, strike, barrier, timeToExp, intRate, divYield, volatility);
    }
  }

  return price;
}


/** for Call Down In, Put Up In*/
double barrierCdiPui(int payoffType, double spot, double strike, double barrier,
                     double timeToExp, double intRate, double divYield, double volatility)
{
  double phi = payoffType;
  double lambda = (intRate - divYield)/(volatility*volatility) + 0.5;
  double sigT = volatility * sqrt(timeToExp);
  double y = log((barrier*barrier)/(spot*strike))/sigT + lambda*sigT;

  double df = exp(-intRate * timeToExp);
  double qf = exp(-divYield * timeToExp);
  
  NormalDistribution normal;

  double t1 = spot * qf * pow(barrier/spot, 2 * lambda) * normal.cdf(phi * y);
  double t2 = strike * df * pow(barrier / spot, 2 * lambda - 2) * normal.cdf(phi * (y - sigT));

  double price = phi * (t1 - t2);

  return price;
}

/** for Call Down Out, Put Up Out*/
double barrierCdoPuo(int payoffType, double spot, double strike, double barrier,
                     double timeToExp, double intRate, double divYield, double volatility)
{
  double phi = payoffType;
  double lambda = (intRate - divYield) / (volatility*volatility) + 0.5;
  double sigT = volatility * sqrt(timeToExp);
  double x1 = log(spot / barrier) / sigT + lambda * sigT;
  double y1 = log(barrier / spot) / sigT + lambda * sigT;

  double df = exp(-intRate * timeToExp);
  double qf = exp(-divYield * timeToExp);

  NormalDistribution normal;

  double t1 = spot * qf * normal.cdf(phi * x1);
  double t2 = strike * df * normal.cdf(phi * (x1 - sigT));
  double t3 = spot * qf * pow(barrier / spot, 2 * lambda) * normal.cdf(phi * y1);
  double t4 = strike * df * pow(barrier / spot, 2 * lambda - 2) * normal.cdf(phi * (y1 - sigT));

  double price = phi * (t1 - t2 - t3 + t4);

  return price;
}

/** for Call Up In, Put Down In*/
double barrierCuiPdi(int payoffType, double spot, double strike, double barrier,
                     double timeToExp, double intRate, double divYield, double volatility)
{
  double phi = payoffType;
  double phi_inv = -1 * payoffType;
  double lambda = (intRate - divYield) / (volatility*volatility) + 0.5;
  double sigT = volatility * sqrt(timeToExp);
  double x1 = log(spot / barrier) / sigT + lambda * sigT;
  double y1 = log(barrier / spot) / sigT + lambda * sigT;
  double y = log((barrier*barrier) / (spot*strike)) / sigT + lambda * sigT;

  double df = exp(-intRate * timeToExp);
  double qf = exp(-divYield * timeToExp);

  NormalDistribution normal;

  double t1 = spot * qf * normal.cdf(phi * x1);
  double t2 = strike * df * normal.cdf(phi * (x1 - sigT));
  double t3 = spot * qf * pow(barrier / spot, 2 * lambda) * (normal.cdf(phi_inv * y) - normal.cdf(phi_inv * y1));
  double t4 = strike * df * pow(barrier / spot, 2 * lambda - 2) * (normal.cdf(phi_inv * (y - sigT)) - normal.cdf(phi_inv * (y1 - sigT)));

  double price = phi * (t1 - t2 - t3 + t4);

  return price;
}

END_NAMESPACE(orf)
