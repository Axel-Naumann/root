/// \file TFrame.cxx
/// \ingroup Gpad ROOT7
/// \author Axel Naumann <axel@cern.ch>
/// \date 2017-09-26
/// \warning This is part of the ROOT 7 prototype! It will change without notice. It might trigger earthquakes. Feedback
/// is welcome!

/*************************************************************************
 * Copyright (C) 1995-2017, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "ROOT/TFrame.hxx"

#include "ROOT/TLogger.hxx"
#include "ROOT/TPadUserCoordBase.hxx"

namespace {
   // FIXME: this is probably worth documenting, thus not in unnamed namespace in source!
   class TPadUserCoordDefault: public Detail::TPadUserCoordBase {

   };
}

ROOT::Experimental::TFrame::TFrame(std::unique_ptr<Detail::TPadUserCoordBase> &&coords, const TPadPos &pos,
                                   const TPadExtent &size)
   : TFrame(pos, size), fUserCoord(std::move(coords))
{
   if (!fUserCoord)
      fUserCoord = std::make_unique<TPadUserCoordDefault>();
}
