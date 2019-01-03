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
  BarrierCallPut(int payoffType, double strike, double barrier, std::string barrier_type, Freq frequency, double timeToExp);

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
BarrierCallPut::BarrierCallPut(int payoffType, double strike, double barrier, std::string barrier_type, Freq frequency, double timeToExp)
  : payoffType_(payoffType), strike_(strike), barrier_(barrier), frequency_(frequency), barrier_type_(barrier_type), timeToExp_(timeToExp)
{
  ORF_ASSERT(payoffType == 1 || payoffType == -1, "BarrierCallPut: the payoff type must be 1 (call) or -1 (put)!");
  ORF_ASSERT(strike > 0.0, "BarrierCallPut: the strike must be positive!");
  ORF_ASSERT(barrier > 0.0, "BarrierCallPut: the barrier must be positive!");
  ORF_ASSERT(timeToExp > 0.0, "BarrierCallPut: the time to expiration must be positive!");

  double time_factor = 0.0;
  size_t nfixings = static_cast<size_t>(0);

  switch (frequency) {
  case BarrierCallPut::Freq::MONTHLY:
    time_factor = 12.0;
    break;
  case BarrierCallPut::Freq::WEEKLY:
    time_factor = 52.0;
    break;
  case BarrierCallPut::Freq::DAILY:
    time_factor = 365.0;
    break;
  default:
    ORF_ASSERT(0, "BarrierCallPut: unknown barrier option frequency type!");
  }

  // number of fixing times determined by Freq = time_factor
  //nfixings = timeToExp * time_factor + 1;
  //double rounded_up_nfixings = ceil(nfixings);
  //ORF_ASSERT(rounded_up_nfixings > 0, "BarrierCallPut: the option has expired!");

  //double stub_time = rounded_up_nfixings > nfixings ? (timeToExp - floor(timeToExp)) / time_factor : 0.0;
  //double offset = rounded_up_nfixings > nfixings ? 1.0 : 0.0;

  // set the fixing times
  //fixTimes_.resize(rounded_up_nfixings);
  //fixTimes_[0] = 0.0;

  //for (size_t i = 0; i < rounded_up_nfixings - 1; ++i)
  //  fixTimes_[i + offset] = i / time_factor + stub_time;
  //fixTimes_[nfixings - 1] = timeToExp_;

  // payTimes_.resize(1);
  // payTimes_[0] = timeToExp_;

  // this product generates only one payment
  // payAmounts_.resize(1);
  nfixings = static_cast<size_t>(timeToExp * DAYS_PER_YEAR) + 1;
  ORF_ASSERT(nfixings > 0, "BarrierCallPut: the option has expired!");
  fixTimes_.resize(nfixings);
  for (size_t i = 0; i < nfixings - 1; ++i)
	  fixTimes_[i] = i / DAYS_PER_YEAR;
  fixTimes_[nfixings - 1] = timeToExp_;

  payTimes_ = fixTimes_;

  // this product could generate a payment on each day between now and expiration.
  payAmounts_.resize(payTimes_.size());
}

inline void BarrierCallPut::eval(Matrix const& pricePath)
{
  ORF_ASSERT(0, "Not implemented yet!");

  // I suspect this implementation isn't necessary. This was used for Monte Carlo i think
}

inline void BarrierCallPut::eval(size_t idx, Vector const& spots, double contValue)
{
  // the continuation value is not used
	double spot = spots[0];

	if (idx == payAmounts_.size() - 1) { // this is the last index
		double payoff = (spot - strike_) * payoffType_;
		if (barrier_type_[0] == 'u') {
			payAmounts_[idx] = payoff > 0.0 && spot < barrier_ ? payoff : 0.0;
		}
		else {
			payAmounts_[idx] = payoff > 0.0 && spot > barrier_ ? payoff : 0.0;
		}	
	}
	else {  // this is not the last index, check whether barrier has been breached
		if (barrier_type_[0] == 'u') {
			payAmounts_[idx] = spot < barrier_ ? contValue : 0;
		}
		else {
			payAmounts_[idx] = spot > barrier_ ? contValue : 0;
		}
		
		
		//double intrinsicValue = (spot - strike_) * payoffType_;
		//intrinsicValue = intrinsicValue >= 0.0 ? intrinsicValue : 0.0;
		//payAmounts_[idx] = contValue >= intrinsicValue ? contValue : 0;
		// zero out the amounts after this index
		//for (size_t j = idx + 1; j < payAmounts_.size(); ++j)
		//	payAmounts_[j] = 0.0;
	}

  // if (idx == fixTimes_.size() - 1) { // this is the last index
  //  double payoff = (S_T - strike_) * payoffType_;
  //  payAmounts_[0] = payoff > 0.0 && contValue != 1.0 ? payoff : 0.0;
  //}
  //else {  // this is not the last index, check the exercise condition
  //  contValue = barrier_type_[0] == 'u' && S_T >= barrier_ && contValue != 1.0 ? 1.0 : 0.0;
  //  contValue = barrier_type_[0] == 'd' && S_T <= barrier_ && contValue != 1.0 ? 1.0 : 0.0;
  //}
	//if (idx == payTimes_.size() - 1) { // this is the last index
	//	if (payoffType_ == 1) {
	//		if (barrier_type_[0] == 'u') {
	//			if (barrier_type_[1] == 'o') {
	//				payAmounts_[idx] = spot >= strike_ && spot < barrier_ ? spot - strike_ : 0.0;
	//			}
	//			else {
	//				ORF_ASSERT(0, "Not implemented!");
	//			}
	//		}
	//		else {
	//			if (barrier_type_[1] == 'o') {
	//				payAmounts_[idx] = spot >= strike_ && spot > barrier_ ? spot - strike_ : 0.0;
	//			}
	//			else {
	//				ORF_ASSERT(0, "Not implemented!");
	//			}
	//		}
	//	}
	//	else {
	//		if (barrier_type_[0] == 'u') {
	//			if (barrier_type_[1] == 'o') {
	//				payAmounts_[idx] = spot <= strike_ && spot < barrier_ ? strike_ - spot : 0.0;
	//			}
	//			else {
	//				ORF_ASSERT(0, "Not implemented!");
	//			}
	//		}
	//		else {
	//			if (barrier_type_[1] == 'o') {
	//				payAmounts_[idx] = spot <= strike_ && spot > barrier_ ? strike_ - spot : 0.0;
	//			}
	//			else {
	//				ORF_ASSERT(0, "Not implemented!");
	//			}
	//		}
	//	}
	//}
	//else {  // this is not the last index, check whether barrier has been hit
	//	//double intrinsicValue = (spot - strike_) * payoffType_;
	//	//intrinsicValue = intrinsicValue >= 0.0 ? intrinsicValue : 0.0;
	//	//payAmounts_[idx] = contValue >= intrinsicValue ? contValue : intrinsicValue;
	//	/////// zero out the amounts after this index
	//	//for (size_t j = idx + 1; j < payAmounts_.size(); ++j)
	//	//	payAmounts_[j] = 0.0;

	//	if (payoffType_ == 1) {
	//		if (barrier_type_[0] == 'u') {
	//			if (barrier_type_[1] == 'o' && spot >= barrier_) {
	//				for (size_t j = idx + 1; j < payAmounts_.size(); ++j)
	//					payAmounts_[j] = 0.0;
	//			}
	//		}
	//		else {
	//			if (barrier_type_[1] == 'o' && spot <= barrier_) {
	//				for (size_t j = idx + 1; j < payAmounts_.size(); ++j)
	//					payAmounts_[j] = 0.0;
	//			}
	//		}
	//	}
	//	else {
	//		if (barrier_type_[0] == 'u') {
	//			if (barrier_type_[1] == 'o' && spot >= barrier_) {
	//				for (size_t j = idx + 1; j < payAmounts_.size(); ++j)
	//					payAmounts_[j] = 0.0;
	//			}
	//		}
	//		else {
	//			if (barrier_type_[1] == 'o' && spot <= barrier_) {
	//				for (size_t j = idx + 1; j < payAmounts_.size(); ++j)
	//					payAmounts_[j] = 0.0;
	//			}
	//		}
	//	}
	//}
}

END_NAMESPACE(orf)

#endif // ORF_BARRIERCALLPUT_HPP