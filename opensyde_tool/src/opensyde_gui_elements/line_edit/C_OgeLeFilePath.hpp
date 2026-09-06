#ifndef C_OGE_LE_FILE_PATH_HPP
#define C_OGE_LE_FILE_PATH_HPP
#include "C_OgeLeFilePathBase.hpp"
#include <QString>
#include <QStringList>
namespace stw { namespace opensyde_gui_elements {
class C_OgeLeFilePath : public C_OgeLeFilePathBase {
public:
    explicit C_OgeLeFilePath(QWidget * opc_Parent = nullptr) : C_OgeLeFilePathBase(opc_Parent) {}
    ~C_OgeLeFilePath() override = default;
};
}}
#endif
