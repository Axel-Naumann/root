//
//  THistStats.h
//  ROOT7
//
//  Created by Axel Naumann on 14/06/15.
//
//

#ifndef ROOT_THistStats_h
#define ROOT_THistStats_h

#include <vector>

namespace ROOT {

template<int DIMENSIONS, class PRECISION>
class THistStatEntries {
public:
  using Coord_t = std::array<double, DIMENSIONS>;
  using Weight_t = PRECISION;

  void Fill(const Coord_t& /*x*/, Weight_t /*weightN*/ = 1.) { ++fEntries; }
  void FillN(const std::array_view<Coord_t> xN,
             const std::array_view<Weight_t> /*weightN*/) {
    fEntries += xN.size();
  }
  void FillN(const std::array_view<Coord_t> xN) {
    fEntries += xN.size();
  }

  int64_t GetEntries() const { return fEntries; };

  std::vector<double>
  GetBinUncertainties(int binidx, const THistImplBase<DIMENSIONS, PRECISION>& hist) const {
    PRECISION sqrtcont = std::sqrt(std::max(hist.GetBinContent(binidx), 0.));
    return std::vector<double>{sqrtcont, sqrtcont};
  }
    private:
  int64_t fEntries = 0;
};

template<int DIMENSIONS, class PRECISION>
class THistStatUncertainty: public THistStatEntries<DIMENSIONS, PRECISION> {
public:
  using Base_t = THistStatEntries<DIMENSIONS, PRECISION>;
  using typename Base_t::Coord_t;
  using typename Base_t::Weight_t;

  void Fill(const Coord_t &x, Weight_t weightN = 1.) {
    Base_t::Fill(x, weightN);
  }

  void FillN(const std::array_view<Coord_t> xN,
             const std::array_view<Weight_t> weightN) {
    Base_t::FillN(xN, weightN);
  }

  void FillN(const std::array_view<Coord_t> xN) {
    Base_t::FillN(xN);
  }
};

template<int DIMENSIONS, class PRECISION>
class THistStatMomentUncert: public THistStatUncertainty<DIMENSIONS, PRECISION> {
public:
  using Base_t = THistStatUncertainty<DIMENSIONS, PRECISION>;
  using typename Base_t::Coord_t;
  using typename Base_t::Weight_t;

  void Fill(const Coord_t &x, Weight_t weightN = 1.) {
    Base_t::Fill(x, weightN);
  }

  void FillN(const std::array_view<Coord_t> xN,
             const std::array_view<Weight_t> weightN) {
    Base_t::FillN(xN, weightN);
  }
  void FillN(const std::array_view<Coord_t> xN) {
    Base_t::FillN(xN);
  }
};


template<int DIMENSIONS, class PRECISION>
class THistStatRuntime: public THistStatEntries<DIMENSIONS, PRECISION> {
public:
  using Base_t = THistStatEntries<DIMENSIONS, PRECISION>;
  using typename Base_t::Coord_t;
  using typename Base_t::Weight_t;

  THistStatRuntime(bool uncertainty, std::vector<bool> &moments);

  void Fill(const Coord_t &x, Weight_t weightN = 1.) {
    Base_t::Fill(x, weightN);
  }

  void FillN(const std::array_view<Coord_t> xN,
             const std::array_view<Weight_t> weightN) {
    Base_t::FillN(xN, weightN);
  }
  void FillN(const std::array_view<Coord_t> xN) {
    Base_t::FillN(xN);
  }
};

} // namespace ROOT
#endif
