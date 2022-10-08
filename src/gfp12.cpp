#include <bitlen.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <constants.h>
#include <gfp12.h>
#include <sstream>

namespace bn256 {

gfp12 gfp12::conjugate() const noexcept {
   const gfp12& a = *this;
   return { a.x_.neg(), a.y_ };
}

gfp12 gfp12::neg() const noexcept {
   const gfp12& a = *this;
   return { a.x_.neg(), a.y_.neg() };
}

// Frobenius computes (xω+y)^p = x^p ω·ξ^((p-1)/6) + y^p
gfp12 gfp12::frobenius() const noexcept {
   const gfp12& a = *this;
   gfp12        e;
   e.x_ = a.x_.frobenius();
   e.y_ = a.y_.frobenius();
   e.x_ = e.x_.mul_scalar(constants::xi_to_p_minus_1_over_6);
   return e;
}

// FrobeniusP2 computes (xω+y)^p² = x^p² ω·ξ^((p²-1)/6) + y^p²
gfp12 gfp12::frobenius_p2() const noexcept {
   const gfp12& a = *this;
   gfp12        e;
   e.x_ = a.x_.frobenius_p2();
   e.x_ = e.x_.mul_gfp(constants::xi_to_p_squared_minus_1_over_6);
   e.y_ = a.y_.frobenius_p2();
   return e;
}

gfp12 gfp12::frobenius_p4() const noexcept {
   const gfp12& a = *this;
   gfp12        e;
   e.x_ = a.x_.frobenius_p4();
   e.x_ = e.x_.mul_gfp(constants::xi_to_p_squared_minus_1_over_3);
   e.y_ = a.y_.frobenius_p4();
   return e;
}

gfp12 gfp12::add(const gfp12& b) const noexcept {
   const gfp12& a = *this;
   return { a.x_.add(b.x_), a.y_.add(b.y_) };
}

gfp12 gfp12::sub(const gfp12& b) const noexcept {
   const gfp12& a = *this;
   return { a.x_.sub(b.x_), a.y_.sub(b.y_) };
}

gfp12 gfp12::mul(const gfp12& b) const noexcept {
   const gfp12& a = *this;

   gfp6 tx = a.x_.mul(b.y_);
   gfp6 t  = b.x_.mul(a.y_);
   tx = tx.add(t);

   gfp6 ty = a.y_.mul(b.y_);
   t  = a.x_.mul(b.x_).mul_tau();

   return { tx, ty.add(t) };
}

gfp12 gfp12::mul_scalar(const gfp6& b) const noexcept {
   const gfp12& a = *this;
   return { a.x_.mul(b), a.y_.mul(b) }; // need review ???
}

gfp12 gfp12::exp(const int512_t& power) const noexcept {
   const gfp12& a   = *this;
   gfp12        sum = one(), t;

   for (int i = bitlen(power); i >= 0; i--) {
      t = sum.square();
      if (bit_test(power, i) != 0) {
         sum = t.mul(a);
      } else {
         sum = t;
      }
   }

   return sum;
}

gfp12 gfp12::square() const noexcept {
   const gfp12& a = *this;
   // Complex squaring algorithm

   gfp6 v0 = a.x_.mul(a.y_);

   gfp6 t  = a.x_.mul_tau();
   t       = a.y_.add(t);
   gfp6 ty = a.x_.add(a.y_);
   ty      = ty.mul(t).sub(v0);
   t       = v0.mul_tau();
   ty      = ty.sub(t);

   return { v0.add(v0), ty };
}

gfp12 gfp12::invert() const noexcept {
   const gfp12& a = *this;
   // See "Implementing cryptographic pairings", M. Scott, section 3.2.
   // ftp://136.206.11.249/pub/crypto/pairings.pdf

   gfp6 t1 = a.x_.square();
   gfp6 t2 = a.y_.square();
   t1      = t1.mul_tau();
   t1      = t2.sub(t1);
   t2      = t1.invert();

   gfp12 e;
   e.x_ = a.x_.neg();
   e.y_ = a.y_;
   e = e.mul_scalar(t2);
   return e;
}

std::string gfp12::string() const { return "(" + x_.string() + "," + y_.string() + ")"; }

std::ostream& operator<<(std::ostream& os, const gfp12& v) { return os << "(" << v.x_ << "," << v.y_ << ")"; }
} // namespace bn256
