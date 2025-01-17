/** 
@file  sptr.hpp
@brief Type aliases for working with std::shared_ptr<T>
*/

#ifndef ORF_SPTR_HPP
#define ORF_SPTR_HPP

#include <orflib/defines.hpp>
#include <memory>
#include <string>

BEGIN_NAMESPACE(orf)

using SPtrInt = std::shared_ptr<int>;
using SPtrLong = std::shared_ptr<long>;
using SPtrDouble = std::shared_ptr<double>;
using SPtrString = std::shared_ptr<std::string>;

END_NAMESPACE(orf)

#endif // ORF_SPTR_HPP
