/**
@file  barriercallput.hpp
@brief The payoff of a European Barrier Call/Put option
*/

#ifndef ORF_BARRIERCALLPUT_HPP
#define ORF_BARRIERCALLPUT_HPP

#include <orflib/products/product.hpp>

BEGIN_NAMESPACE(orf)

/** The Barrier call/put class
*/
class BarrierCallPut : public Product
{
public:

  enum class Freq
  {
    MONTHLY,      // 12/year
    WEEKLY,       // 52/year
    DAILY         // 365/year
  };

  /** Initializing ctor */
  BarrierCallPut(int payoffType, double strike, double barrier, std::string barrierType, Freq frequency, double timeToExp);

  /** The number of assets this product depends on */
  virtual size_t nAssets() const override { return 1; }

  /** Evaluates the product given the passed-in path
    The "pricePath" matrix must have as many rows as
    the number of fixing times
  */
  virtual void eval(Matrix const& pricePath) override;

  /** Evaluates the product at fixing time index idx
  */
  virtual void eval(size_t idx, Vector const& spots, double contValue) override;

  /** Returns whether the grid needs to be aligned for PDE solving */
  bool needsAlignment() override;

  /** Returns the alignment vector */
  std::vector<double> getAlignmentVector() override;

private:
  int payoffType_;          // 1: call; -1 put
  double strike_;
  double barrier_;
  std::string barrier_type_;
  Freq frequency_;
  double timeToExp_;
};

///////////////////////////////////////////////////////////////////////////////
// Inline definitions

inline
BarrierCallPut::BarrierCallPut(int payoffType, double strike, double barrier, std::string barrierType, Freq frequency, double timeToExp)
  : payoffType_(payoffType), strike_(strike), barrier_(barrier), frequency_(frequency), barrier_type_(barrierType), timeToExp_(timeToExp)
{
  ORF_ASSERT(payoffType == 1 || payoffType == -1, "BarrierCallPut: the payoff type must be 1 (call) or -1 (put)!");
  ORF_ASSERT(strike > 0.0, "BarrierCallPut: the strike must be positive!");
  ORF_ASSERT(barrier > 0.0, "BarrierCallPut: the barrier must be positive!");
  ORF_ASSERT(timeToExp > 0.0, "BarrierCallPut: the time to expiration must be positive!");
  ORF_ASSERT(barrierType == "uo" || barrierType == "do",
             "BarrierCallPut: invalid barrier type : must be either uo or do.")

  double time_factor = 0.0;
  double monitoring_freq = 0.0;
  double nfixings = 0.0;

  switch (frequency) {
  case BarrierCallPut::Freq::MONTHLY:
    monitoring_freq = 12.0;
    time_factor = 30.0;
    break;
  case BarrierCallPut::Freq::WEEKLY:
    monitoring_freq = 52.0;
    time_factor = 7.0;
    break;
  case BarrierCallPut::Freq::DAILY:
    monitoring_freq = 365.0;
    time_factor = 1.0;
    break;
  default:
    ORF_ASSERT(0, "BarrierCallPut: unknown barrier option frequency type!");
  }

  double num_days = ceil(timeToExp_ * 365.0);
  double rounded_num_days = floor(num_days / time_factor) * time_factor;

  double stub_time = (num_days > rounded_num_days) ? num_days - rounded_num_days : 0.0;
  double offset = stub_time > 0 ? 1.0 : 0.0;

  // number of fixing times determined by Freq = time_factor AS WELL AS whether there is a stub
  nfixings = timeToExp_ * monitoring_freq + offset + 1;
  size_t rounded_up_nfixings = static_cast<size_t>(ceil(nfixings));
  ORF_ASSERT(rounded_up_nfixings > 0, "BarrierCallPut: the option has expired!");

  // set the fixing times
  fixTimes_.resize(rounded_up_nfixings);
  fixTimes_[0] = 0.0;

  for (size_t i = 0; i < rounded_up_nfixings - 1; ++i)
    fixTimes_[i + offset] = i * (time_factor/365.0) + stub_time/365.0;
  fixTimes_[rounded_up_nfixings - 1] = timeToExp_;

  payTimes_ = fixTimes_;

  // this product takes either its continuation value or zero at the fixing times
  payAmounts_.resize(payTimes_.size());
}

inline void BarrierCallPut::eval(Matrix const& pricePath)
{
  ORF_ASSERT(0, "Not implemented yet!");

  // This implementation not neccessary
}

inline void BarrierCallPut::eval(size_t idx, Vector const& spots, double contValue)
{
	double spot = spots[0];

	if (idx == payAmounts_.size() - 1) { // this is the last index
		double payoff = (spot - strike_) * payoffType_;
		if (barrier_type_[0] == 'u') {
			payAmounts_[idx] = payoff > 0.0 && spot < barrier_ - 0.00001 ? payoff : 0.0;
		}
		else {
			payAmounts_[idx] = payoff > 0.0 && spot > barrier_ + 0.00001 ? payoff : 0.0;
		}	
	}
	else {  // this is not the last index, check whether barrier has been breached
		if (barrier_type_[0] == 'u') {
			payAmounts_[idx] = spot < barrier_ - 0.00001 ? contValue : 0;
		}
		else {
			payAmounts_[idx] = spot > barrier_ + 0.00001 ? contValue : 0;
		}
	}
}

inline bool BarrierCallPut::needsAlignment()
{
  return true;
}

inline std::vector<double> BarrierCallPut::getAlignmentVector()
{
  return { barrier_ };
}

END_NAMESPACE(orf)

#endif // ORF_BARRIERCALLPUT_HPP