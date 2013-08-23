//----------------------------------------------------------------------------
//
// "Copyright Centre National d'Etudes Spatiales"
//
// License:  LGPL
//
// See LICENSE.txt file in the top level directory for more details.
//
//----------------------------------------------------------------------------
// $Id$

#ifndef ossimErsSarModel_H
#define ossimErsSarModel_H

#include <otb/JSDDateTime.h>
#include <ossimGeometricSarSensorModel.h>

#include <ossim/projection/ossimMapProjection.h>
#include <ossim/base/ossimIpt.h>
#include <ossim/base/ossimFilename.h>
#include <ossim/base/ossimGpt.h>
#include <ossim/base/ossimDpt.h>

#include <iostream>

namespace ossimplugins
{

class PlatformPosition;
class SensorParams;
class RefPoint;
class ErsSarLeader;
/**
 * @brief This class is able to direct localisation and indirect localisation using the ErsSar sensor model
 *
 */
class ossimErsSarModel : public ossimGeometricSarSensorModel
{
public:
  /**
   * @brief Constructor
   */
  ossimErsSarModel();

  /**
   * @brief Destructor
   */
  virtual ~ossimErsSarModel();

  /**
   * @brief Method to return the class name.
   * @return The name of this class.
   */
  virtual ossimString getClassName() const;

  /**
   * @brief Returns pointer to a new instance, copy of this.
   */
  virtual ossimObject* dup() const;

  /**
   * @brief This function associates an image column number to a slant range when the image is georeferenced (ground projected)
   * @param col Column coordinate of the image point
   */
  virtual double getSlantRangeFromGeoreferenced(double col) const;

  /**
  * @brief Method to instantiate model from the leader file.
  * @param file
  * @return true on success, false on error.
  */
  bool open(const ossimFilename& file);

  /**
   * @brief Method to save object state to a keyword list.
   * @param kwl Keyword list to save to.
   * @param prefix added to keys when saved.
   * @return true on success, false on error.
   */
  virtual bool saveState(ossimKeywordlist& kwl,
                         const char* prefix = 0) const;

  /**
   * @brief Method to the load (recreate) the state of the object from a
   * keyword list. Return true if ok or false on error.
   * @return true if load OK, false on error
   */
  virtual bool loadState(const ossimKeywordlist &kwl, const char *prefix = 0);

protected:
  virtual bool InitPlatformPosition(const ossimKeywordlist &kwl, const char *prefix);
  virtual bool InitSensorParams(const ossimKeywordlist &kwl, const char *prefix);
  virtual bool InitRefPoint(const ossimKeywordlist &kwl, const char *prefix);
  /**
  * @brief Initializes the Slant Range for each Ground Range data sets : theNumberSRGR,theSRGRCoeffset,_srgr_update,thePixelSpacing,_isProductGeoreferenced
  */
  virtual bool InitSRGR(const ossimKeywordlist &kwl, const char *prefix);

private:
  /**
   *  @brief Slant Range for each Ground Range (SRGR) number of coefficients sets
   */
  int   theNumberSRGR;
  /**
   * @brief SRGR coefficient sets
   */
  double theSRGRCoeffset[1][3];
  /**
   * @brief Pixel spacing
   */
  double thePixelSpacing;

  /**
   * @brief List of metadata contained in the Leader file
   */
  ErsSarLeader *theErsSarleader;



  virtual bool isErsLeader(const ossimFilename& file) const;
  virtual ossimFilename findErsLeader(const ossimFilename& file) const;

  TYPE_DATA

};

}
#endif
