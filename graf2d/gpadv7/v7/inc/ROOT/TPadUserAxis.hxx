/// \file ROOT/TPadUserAxis.hxx
/// \ingroup Gpad ROOT7
/// \author Axel Naumann <axel@cern.ch>
/// \date 2017-07-15
/// \warning This is part of the ROOT 7 prototype! It will change without notice. It might trigger earthquakes. Feedback
/// is welcome!

/*************************************************************************
 * Copyright (C) 1995-2018, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT7_TPadUserAxis
#define ROOT7_TPadUserAxis

#include <ROOT/TPadLength.hxx>

#include <algorithm>
#include <limits>

namespace ROOT {
namespace Experimental {

/** \class ROOT::Experimental::Internal::TPadUserAxisBase
  Base class for user coordinates (e.g. for histograms) used by `TPad` and `TCanvas`.
  */

class TPadUserAxisBase {
public:
   /// Types of axis bounds to respect by the painter. Non-fixed ones will be updated by
   /// the painter once the first paint has happened.
   enum EAxisBoundsKind {
      kAxisBoundsAuto, ///< no defined axis range; the painter will decide 
      kAxisBoundsBegin = 1, ///< the axis begin is to be respected by the painter.
      kAxisBoundsEnd = 2, ///< the axis end is to be respected by the painter.
      kAxisBoundsBeginEnd = kAxisBoundsBegin | kAxisBoundsEnd, ///< the axis minimum and maximum are to be respected by the painter
   };

private:
   /// Axis bounds to be used by the painter.
   int fBoundsKind = kAxisBoundsAuto;

   /// Begin of the axis range (but see fBoundsKind)
   double fBegin = 0.;

   /// End of the axis range (but see fBoundsKind)
   double fEnd = 1.;

protected:
   /// For (pos-min)/(max-min) calculations, return a sensible, div-by-0 protected denominator.
   double GetSensibleDenominator() const
   {
      if (fBegin < fEnd)
         return std::max(std::numeric_limits<double>::min(), fEnd - fBegin);
      return std::min(-std::numeric_limits<double>::min(), fEnd - fBegin);
   }

protected:
   /// Allow derived classes to default construct a TPadUserAxisBase.
   TPadUserAxisBase() = default;

   /// Construct a cartesian axis from min and max, setting fBoundsKind to kAxisBoundsMinMax.
   TPadUserAxisBase(double begin, double end): fBoundsKind(kAxisBoundsBeginEnd), fBegin(begin), fEnd(end) {}

   /// Construct a cartesian axis with min or max, depending on the boundKind parameter.
   TPadUserAxisBase(EAxisBoundsKind boundKind, double bound):
   fBoundsKind(boundKind), fBegin(bound), fEnd(bound) {}

   /// Disable spliced copy construction.
   TPadUserAxisBase(const TPadUserAxisBase &) = default;

   /// Disable spliced assignment.
   TPadUserAxisBase &operator=(const TPadUserAxisBase &) = default;

public:
   virtual ~TPadUserAxisBase();

   EAxisBoundsKind GetBoundsKind() const { return static_cast<EAxisBoundsKind>(fBoundsKind); }
   bool RespectBegin() const { return fBoundsKind & kAxisBoundsBegin; }
   bool RespectEnd() const { return fBoundsKind & kAxisBoundsEnd; }

   double GetBegin() const { return fBegin; }
   double GetEnd() const { return fEnd; }

   void SetBounds(double begin, double end)
   {
      fBoundsKind = kAxisBoundsBeginEnd;
      fBegin = begin;
      fEnd = end;
   }
   void SetBound(EAxisBoundsKind boundKind, double bound) { fBoundsKind = boundKind; fBegin = fEnd = bound; }
   void SetAutoBounds() { fBoundsKind = kAxisBoundsAuto; }

   void SetBegin(double begin) { fBoundsKind |= kAxisBoundsBegin; fBegin = begin; }
   void SetEnd(double end) { fBoundsKind |= kAxisBoundsEnd; fEnd = end; }

   /// Convert user coordinates to normal coordinates.
   virtual TPadLength::Normal ToNormal(const TPadLength::User &) const = 0;
};

class TPadCartesianUserAxis: public TPadUserAxisBase {
private:
   /// Whether this axis should be painted as log scale.
   bool fLogScale = false;

public:
   /// Construct a cartesian axis with automatic axis bounds.
   TPadCartesianUserAxis() = default;

   /// Construct a cartesian axis from min and max, setting fBoundsKind to kAxisBoundsMinMax.
   TPadCartesianUserAxis(double begin, double end): TPadUserAxisBase(begin, end) {}

   /// Construct a cartesian axis with min or max, depending on the boundKind parameter.
   TPadCartesianUserAxis(EAxisBoundsKind boundKind, double bound):
      TPadUserAxisBase(boundKind, bound) {}

   bool IsLogScale() const { return fLogScale; }
   void SetLogScale(bool logScale = true) { fLogScale = logScale; }
   
   /// Convert user coordinates to normal coordinates.
   TPadLength::Normal ToNormal(const TPadLength::User &usercoord) const override;
   
};
} // namespace Experimental
} // namespace ROOT

#endif
