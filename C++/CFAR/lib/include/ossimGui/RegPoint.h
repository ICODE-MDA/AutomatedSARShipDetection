#ifndef ossimGuiRegPoint_HEADER
#define ossimGuiRegPoint_HEADER

#include <QtGui/QWidget>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QGraphicsLineItem>
#include <QtGui/QGraphicsRectItem>
#include <QtGui/QPen>
#include <ossimGui/Export.h>
#include <ossimGui/AnnotationItem.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimString.h>


namespace ossimGui {

	class OSSIMGUI_DLL RegPoint : public AnnotationItem
	{

	public:
	   RegPoint(const ossimDpt& scenePos, const ossimDpt& imgPos, const ossimString& id = "");
	   ossimString getID()const {return m_id;}
	   ossimDpt getImgPos()const {return m_imgPos;}
	   bool isActive()const {return m_isActive;}
	   bool isControlPoint()const {return m_isControlPoint;}

	   void setActive(const bool& active);
	   void setAsControlPoint(const bool& control) {m_isControlPoint = control;}

	   virtual void doThisTest() {std::cout<<"\n doThisTest"<<std::endl;}

	protected:
		virtual void 	hoverEnterEvent(QGraphicsSceneHoverEvent* event);
		virtual void 	hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
	   virtual void 	paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	   virtual QRectF boundingRect() const;

	   QLineF m_ver;
	   QLineF m_hor;
	   qreal  m_len;
	   QPen   m_pen;
	   QPen   m_savedPen;

	   ossimString m_id;
	   ossimDpt m_imgPos;
	   bool m_isActive;
	   bool m_isControlPoint;
	};

}

#endif // ossimGuiRegPoint_HEADER
