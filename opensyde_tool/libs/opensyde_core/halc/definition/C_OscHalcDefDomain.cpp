//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       HALC definition domain

   Detailed description (optional). The module shall be described detailed if it is not described completely
   by short description.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_SclChecksums.hpp"
#include "C_OscHalcDefDomain.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscHalcDefDomain::C_OscHalcDefDomain(void) :
   e_Category(C_OscHalcDefDomain::eCA_OTHER)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscHalcDefDomain::~C_OscHalcDefDomain(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out]  oru32_HashValue  Hash value with initial [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscHalcDefDomain::CalcHash(uint32_t & oru32_HashValue) const
{
   const QByteArray c_IdData = this->c_Id.toUtf8();
   stw::scl::C_SclChecksums::CalcCRC32(c_IdData.constData(), static_cast<uint32_t>(c_IdData.size()), oru32_HashValue);
   const QByteArray c_NameData = this->c_Name.toUtf8();
   stw::scl::C_SclChecksums::CalcCRC32(c_NameData.constData(), static_cast<uint32_t>(c_NameData.size()), oru32_HashValue);
   const QByteArray c_CommentData = this->c_Comment.toUtf8();
   stw::scl::C_SclChecksums::CalcCRC32(c_CommentData.constData(), static_cast<uint32_t>(c_CommentData.size()),
                                       oru32_HashValue);
   const QByteArray c_SingularNameData = this->c_SingularName.toUtf8();
   stw::scl::C_SclChecksums::CalcCRC32(c_SingularNameData.constData(), static_cast<uint32_t>(c_SingularNameData.size()),
                                       oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(&this->e_Category, sizeof(this->e_Category), oru32_HashValue);

   for (uint32_t u32_It = 0UL; u32_It < this->c_Channels.size(); ++u32_It)
   {
      this->c_Channels[u32_It].CalcHash(oru32_HashValue);
   }

   for (uint32_t u32_It = 0UL; u32_It < this->c_ChannelUseCases.size(); ++u32_It)
   {
      this->c_ChannelUseCases[u32_It].CalcHash(oru32_HashValue);
   }

   this->c_DomainValues.CalcHash(oru32_HashValue);
   this->c_ChannelValues.CalcHash(oru32_HashValue);
}
