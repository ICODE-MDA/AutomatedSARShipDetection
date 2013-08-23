#ifndef ossimGuiOverlayBase_HEADER
#define ossimGuiOverlayBase_HEADER

#include <QtGui/QWidget>
#include <QPointF>
#include <ossimGui/Export.h>
#include <ossim/base/ossimObject.h>
#include <ossim/base/ossimRefPtr.h>
#include <ossim/base/ossimDpt.h>

class QGraphicsScene;

namespace ossimGui {

	class OSSIMGUI_DLL OverlayBase : public QObject
	{
		Q_OBJECT
		
	public:
	   OverlayBase(QGraphicsScene* scene);
      virtual bool isActive()const {return m_isActive;}
      virtual void setActive(const bool& active) {m_isActive = active;}

	protected:
		QGraphicsScene*	m_scene;
      bool					m_isActive;

	};

}

#endif // ossimGuiOverlayBase_HEADER
