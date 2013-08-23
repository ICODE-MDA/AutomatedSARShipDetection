#ifndef ossimGuiMetricOverlay_HEADER
#define ossimGuiMetricOverlay_HEADER

#include <ossimGui/OverlayBase.h>
#include <ossimGui/Export.h>
#include <ossimGui/MarkPoint.h>
#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimString.h>
#include <ossim/base/ossimDpt.h>
#include <map>

typedef std::map<ossimString, ossimGui::MarkPoint*> IdMptMap_t;
typedef std::map<ossimString, ossimGui::MarkPoint*>::iterator IdMptMapIter_t;

namespace ossimGui {
		
	class IvtGeomTransform;

	class OSSIMGUI_DLL MetricOverlay : public OverlayBase
	{
		Q_OBJECT
		
	public:
		MetricOverlay(QGraphicsScene* scene);
		virtual void reset();

		virtual void addPoint(const ossimDpt& scenePt, const ossimDpt& imagePt, const ossimString& id);
		virtual void addPoint(const ossimDpt& scenePt, const ossimDpt& imagePt);
		virtual void removePoint(const ossimString& id);
		virtual void togglePointActive(const ossimString& id);

    	virtual bool getImgPoint(const ossimString& id, ossimDpt& imgPt, bool& isActive);
    	virtual ossimGui::MarkPoint* getMarkPoint(const ossimString& id);
    	virtual ossimString getCurrentId()const {return m_currentId;}
    	virtual ossim_uint32 getNumPoints()const {return m_IdPtXref.size();}

    	virtual void setVisible(const bool& visible);
    	virtual void setCurrentId(const ossimString& id) {m_currentId = id;}
      virtual void setView(ossimRefPtr<IvtGeomTransform> ivtg);
    
   signals:
   	void pointActivated(const ossimString&);
   	void pointDeactivated(const ossimString&);
   	void pointRemoved(const ossimString&);

	protected:
		IdMptMap_t 	m_IdPtXref;
      ossimString m_currentId;

	};

}

#endif // ossimGuiMetricOverlay_HEADER
