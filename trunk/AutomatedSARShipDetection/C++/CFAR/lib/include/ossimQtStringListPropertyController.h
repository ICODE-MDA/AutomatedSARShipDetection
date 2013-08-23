//*******************************************************************
// Copyright (C) 2000 ImageLinks Inc. 
//
// OSSIM is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
//
// You should have received a copy of the GNU General Public License
// along with this software. If not, write to the Free Software 
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-
// 1307, USA.
//
// See the GPL in the COPYING.GPL file for more details.
//
// Author: Garrett Potts (gpotts@imagelinks.com)
//
//*************************************************************************
// $Id: ossimQtStringListPropertyController.h 12109 2007-12-04 18:09:45Z gpotts $
#ifndef ossimQtStringListPropertyController_HEADER
#define ossimQtStringListPropertyController_HEADER
#include <QtCore/QObject>

class ossimQtStringListPropertyDialog;
class ossimStringListProperty;

class ossimQtStringListPropertyController : public QObject
{
   Q_OBJECT
public:
   ossimQtStringListPropertyController(ossimQtStringListPropertyDialog* dialog);
   virtual ~ossimQtStringListPropertyController();
   
   void setOssimProperty(const ossimStringListProperty* prop);
   const ossimStringListProperty* getOssimProperty()const;

public slots:
   virtual void applyButton();
   
protected:
   ossimQtStringListPropertyDialog *theDialog;
   ossimStringListProperty         *theProperty;

   void transferPropertyToDialog();
   void transferPropertyFromDialog();

signals:
   void apply(ossimStringListProperty* prop);
   void changed(ossimStringListProperty* prop);
};

#endif
