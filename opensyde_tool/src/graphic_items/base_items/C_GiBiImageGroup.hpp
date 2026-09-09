//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Implementation for drawing element image (header)

   See cpp file for detailed description

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_GIIMAGEGROUP_HPP
#define C_GIIMAGEGROUP_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>

#include <cstdint>

#include "C_GiImageGroupWithoutData.hpp"
#include "C_PuiBsImage.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_GiBiImageGroup :
   public C_GiImageGroupWithoutData
{
public:
   C_GiBiImageGroup(const uint64_t & oru64_Id, const QString & orc_ImagePath, QGraphicsItem * const opc_Parent = nullptr);

   C_GiBiImageGroup(const uint64_t & oru64_Id, const double of64_Width, const double of64_Height,
                    const QPixmap & orc_Image, const QByteArray & orc_Format, QGraphicsItem * const opc_Parent = nullptr);

   int32_t type() const override;

   //GI base
   void SetZetValueCustom(const double of64_ZetValue) override;

protected:
   void m_UpdateData(stw::opensyde_gui_logic::C_PuiBsImage & orc_Data) const;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
