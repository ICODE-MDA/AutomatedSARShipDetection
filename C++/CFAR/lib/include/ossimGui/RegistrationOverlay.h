#ifndef ossimGuiRegistrationOverlay_HEADER
#define ossimGuiRegistrationOverlay_HEADER

// #include <QtGui/QWidget>
#include <ossimGui/OverlayBase.h>
#include <ossimGui/Export.h>
#include <ossimGui/RegPoint.h>
#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimDpt.h>
#include <map>

typedef std::map<ossimString, ossimGui::RegPoint*> IdDptMap_t;
typedef std::map<ossimString, ossimGui::RegPoint*>::iterator IdDptMapIter_t;

namespace ossimGui {
		
	class IvtGeomTransform;

	class OSSIMGUI_DLL RegistrationOverlay : public OverlayBase
	{
		Q_OBJECT

	public:
	   RegistrationOverlay(QGraphicsScene* scene);
	   virtual void reset();

	   virtual void addPoint(const ossimDpt& scenePt, const ossimDpt& imagePt, const ossimString& id);
	   virtual void addPoint(const ossimDpt& scenePt, const ossimDpt& imagePt);
	   virtual void removePoint(const ossimString& id);
	   virtual void togglePointActive(const ossimString& id);

      virtual bool getImgPoint(const ossimString& id, ossimDpt& imgPt, bool& isActive);
      virtual ossimGui::RegPoint* getRegPoint(const ossimString& id);
      virtual ossimString getCurrentId()const {return m_currentId;}
      virtual ossim_uint32 getNumPoints()const {return m_IdPtXref.size();}
      virtual bool isControlImage()const {return m_isControlImage;}

      virtual void setVisible(const bool& visible);
      virtual void setAsControl(const bool& controlImage);
      virtual void setCurrentId(const ossimString& id) {m_currentId = id;}
      virtual void setView(ossimRefPtr<IvtGeomTransform> ivtg);
    
   signals:
   	void pointActivated(const ossimString&);
   	void pointDeactivated(const ossimString&);
   	void pointRemoved(const ossimString&);

	protected:
		IdDptMap_t 	m_IdPtXref;
      ossimString m_currentId;
      bool			m_isControlImage;

	};

}

#endif // ossimGuiRegistrationOverlay_HEADER
