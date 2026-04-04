#ifndef C_OGE_PUB_SVG_ICON_ONLY_HPP
#define C_OGE_PUB_SVG_ICON_ONLY_HPP
#include <QPushButton>
#include <QString>
namespace stw { namespace opensyde_gui_elements {
class C_OgePubSvgIconOnly : public QPushButton {
public:
    explicit C_OgePubSvgIconOnly(QWidget * opc_Parent = nullptr) : QPushButton(opc_Parent) {}
    ~C_OgePubSvgIconOnly() override = default;
    void SetSvg(const QString&) {}
    void SetSvg(const QString&, const QString&) {}
    void SetSvg(const QString&, const QString&, const QString&) {}
    void SetSvg(const QString&, const QString&, const QString&, const QString&) {}
    void SetSvg(const QString&, const QString&, const QString&, const QString&, const QString&) {}
    void SetSvg(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&) {}
    void SetSvg(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&) {}
    void SetSvg(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&) {}
    void SetCustomIcons(const QString&, const QString&, const QString&, const QString&) {}
    void SetCustomIcons(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&) {}
    void SetToolTipInformation(const QString&, const QString&) {}
};
}}
#endif
