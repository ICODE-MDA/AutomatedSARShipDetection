#ifndef ossimGuiMarkPoint_HEADER
#define ossimGuiMarkPoint_HEADER

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

	class OSSIMGUI_DLL MarkPoint : public AnnotationItem
	{

	public:
	   MarkPoint(const ossimDpt& scenePos, const ossimDpt& imgPos, const ossimString& id = "");
	   inline ossimString getID()const {return m_id;}
	   inline ossimDpt getImgPos()const {return m_imgPos;}
	   inline bool isActive()const {return m_isActive;}
	   inline void setActive(const bool& active) {m_isActive = active;}

	   virtual void doThisTest() {std::cout<<"\n doThisTest"<<std::endl;}

	protected:
		virtual void 	hoverEnterEvent(QGraphicsSceneHoverEvent* event);
		virtual void 	hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
	   virtual void 	paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	   virtual QRectF boundingRect() const;

	   QRectF m_rect;
	   QLineF m_ver;
	   QLineF m_hor;
	   qreal  m_len;
	   QPen   m_pen;

	   ossimString m_id;
	   ossimDpt m_imgPos;
	   bool m_isActive;
	};

}

#endif // ossimMarkPoint_HEADER
