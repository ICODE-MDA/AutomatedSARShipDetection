#ifndef ossimGuiAnnotationItem_HEADER
#define ossimGuiAnnotationItem_HEADER

#include <QtGui/QWidget>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QGraphicsLineItem>
#include <QtGui/QGraphicsRectItem>
#include <QtGui/QPen>
#include <ossimGui/Export.h>
#include <ossim/base/ossimDpt.h>
#include <ossim/base/ossimString.h>


namespace ossimGui {

	class OSSIMGUI_DLL AnnotationItem : public QGraphicsItem
	{

	public:
	   AnnotationItem();

	protected:
		virtual void 	hoverEnterEvent(QGraphicsSceneHoverEvent* event);
		virtual void 	hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
	   virtual void 	paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) = 0;
	   virtual QRectF boundingRect() const = 0;

	   virtual void doThisTest() = 0;
	
	   QPen m_apen;
	};

}

#endif // ossimGuiAnnotationItem_HEADER
