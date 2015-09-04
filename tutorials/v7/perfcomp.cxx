//
// Created by Axel Naumann on 08/07/15.
//

#include "ROOT/THist.h"
#include "ROOT/TFit.h"
#include "ROOT/THistBufferedFill.h"

#include "TH2.h"

#include <chrono>
#include <iostream>
#include <type_traits>

long createNewII(int count) {
  long ret = 1;
  for (int i = 0; i < count; ++i) {
    ROOT::TH2D hist({{{{0., 0.1, 0.3, 1.}}, {{0., 1., 2., 3., 10.}}}});
    ret ^= (long)&hist;
  }
  return ret;
}

#define BINSOLD \
  static const int nBinsX = 4; \
  double x[nBinsX] = {0., 0.1, 0.3, 1.}; \
  static const int nBinsY = 5; \
  double y[nBinsY] = {0., 1., 2., 3., 10.}

#define DECLOLD \
  TH2D hist("a", "a hist", nBinsX - 1, x, nBinsY - 1, y)


#define OLD \
  BINSOLD; \
  DECLOLD

long createOldII(int count) {
  BINSOLD;
  long ret = 1;
  for (int i = 0; i < count; ++i) {
    DECLOLD;
    ret ^= (long)&hist;
  }
  return ret;
}

long fillNewII(int count) {
  ROOT::TH2D hist({{{{0., 0.1, 0.3, 1.}}, {{0., 1., 2., 3., 10.}}}});
  for (int i = 0; i < count; ++i)
    hist.Fill({0.611, 0.611});
  return hist.GetNDim();
}

long fillOldII(int count) {
  OLD;
  for (int i = 0; i < count; ++i)
    hist.Fill(0.611, 0.611);
  return (long)hist.GetEntries();
}

long fillNII(int count) {
  ROOT::TH2D hist({{{{0., 0.1, 0.3, 1.}}, {{0., 1., 2., 3., 10.}}}});
  std::vector<std::array<double,2>> v(count);
  for (int i = 0; i < count; ++i)
    v[i] = {0.611, 0.611};
  hist.FillN(v);
  return hist.GetNDim();
}

long fillBufferedOldII(int count) {
  OLD;
  hist.SetBuffer(TH1::GetDefaultBufferSize());
  for (int i = 0; i < count; ++i)
    hist.Fill(0.611, 0.611);
  return (long)hist.GetEntries();
}

long fillBufferedNewII(int count) {
  ROOT::TH2D hist({{{{0., 0.1, 0.3, 1.}}, {{0., 1., 2., 3., 10.}}}});
  ROOT::THistBufferedFill<ROOT::TH2D> filler(hist);
  for (int i = 0; i < count; ++i)
    filler.Fill({0.611, 0.611});
  return hist.GetNDim();
}



// EQUIDISTANT

long createNewEE(int count) {
  long ret = 1;
  for (int i = 0; i < count; ++i) {
    ROOT::TH2D hist({{{100, 0., 1.}, {5, 0., 10.}}});
    ret ^= (long)&hist;
  }
  return ret;
}

long createOldEE(int count) {
  long ret = 1;
  for (int i = 0; i < count; ++i) {
    TH2D hist("a", "a hist", 100, 0., 1., 5, 0., 10.);
    ret ^= (long)&hist;
  }
  return ret;
}

long fillNewEE(int count) {
  ROOT::TH2D hist({{{100, 0., 1.}, {5, 0., 10.}}});
  for (int i = 0; i < count; ++i)
    hist.Fill({0.611, 0.611});
  return hist.GetNDim();
}

long fillOldEE(int count) {
  TH2D hist("a", "a hist", 100, 0., 1., 5, 0., 10.);
  for (int i = 0; i < count; ++i)
    hist.Fill(0.611, 0.611);
  return (long)hist.GetEntries();
}

long fillNEE(int count) {
  ROOT::TH2D hist({{{100, 0., 1.}, {5, 0., 10.}}});
  std::vector<std::array<double,2>> v(count);
  for (int i = 0; i < count; ++i)
    v[i] = {0.611, 0.611};
  hist.FillN(v);
  return hist.GetNDim();
}

long fillBufferedOldEE(int count) {
  TH2D hist("a", "a hist", 100, 0., 1., 5, 0., 10.);
  hist.SetBuffer(TH1::GetDefaultBufferSize());
  for (int i = 0; i < count; ++i)
    hist.Fill(0.611, 0.611);
  return (long)hist.GetEntries();
}

long fillBufferedNewEE(int count) {
  ROOT::TH2D hist({{{100, 0., 1.}, {5, 0., 10.}}});
  ROOT::THistBufferedFill<ROOT::TH2D> filler(hist);
  for (int i = 0; i < count; ++i)
    filler.Fill({0.611, 0.611});
  return hist.GetNDim();
}


using timefunc_t = std::add_pointer_t<long(int)>;

void time1(timefunc_t run, int count, const std::string& name) {
  using namespace std::chrono;
  auto start = high_resolution_clock::now();
  run(count);
  auto end = high_resolution_clock::now();
  duration<double> time_span = duration_cast<duration<double>>(end - start);

  std::cout << count << " * " << name << ": " << time_span.count() << "seconds \n";
}

void time(timefunc_t r6, timefunc_t r7, int count, const std::string& name) {
  time1(r6, count, name + " (ROOT6)");
  time1(r7, count, name + " (ROOT7)");
}

int main(int argc, const char* argv[]) {
  int factor = 1000000;
  //factor = 1; // debug, fast!
  time(createOldII, createNewII, factor, "create 2D hists [II]");
  time(createOldEE, createNewEE, factor, "create 2D hists [EE]");
  time(fillOldII, fillNewII, 100 * factor, "2D fills [II]");
  time(fillOldEE, fillNewEE, 100 * factor, "2D fills [EE]");
  time(fillBufferedOldII, fillBufferedNewII, 100 * factor, "2D fills (buffered) [II]");
  time(fillBufferedOldEE, fillBufferedNewEE, 100 * factor, "2D fills (buffered) [EE]");
  return 0;
}
